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

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <iostream>
#include <vector>
#include <map>
// #include <process.h>
#include <sstream>
#include "sqlite.h"
#include "tipos_dados.h"

using namespace std;


/* Utilizada para preencher o vetor de ID_RDt + ID_efeito. Este por sua vez é 
   um agrupamento dos efeitos + rdts frequentes, faz com que no momento do calculo
   do supConfCE não sejam realizadas passagens inuteis no banco (combinação efeito frq
   para RDt errada). Mais detalhes em descoberta.cpp método calculoSupConfCE() */
typedef struct rdtefeito
{
   int ID_RDt, ID_efeito;
}RDtEfeito;

/* Se a quantidade de comandos for maior que MAX_COMANDOS
   os comandos são executados (ver "inserir", "atualizar") */
class Database 
{
   private:
      /* conexão com o banco de dados (disco). Também possui um banco em memoria
         anexado a esta conexão. */
      sqlite3* bd;
      /* nome do arquivo de entrada, este valor é recebido por parametro */
      string nomeArquivoEntrada;
      /* nome do arquivo de banco de dados */
      string nomeBD;
      /* buffer para a execução de uma lista de comandos */
      string comandos;
      int qtd_comandos;
      
      /* Função de callback utilizada para filtrar os Efeitos não frequentes
         segundo a Frequencia Minima de Efeito (FME) */
      static int callback_filtrar_FME(void*, int, char**, char**);
      /* Função de callback para debug, executa uma SQL e escreve na saída padrão 
         o resultado */
      static int callback_debug(void *, int, char **, char **);
      void criar();
      /* TODO: voltar executar para o private. Isso foi feito para que fosse possível
         modificar o código facilmente para escrever no disco a tabela sumarizada */
      int enfileirar(string);
    public:
      int executar(string);
      /* qtd maxima de comandos que podem ser empilhados em "comandos" */
      int MAX_COMANDOS;
      Database(int, string);
      ~Database();
      int persistir();
      int persistir_dicionario();
      int persistir_efeito();
      int persistir_RDts(vector<RDt>&);
      int selecionar_int(string);
      int selecionar_int(string, int);
      unsigned long long int selecionar_llint(string);
      void selecionar(string, vector<string> &);
      void selecionar(string, vector<int> &);
      void selecionar(string, vector<Dicionario> &);
      void selecionar_RDts(vector<RDt> &);
      void selecionar_efeitos(vector<Efeito> &);
      void selecionar_dicionario(vector<int> &);
      void selecionar_dicionario(map<int, string>&);
      void selecionar_sumarizada(vector<RSCE> &, int);
      void selecionar(int, vector<Evento> &);
      int inserir(string, vector<string>, vector<string>);
      int inserir(string, vector<int>);
      int inserir(string, int, int, int, int);
      int inserir(string, string, string);
      int atualizar(string, vector<string>, vector<int>, int);
      int atualizar(string, string, int, int);
      int deletar(string, string, int);
      int deletar(string);
      int atualizar_distancia(int rowid);
      unsigned long long int selecionar_llint(string query, int value);

      /* filtro de efeito frq, supCE e confCE */
      int inserir_efeitos_FRQ(float, vector<RDt>*);
      int selecionar_SCE(vector<int>&, int);

      const string getComandos() { return comandos; }
      sqlite3* getBD() { return bd; }

      /* Métodos de teste */
      int executar_debug(string);
};


/* Parametro para a função callback_filtrar_FME */
typedef struct paraF
{
   Database* bd;
   float FME;
   vector<RDt>* rdts;
}ParametroFME;

#endif
