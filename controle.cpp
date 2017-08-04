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

#include "controle.h"

/*   Se nenhum valor for especificado para FME, SMCE, CMCE e MC os valors dafault
   sao utilizados (1%, 5%, 10% e 3 respectivamente). */
Controle::Controle():FME(1), SMCE(5), CMCE(10), MC(3), nCluster(3), MAX_COMANDOS(100000)
{
}

/* Parametros validos -fFME -sSMCE -cCMCE -mMC -dNOME_ENTRADA -kNOME_SAIDA -u#CLUSTER -rNOME_RDt
      -qQTD_MAX_COMANDOS_SQL_BUFFER
   Se nao houver NOME_SAIDA é utilizado o NOME_ENTRADA + hoje() + hora() 
      para o nome do arquivo de banco de dados de saida. 
   O parametro arquivo de RDt tem prioridade sobre #Cluster, ou seja, se 
      forem ambos passados, é lido o arquivo de RDt e ignorado o #Cluster 
      se nenhum for passado é assumido #Cluster default = 3 */
int Controle::avalia_parametros(int argc, char* argv[])
{
   string par = "";
   for(int i = 1 ; i < argc ; ++i)
   {
      par += argv[i];
      par += ":";
   }
   if(par.size() == 0)
   {
      MSG_ERRO << "Invalid parameters" << endl;
      return 1;
   }
   while(par.size() > 1)
   {
      if(par[0] == '-')
      {
         char chave = par[1];
         string valor = par.substr(2, par.find(":")-2);
         par = par.substr(par.find(":")+1, string::npos);
         switch(chave)
         {
            case 'f':
            {
               FME = atof(valor.c_str());
               if(FME > 100.0 || FME < 0.0)
               {
                  MSG_AVISO << "Invalid Minimum Event Frequency. Using Default (1%)" << endl;
                  FME = 0;
               }
            }
            case 's':
            {
               SMCE = atof(valor.c_str());
               if(SMCE > 100.0 || SMCE < 0.0)
               {
                  MSG_AVISO << "Invalid Support CE. Using Default (5%)" << endl;
                  SMCE = 10.0;
               }
               break;
            }
            case 'c':
            {
               CMCE = atof(valor.c_str());
               if(CMCE > 100.0 || CMCE < 0.0)
               {
                  MSG_AVISO << "Invalid Confidence CE. Using Default (10%)" << endl;
                  CMCE = 80.0;
               }
               break;
            }
            case 'm':
            {
               MC = atoi(valor.c_str());
               if(MC < 1)
               {
                  MSG_AVISO << "Invalid Minimum Causes. Using default (3)" << endl;
                  MC = 3;
               }
               break;
            }
            case 'q':
            {
               MAX_COMANDOS = atoi(valor.c_str());
               if(MAX_COMANDOS < 1)
               {
                  MSG_AVISO << "MAX_COMMANDS invalid. Using default (MAX_COMANDS = 100000)" << endl;
                  MAX_COMANDOS = 100000;
               }
               break;
            }
            case 'd': 
            {
               nomeEntrada = valor;
               break;
            }
            case 'k': 
            {
               nomeBD = valor;
               break;
            }
//            case 'u': 
//            {
//               nCluster = atoi(valor.c_str());
//               if(nCluster < 1)
//               {
//                  MSG_AVISO << "Parametro #Cluster invalido, utilizando default (#Cluster = 3)" << endl;
//                  nCluster = 3;
//               }
//               break;
//            }
            case 'r':
            {
               nomeRDt = valor;
               break;
            }
            default:
               MSG_ERRO << "Parametro invalido: '" << chave << "'" << endl;
               return 1;
         }
      }
      else
      {
         MSG_ERRO << "incluir - antes do identificador de parametro.." << endl;
         return 1;
      }
   }
   return 0;
}

/* Delega a execução para as classes que compõe a descoberta de RCE 
      1. Traducao
      2. Definicao
      3. Sumarizacao
      4. Descoberta */
int Controle::iniciar(int argc, char** argv)
{
   MSG << "RCE1.0 [" << __DATE__ << " " << __TIME__ << "]" << endl<< endl;
   /* Um parametro esta invalido */
   if(avalia_parametros(argc, argv))
   {
      MSG <<"\t-fFME\t\tMinimum Event Frequency. #efeito(rdt) / #SCE(rdt). Default = 1\n" << \
            "\t-sSMCE\t\tMinimum Support CE. #efeito_causa(rdt) / #SCE(rdt). Default = 5\n" << \
            "\t-cCMCE\t\tMinimum Confidence CE. #efeito_causa(rdt) / #efeito(rdt). Default = 10n" << \
            "\t-mMC\t\tMax Causes. Maximum number of causes in the cause set. Default = 3\n" << \
            "\t-dINPUT_FILE\tInput file name.\n" <<
            "\t-kOUTPUT_FILE\tOutput dataset containing the rules name. " << \
            "Optional (uses input file name + timestamp if none specified)\n" <<
            "\t-rTDR_FILE Time Distance Restriction file name.\n" <<
            "\t-qMAX_COMMANDS Max commands in SQL buffer\t" << \
            "(How many commands to be queued before executing them). Default = 100000\n" << endl;
      return -1;
   }
   MSG << "Parameters: \n\tSMCE = " << SMCE << "\n\tCMCE = " << CMCE << "\n\tMC = " << MC << \
      "\n\tFME = " << FME << "\n\tnomeEntrada = " \
      << nomeEntrada << "\n\tnomeSaida = " << (nomeBD=="" ? "Nenhum especificado (utilizando data+hora atual)." : nomeBD) << \
      "\n\tQuantidade de Clusters = " << nCluster \
      << "\n\tNome do arquivo de RDt = " << (nomeRDt==""?"Nenhum especificado(usando cluster)." : nomeRDt) \
      << "\n\tQtd Maxima de comandos SQL no buffer = " << MAX_COMANDOS << endl << endl;

   Database bd(MAX_COMANDOS, nomeBD);

   Traducao traducao(&bd);
   if(traducao.preparar_Traducao(nomeEntrada.c_str()))
   {
      MSG_ERRO << "Erro ao preparar a traducao" << endl;
      return 1;
   }
   if(traducao.ler_evento_entrada())
   {
      MSG_ERRO << "Erro na leitura da base de entrada..." << endl;
      return 1;                                                                                              
   }

     Definicao _definicao(&bd);
     if(nomeRDt.empty())
     {
         if(_definicao.prepara_Definicao(nCluster))
         {
              MSG_ERRO << "Erro durante a definicao.." << endl;
              return 1;
         }
         
         if(_definicao.ler_Distancias())
         {
              MSG_ERRO << "Erro durante a leitura das distancias..." << endl;
              return 1;                                     
         }
      }
      else
      {
         if(_definicao.prepara_Definicao(nomeRDt.c_str()))
         {
              MSG_ERRO << "Erro durante a preparacao para a Definicao" << endl;
              return 1;
         }  
         if(_definicao.ler_rdts())
         {
              MSG_ERRO << "Error reading TDR file" << endl;
              return 1;                                                                           
         }
     }

   //bd.executar_debug("SELECT * FROM bd_memoria.RDt");

   Sumarizacao sumarizacao(&bd);
   sumarizacao.sumarizar();
   sumarizacao.removerCausasNFRQ(SMCE);
   //Especifica o tamanho do buffer de comandos para a descoberta igual ao de Database */
   ArvoreCandidatos::MAX_COMANDOS = bd.MAX_COMANDOS;
   Descoberta descoberta(&bd, sumarizacao.causas_qtd, SMCE, CMCE, FME, MC, MAX_COMANDOS);
   descoberta.descobertaRCE();
    return 0;
}
