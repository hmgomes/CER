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
#include "traducao.h"
#include "database.h"
#include "evento_entrada.h"
#include "utils.h"

using namespace std;

Traducao::Traducao(Database *db):bd(db)
{
}

Traducao::~Traducao()
{
    arquivo_entrada.close();
}

/* Prepara a classe Traducao e definindo o
arquivo de entrada. */
int Traducao::preparar_Traducao(const char *f)
{   
   definir_nome_arquivo(f);
   if(nome_arquivo.empty())
   {
      MSG_ERRO << "Erro Nome do arquivo do banco de dados de entrada invalido" << endl;
      return 1;
   }
                        
   arquivo_entrada.open(nome_arquivo.c_str());
   
   if( !arquivo_entrada ) 
   {
     MSG_ERRO << "Erro ao abrir arquivo de entrada " << nome_arquivo << endl;
     return 1;
   }
   
   return 0;
}

void Traducao::definir_nome_arquivo(const char *f)
{
    nome_arquivo = string(f);
}

/* Método que percorre o arquivo de entrada e popula as tabelas Dicionario e 
Traduzida através do método inserir_evento_entrada */
int Traducao::ler_evento_entrada()
{
   MSG << "Iniciando Traducao..." << endl;
   string line, sevent;
   stringstream saux;
   EventoEntrada *e;
   int id_reg = 0;
   
   if(!arquivo_entrada.is_open())
   {
      cerr << "Erro ao abrir o arquivo de entrada " << nome_arquivo << endl;
      return 1;                           
   }
   
   while(getline(arquivo_entrada, line)) 
   {
      saux.str(std::string());
      saux << line;
      id_reg++;
      while(getline(saux, sevent, ' '))
      {
         if(sevent.find("(") == string::npos or sevent.find(")") == string::npos)
         {
            cout << "Invalid input Event - " << sevent << endl;
            return 1;
         }
         e = new EventoEntrada(sevent.c_str());
         inserir_evento_entrada(e, id_reg);
         free(e);
      }
      saux.clear();
   }
   
   bd->persistir();
   MSG << "Finalizando Traducao..." << endl << endl;
   return 0;
}

/* Método que tem como objetivo a inserção dos Eventos de Entrada no BD */
int Traducao::inserir_evento_entrada(EventoEntrada *e, int id_reg)
{
   string query;
   stringstream sint;
   int id_evento = 0;
   vector<string> campos;
   vector<string> valores;
   vector<Dicionario> dicionario;
   
   //Dicionario
   dicionario.clear();
   bd->selecionar(e->retorna_nome_EventoEntrada(), dicionario);
   
   if(dicionario.empty())
   {
      query = "'";
      query += e->retorna_nome_EventoEntrada();        
      query += "'";
      
      bd->inserir("dicionario", "descricao", query.c_str());
      bd->persistir();
      
      id_evento = bd->selecionar_int(DICIONARIO_MAX_IDEVENTO);
   }
   else
   {
      id_evento = dicionario[0].ID_evento;          
   }

//Traduzida   
   campos.clear();
   valores.clear();
   campos.push_back("ID_SE");
   campos.push_back("ID_evento");
   campos.push_back("TS");
   
   sint.str(std::string()); 
   sint << id_reg;    
   valores.push_back(sint.str());
   
   sint.str(std::string()); 
   sint << id_evento;
   valores.push_back(sint.str());
   
   sint.str(std::string()); 
   sint << e->retorna_epoch();
   valores.push_back(sint.str());
   
   bd->inserir("traduzida", campos, valores);
    return 0;
}
