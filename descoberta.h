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

#ifndef __DESCOBERTA_H__
#define __DESCOBERTA_H__
#include "arvore_candidatos.h"
#include <map>
#include "utils.h"
#include "database.h"

/* Frequência de Efeito (FE) por Restricao de Distância de Tempo (RDt)
      FE(e,rdt) = #ocorrências(e,rdt) / #total_SCE(rdt)

   Suporte de Causa e Efeito (SupCE) por Restricao de Distância de Tempo (RDt)
      SupCE(rce, rdt) = #ocorrências(rce, rdt) / #total_SCE(rdt)

   Confiança de Causa e Efeito (ConfCE) por Restricao de Distancia de Tempo (RDt)
      ConfCE(rce, rdt) = #ocorrências(rce, rdt) / #ocorrências(e, rdt)

    Obs: rce significa Regra de Causa e efeito, formado pela tupla (c, e), onde
         c eh um conjunto de causa, logo c = {evt1,evt2,...,evtN}
         e eh um evento efeito, logo e = evtE.
         rdt representa uma restricao de distância de tempo formada por DtMi e DtMa,
            distância mínima e distância máxima, respectivamente. */
class Descoberta
{
   private:
      Database* bd;
      /* Frequencia Minima de Efeito, Suporte Minimo de Causa e Efeito,
         Confianca Minima de Causa e Efeito*/
      float FME, SMCE, CMCE;
      /* Maximo de Eventos na Causa */
      int MC;
      /* todas as RDts */
      vector<RDt> rdts;
      /* apos a execucao do metodo filtroEfeito, contem todos os efeitos FRQ */
      vector<Efeito> efeitos_FRQ;
      /* contem as causas FRQ */
      vector<CausaQtd>& causas_FRQ;
      /* contem a tabela dicionario na forma de um map */
      map<int, string> dicionario;
      int geracao;
      vector<RDtEfeito> filtro;
      vector<ArvoreCandidatos> arvoresCandidatos;
      void gerar1C();
   public:
      Descoberta(Database*, vector<CausaQtd>&, float, float, float, int, int);
      int filtroEfeito();
      int descobertaRCE();
};

#endif
