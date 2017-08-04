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

#ifndef __TIPOS_DADOS__
#define __TIPOS_DADOS__
#include <vector>
#include <iostream>

using namespace std;

/* Registro da tabela RDt */
class RDt
{
   public:
      int ID_RDt, DtMi, DtMa, quantidade;
      RDt():ID_RDt(0){}
      //RDt(long long int _rowid, int _DtMi, int _DtMa, int _quantidade):\
      //   Registro(_rowid), DtMi(_DtMi), DtMa(_DtMa), quantidade(_quantidade) {}
      RDt(int _ID_RDt, int _DtMi, int _DtMa, int _quantidade):\
         ID_RDt(_ID_RDt), DtMi(_DtMi), DtMa(_DtMa), quantidade(_quantidade) {}
};

/* Registro da tabela Traduzida */
/*rowid é um alias para ID_Evento, pois ID_Evento é chave primária 
         http://www.sqlite.org/lang_createtable.html#rowid */
class Evento
{
   public:
      int rowid, ID_SE, ID_evento, TS;
      Evento(int _rowid, int _ID_SE, int _ID_evento, int _TS):\
         rowid(_rowid), ID_SE(_ID_SE), ID_evento(_ID_evento), TS(_TS) {}
};

/* Registro da tabela Sumarizada */
class RSCE
{
   public:
      int rowid, ID_SCE, ID_causa, ID_efeito, ID_RDt;
      RSCE(int _rowid, int _ID_SCE, int _ID_causa, int _ID_efeito, int _ID_RDt):\
         rowid(_rowid), ID_SCE(_ID_SCE), ID_causa(_ID_causa), ID_efeito(_ID_efeito), \
         ID_RDt(_ID_RDt) {}
};

/* Registro da tabela Efeito */
class Efeito
{
   public:
      int rowid, ID_efeito, ID_RDt, quantidade;
      float frequencia;
      Efeito(int _rowid, int _ID_efeito, int _ID_RDt, int _quantidade, float _frequencia):\
         rowid(_rowid), ID_efeito(_ID_efeito), ID_RDt(_ID_RDt), quantidade(_quantidade), \
         frequencia(_frequencia) {}
};
/*
class RCE : public Registro
{
   private:
      int rowid, ID_RCE, tamanho;
      string corpo, RDt;
      double suporteCE, confiancaCE, frequenciaEfeito;
      RCE(int _rowid):rowid(_rowid) {}
};
*/

/* Representação de um NoCandidato. Um candidato é a união da sequência de 
   NoCandidato em uma Árvore de Candidatos. Cada instância de NoCandidato 
   representa a adição de uma nova geração de candidatos. A quantidade em 
   cada nó indica a ocorrência deste candidato na base, por exemplo:
      A-B-C -> E, sendo ABC o conjunto causa e E o efeito.
      A -> E tem profundidade = 1 (Candidato 1-C) e quantidade = 200
      A-B -> E tem profundidade = 2 (Candidato 2-C) e quantidade = 150
      A-B-C -> E tem profundidade = 3 (Candidato 3-C) e quantidade = 100
   Obs: Assim como no algoritmo Apriori usamos a propriedade de antimonotonicidade, 
      também conhecida como propriedade "Apriori", ela enuncia que: Se A não é frequente
      logo A-B também não o será. Este modelo de representação esta baseado nisso, 
      os candidatos são gerados a partir daqueles que eram frequentes na geração anterior. */
class NoCandidato
{
   public:
      int ID_causa;
      int quantidade;
      vector<NoCandidato> candidatos;
      NoCandidato(int _ID_causa): ID_causa(_ID_causa), quantidade(0) {}
      /* Ao copiar um nó não deve herdar a quantidade de ocorrências */
      NoCandidato(const NoCandidato& c)
      {
         *this = c;
         this->quantidade = 0;
      }
};

typedef struct CFRQ
{
   int ID_RDt;
   int ID_causa;
   int quantidade;
}CausaQtd;

/* NOVO - Registro da tabela Dicionario */
class Dicionario
{
   public:
      unsigned int ID_evento;
      const unsigned char *descricao;
      Dicionario(int _ID_evento, const unsigned  char * _descricao):\
         ID_evento(_ID_evento), descricao(_descricao) {}
};

#endif
