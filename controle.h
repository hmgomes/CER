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

#ifndef __CONTROLE_H__
#define __CONTROLE_H__
#include "database.h"
#include "sumarizacao.h"
#include "descoberta.h"
#include "traducao.h"
#include "definicao.h"

class Controle
{
   private:
      /* Parameters... */
      /* Min Frequency */
      float FME;
      /* Minimum Support CE */
      float SMCE;
      /* Minimum Confidence CE */
      float CMCE;
      /* Max Cause Events */
      int MC;
      /* Maximum size of the SQL buffer. 
         There are two types of buffers. Details in Database.cpp and Descoberta.cpp */
      int MAX_COMANDOS;
      /* File names: input file, output and time distances */
      int nCluster;
      string nomeEntrada;
      string nomeBD;
      string nomeRDt;
      Database* bd;
   public:
      Controle();
      /* Interpreter for parameters -fFME -sSMCE -cCMCE -mMC -dNOME_ENTRADA -kNOME_SAIDA -u#CLUSTER -rNOME_RDt */
      int avalia_parametros(int, char**);
      int iniciar(int, char**);
};

#endif
