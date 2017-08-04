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

#ifndef __ARVORE_CANDIDATOS_H__
#define __ARVORE_CANDIDATOS_H__
#include <sstream>
#include <map>
#include "tipos_dados.h"
#include "sqlite.h"

#define MSG_ID MSG << "[" << rdt.ID_RDt << "," << efeito.ID_efeito << "," << geracao << "] " 

class ArvoreCandidatos
{
   public:
      RDt rdt;
      Efeito efeito;
      map<int, string>* dicionario;
      vector<NoCandidato> candidatos;
      int geracao;
      /* se a arvore deixou de gerar candidatos ela passa a ser invalida, isso
         significa que ela pode ser removida. */
      bool valida;
      
      /* atributos para manipução da base de dados */
      sqlite3* bd;
      sqlite3_stmt* stmtSumarizada;

      static string comandos;
      static int qtd_comandos;
      static int MAX_COMANDOS;
   public:
      ArvoreCandidatos(sqlite3* _bd, RDt _rdt, Efeito _efeito, map<int,string>* _dicionario): \
      bd(_bd), rdt(_rdt), efeito(_efeito), geracao(1), dicionario(_dicionario), valida(true) {}
      ~ArvoreCandidatos() {}

      void ativar(float, float);
      void validar();
      void validar(NoCandidato&, int);
      void contarOcorrencias();
      void calculoSupConfCE(float, float);
      void calculoSupConfCE(float, float,NoCandidato&,vector<int>,int,RDt&,Efeito&);
      void novaGeracao();
      void novaGeracao(NoCandidato&, int);
      void match(vector<int>&);
      void match(NoCandidato&, vector<int>);

      /* métodos para teste da árvore */
      void mostrar();
      void mostrar(NoCandidato&, string);
      void status();
      void status(NoCandidato&,int,int,string);

      /* métodos para manipução da base de dados */
      string traduzCausa(vector<int>);
      void inserir_RCE(int, int, int, int, string, float, float);
      void deletar_SCE(int);
      static void enfileirar(string, sqlite3*);

      static void persistir(sqlite3*);
};

#endif
