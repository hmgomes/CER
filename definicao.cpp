/*
 *    @author Heitor Murilo Gomes (heitor_murilo_gomes at yahoo dot com dot br)
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <math.h>
#include <limits>
#include "database.h"
#include "Definicao.h"
#include "utils.h"

using namespace std;

Definicao::Definicao(Database *_bd):bd(_bd)
{
    nome_arquivo.erase();
}

/* Prepara a classe Definicao para a entrada manual de RDts */
int Definicao::prepara_Definicao(const char *f)
{
    nome_arquivo = string(f);
    return 0;
}

/* Prepara a classe Definicao para a descoberta automática de RDts */
int Definicao::prepara_Definicao(int n)
{   
    ncentroides = n;
    return 0;
}

/* Método para calcular as distancias a partir da tabela Traduziada, inserindo
na tabela de Distancias */
int Definicao::ler_Distancias()
{
   MSG << "Iniciando leitura das Distancias... " << endl;
   int rowid;
   int maxID_SE = bd->selecionar_int(TRADUZIDA_MAX_ID_SE);
   /* vector com uma sequencia de SE */
   vector<Evento> SE;
   /* percorre o conjunto SE em ordem decrescente... */
   for(int j = 0 ; j <= maxID_SE ; ++j)
   {
      bd->selecionar(j, SE);
      if(SE.empty())
         continue;
      for(int e = (int) SE.size()-1 ; e > 0 ; --e)
      {
         /* percorre o conjunto SE em ordem crescente... */
         for(int c = 0 ; c < SE.size() ; ++c)
         {
            /* calcula a distancia do efeito para a causa */
            int distancia = SE[e].TS - SE[c].TS;
            rowid = bd->selecionar_int(DISTANCIA_SELECT_ROWID_DISTANCIA, distancia);
            if(rowid == 0 && distancia > 0)
            {
               vector<int>valores;
               valores.push_back(distancia);
               valores.push_back(1);
               valores.push_back(-1);
               bd->inserir("distancia (distancia, quantidade, cluster)", valores);
               bd->persistir();
            }
            else
            {
               bd->Database::atualizar_distancia(rowid);
            }
         }
      }
   }
   bd->persistir();
   MSG << "Finalizando leitura das Distancias... " << endl << endl;
   MSG << "Total de distancias = " << bd->selecionar_int("SELECT COUNT(1) FROM bd_memoria.distancia") << endl;
   return 0;
}

/* Método para gerar um número randomico de 64bits */
int Definicao::rand_ll()
{
	int random = rand();
	return (int)random*(2147483647/RAND_MAX);
}

/* método de newton raphson para encontrar a raiz quadrada de um número. */
long double Definicao::sqrtll (unsigned long long value, long double old_value)
{
   long double new_value = (value + powl(old_value,2))/(2*old_value);
   long double diff = old_value - new_value;
     
   if(diff > 0.0001 || diff < -0.0001)
   {
      return sqrtll (value, new_value);
   }  
   return new_value;
}

/* Método para processar arquivo de RDt definidas pelo usuário */
int Definicao::ler_rdts() 
{
    MSG << "Iniciando leitura das RDt (Definicao parametrizada)... "  << endl;
    string line, sRDt, sseg;
    stringstream saux, sauxRDt, sminRDt, smaxRDt;
    unsigned int seg, minRDt = 0, maxRDt = 0, count = 0, iaux = 0;
    bool min = true;
    vector<string> campos;
    vector<string> valores;    

   if(nome_arquivo.empty())
   {
      MSG_ERRO << "Erro Nome do arquivo de RDt invalido" << endl;
      return 1;
   }

   arquivo_rdts.open(nome_arquivo.c_str());
 
   if( !arquivo_rdts ) 
   {
      MSG_ERRO << "Erro ao abrir arquivo de RDt " << nome_arquivo << endl;
      return 1;
   }

   while(getline(arquivo_rdts, line)) 
   {
      if(line.length() > 0)
      {
         saux.clear();
         saux.str(std::string());                                                                    
         saux << line;        
         min = true;
         minRDt = 0;
         maxRDt = 0;
    
         while(getline(saux, sRDt, ';'))
         {
               sauxRDt.clear();
               sauxRDt.str(std::string());                            
               sauxRDt << sRDt;           
               count = 0;
               seg = 0;
               iaux = 0;
               sseg.clear();
               while(getline(sauxRDt, sseg, ':'))
               {
                    if(sseg.length() > 0)
                     {
                        switch (count)
                        {
                            case 0: seg += atoi(sseg.c_str())*31104000; break; //ANO = 12 meses
                            case 1: seg += atoi(sseg.c_str())*2592000; break; //MES = 30 dias
                            case 2: seg += atoi(sseg.c_str())*86400; break; //DIA
                            case 3: seg += atoi(sseg.c_str())*3600; break; //HORA
                            case 4: seg += atoi(sseg.c_str())*60; break; //MINUTOS
                            case 5: seg += atoi(sseg.c_str()); break; //SEGUNDOS
                            default: break;                    
                        }
                    }
                    count++;                     
                }    
                if(min == true)   
                    minRDt = seg;
                else
                    maxRDt = seg;
                min = false;    
            }   
            if(minRDt > maxRDt)
            {
                seg = minRDt;
                minRDt = maxRDt;
                maxRDt = seg;
            }

            sminRDt.clear();
            sminRDt.str(std::string());
            sminRDt << minRDt;

            smaxRDt.clear();
            smaxRDt.str(std::string());
            smaxRDt << maxRDt;

            campos.clear();
            valores.clear();
            campos.push_back("DtMi");
            campos.push_back("DtMa");
            campos.push_back("quantidade");
            
            valores.push_back(sminRDt.str());
            valores.push_back(smaxRDt.str());
            valores.push_back("0");
            
            bd->inserir("RDt", campos, valores);
        }
    }
    arquivo_rdts.close();
    bd->persistir();
    MSG << "Finalizando leitura das RDt (Definicao parametrizada)... " << endl << endl;
    return 0;
}
