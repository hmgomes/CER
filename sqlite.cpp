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

#include "sqlite.h"

/* http://www.sqlite.org/c3ref/open.html */
int SQLite::abrir(sqlite3** bd, const char* nome, int flags)
{
    /* Por algum motivo no mac o v2 nao funciona o SQLITE_OPEN_CREATE, por isso usa a primeira versao para criar */
    int rc = -1;
    if(flags == SQLITE_OPEN_CREATE)
        rc = sqlite3_open(nome, bd);// , flags, NULL);
    else
        rc = sqlite3_open_v2(nome, bd, flags, NULL);
   if( rc )
   {
      MSG_ERRO << "Nao foi possivel abrir/criar o database. Erro [" << rc << "]: " << ultimo_erro(*bd) << endl;
      //getchar();
      exit(1);
   }
   // MSG_AVISO << rc << endl;
   return rc;
}
/* http://www.sqlite.org/c3ref/close.html */
int SQLite::fechar(sqlite3* bd)
{
   int rc = sqlite3_close(bd);
   if( rc )
   {
      MSG_ERRO << "Nao foi possivel fechar o database. Erro [" << rc << "]: " << ultimo_erro(bd) << endl;
      //getchar();
   }
   // MSG_AVISO << rc << endl;
   return rc;
}
/* Nesta função estão encapsuladas duas outras do sqlite. São elas: 
      sqlite3_reset() e sqlite3_clear_bindings(). 
   A primeira retorna o statement ao seu estado original (antes das chamadas a sqlite3_step)
      http://www.sqlite.org/c3ref/reset.html 
   A segunda remove todos os valores que foram anexados aquele statement. 
      http://www.sqlite.org/c3ref/clear_bindings.html */
int SQLite::restaurar(sqlite3_stmt* stmt)
{
   int rc = sqlite3_reset(stmt);
   if( rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel restaurar o stmt. Erro [" << rc << "]: " << endl;
      //getchar();
   }
   rc = sqlite3_clear_bindings(stmt);
   if( rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel remover os bindings do stmt. Erro [" << rc << "]: " << endl;
      //getchar();
   }
   // MSG_AVISO << rc << endl;
   return rc;
}

/* O ultimo argumento da funcao sqlite3_prepare é um ponteiro para o 
   proximo comando dos comandos presentes no segundo parametro. Estes 
   comandos sao separados por ponto e vírgula. Neste trabalho não são
   utilizados statements com mais de um comando no segundo argumento, 
   portanto no método "preparar" não é necessário passar tal parametro e
   ele é sempre NULL quando da chamada de sqlite3_prepare aqui. */
int SQLite::preparar(sqlite3* bd, const char* sql, sqlite3_stmt** stmt)
{
   int rc = sqlite3_prepare_v2(bd, sql, -1, stmt, NULL);
   if( rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel preparar o stmt. Erro [" << rc << "]: " << ultimo_erro(bd) << endl;
      //getchar();
   }
   // MSG_AVISO << rc << endl;
   return rc;
}

int SQLite::finalizar(sqlite3_stmt* stmt)
{
   int rc = sqlite3_finalize(stmt);
   if( rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel finalizar o stmt. Erro [" << rc << "]: ";// << ultimo_erro(bd) << endl;
      //getchar();
   }
   // MSG_AVISO << rc << endl;
   return rc;
}

/* http://www.sqlite.org/c3ref/extended_result_codes.html
   habilita o uso da tabela de erros extendidos do sqlite */
int SQLite::habilitar_erros_extendidos(sqlite3* bd, int ligado)
{
   return sqlite3_extended_result_codes(bd, ligado);
}

/* Adiciona um parametro int a um statement já compilado 
   http://www.sqlite.org/c3ref/bind_blob.html 
   OBS: contraria a qualquer outra função, biblioteca, e afins criadas em C, 
   o bind inicia o indice em 1, então solicitar que seja anexado um valor no 
   indice 0 vai falhar na hora da recuperação dos dados(e não na hora de fazer
   o bind como era de se esperar). */
int SQLite::anexar_int(sqlite3_stmt* stmt, int idx, int valor)
{
   int rc = 0;
   rc = sqlite3_bind_int(stmt, idx, valor);
   if( rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel anexar o valor " << valor << " (int). \
         Erro [" << rc << "]: ";// << ultimo_erro(bd) << endl;
      //getchar();
   }
   // MSG_AVISO << rc << endl;
   return rc;
}

/* http://www.sqlite.org/c3ref/step.html */
int SQLite::passo(sqlite3_stmt* stmt)
{
   int rc = sqlite3_step(stmt);
   if( rc != SQLITE_ROW && rc != SQLITE_DONE && rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel executar o passo. Erro [" << rc << "]: ";// << ultimo_erro(bd) << endl;
      //getchar();
   }
   // MSG_AVISO << rc << endl;
   return rc;
}

/* http://www.sqlite.org/c3ref/exec.html
   A função sqlite3_exec é um 'wrapper' para as funções
   sqlite3_prepare_v2(), sqlite3_step() e sqlite3_finalize. 
   Isso significa que sera criado um statement e ele terá o(s)
   comando(s) compilado(s), executado(s) e será finalizado. 
   São disponibilizados dois métodos: O primeiro apenas executa os
      comandos, logo não recebe (e na hora de chamar usa NULL) o 
      parametro da função de callback (parametro 3 de sqlite3_exec).
   O segundo método recebe o ponteiro para a função de callback, 
      porém omite o ponteiro para os parametros, pois neste trabalho
      não são passados parametros para as funções de callback que
      executam a partir do sqlite3_exec. */
/* Versão 1: apenas executa (util para INSERT e UPDATE) */
int SQLite::executar(sqlite3* bd, const char* sql)
{  
   char* msg_erro = NULL;
   int rc = sqlite3_exec(bd, sql, NULL, NULL, &msg_erro);
   if( rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel executar. Erro(sqlite) [" << rc << "]: " << msg_erro << \
         "\n************SQL*********** \n" << sql << "\n************SQL***********" << endl;
      //getchar();
      sqlite3_free(msg_erro);
      return 1;
   }
   // MSG_AVISO << rc << endl;
   return rc;
}
/* Versão 2: recebe função de callback (util para SELECT) */
int SQLite::executar(sqlite3* bd, const char* sql, sqlite3_callback cl)
{  
   char* msg_erro = NULL;
   int rc = sqlite3_exec(bd, sql, cl, NULL, &msg_erro);
   if( rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel executar. Erro(sqlite) [" << rc << "]: " << msg_erro << \
         "\n************SQL*********** \n" << sql << "\n************SQL***********" << endl;
      //getchar();
      sqlite3_free(msg_erro);
      return 1;
   }
   // MSG_AVISO << rc << endl;
   return rc;
}

/* Versão 3: recebe função de callback e parametros para a mesma (pr)*/
int SQLite::executar(sqlite3* bd, const char* sql, sqlite3_callback cl, void* pr)
{  
   char* msg_erro = NULL;
   int rc = sqlite3_exec(bd, sql, cl, pr, &msg_erro);
   if( rc != SQLITE_OK )
   {
      MSG_ERRO << "Nao foi possivel executar. Erro(sqlite) [" << rc << "]: " << msg_erro << \
         "\n************SQL*********** \n" << sql << "\n************SQL***********\n" << endl;
      //getchar();
      sqlite3_free(msg_erro);
      return 1;
   }
   return rc;
}

/* http://www.sqlite.org/c3ref/column_blob.html
   Retorna o valor de uma coluna do stmt, baseado no indice 
   valor de retorno será convertido para "const unsigned char*" */
const unsigned char* SQLite::valor_coluna_texto(sqlite3_stmt* stmt, int indice)
{
   const unsigned char *val = sqlite3_column_text(stmt, indice);
   if( val == NULL )
   {
      MSG_ERRO << "valor da coluna null(text)" << endl;
   }

   return val;
}

/* http://www.sqlite.org/c3ref/column_blob.html
   Retorna o valor de uma coluna do stmt, baseado no indice 
   valor de retorno será convertido para "float" */
float SQLite::valor_coluna_float(sqlite3_stmt* stmt, int indice)
{
   float val = atof((const char*)sqlite3_column_text(stmt, indice));
   if( val == 0.0f )
   {
      MSG_ERRO << "valor da coluna null (float)" << endl;
      //getchar();
   }

   return val;
}

/* http://www.sqlite.org/c3ref/column_blob.html 
   Retorna o valor de uma coluna do stmt, baseado no indice 
   valor de retorno será convertido para "int" */
int SQLite::valor_coluna_int(sqlite3_stmt* stmt, int indice)
{
   int val = sqlite3_column_int(stmt, indice);
   if( sqlite3_column_type(stmt, indice) == SQLITE_NULL )
   {
      //MSG_ERRO << "Valor da coluna[" << indice << "](int) = NULL" << endl; 
   }
   return val;
}

/* http://www.sqlite.org/c3ref/column_blob.html 
   Retorna o valor de uma coluna do stmt, baseado no indice 
   valor de retorno será convertido para "long long int" (sqlite3_int64) */
long long int SQLite::valor_coluna_int64(sqlite3_stmt* stmt, int indice)
{
   long long int val = sqlite3_column_int64(stmt, indice);
   if( sqlite3_column_type(stmt, indice) == SQLITE_NULL )
   {
      //MSG_ERRO << "Aviso: Valor da coluna[" << indice << "](int64) = NULL" << endl;
   }
   return val;
}

const char* SQLite::ultimo_erro(sqlite3* bd)
{
   return sqlite3_errmsg(bd);
}
