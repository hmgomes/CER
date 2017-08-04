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

#ifndef __SUMARIZACAO_H__
#define __SUMARIZACAO_H__
#include "database.h"
#include "tipos_dados.h"

class Sumarizacao
{
   private:
      Database* bd;
   public:
   /* Lista de candidatos à candidatos 1-C. Mais detalhes em sumarizacao.cpp */
      vector<CausaQtd> causas_qtd;
      Sumarizacao(Database*);
      void sumarizar();
      void iniciarCausasFRQ();
      void removerCausasNFRQ(float);
};

#endif
