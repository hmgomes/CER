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

#ifndef __SQLITE_H__
#define __SQLITE_H__
#include "utils.h"

/* Funcoes mapeadas da biblioteca sqlite3
   Lista de funcoes do SQLite (inglês): http://www.sqlite.org/c3ref/funclist.html
   Mapear as funcoes, permite manter o tratamento de erros limitado
      a esta classe.   
   Como indicado na documentacao do SQLite, eh feito uso das novas versoes
      das funcoes (terminadas em v2). */
class SQLite
{
   public:
      static int abrir(sqlite3**, const char*, int); /* sqlite3_open */
      static int fechar(sqlite3*); /* sqlite3_close */
      static int restaurar(sqlite3_stmt*); /* sqlite3_reset */    
      static int preparar(sqlite3*, const char*, sqlite3_stmt**); /*sqlite3_prepare_v2 */
      static int finalizar(sqlite3_stmt*); /* sqlite3_finalize */
      static int habilitar_erros_extendidos(sqlite3*, int); /* sqlite3_extended_result_codes */
      static int anexar_int(sqlite3_stmt*, int, int); /* sqlite3_bind_int */
      static int passo(sqlite3_stmt*); /*sqlite3_step */
      static int executar(sqlite3*, const char*); /* sqlite3_exec */
      static int executar(sqlite3*, const char*,  sqlite3_callback); /* sqlite3_exec */
      static int executar(sqlite3*, const char*,  sqlite3_callback, void*); /* sqlite3_exec */
      static const unsigned char* valor_coluna_texto(sqlite3_stmt*, int); /* sqlite3_column_text */
      static float valor_coluna_float(sqlite3_stmt*, int); /* sqlite3_column_text */
      static int valor_coluna_int(sqlite3_stmt*, int); /* sqlite3_column_int */
      static long long int valor_coluna_int64(sqlite3_stmt*, int); /* sqlite3_column_int64 */
      static const char* ultimo_erro(sqlite3*); /* sqlite3_errmsg */
};

#endif

