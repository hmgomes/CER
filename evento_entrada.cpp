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
#include <string>
#include <time.h>
#include "evento_entrada.h"
#include "utils.h"

using namespace std;

/* Construtor da classe, le o eventoEntrada e popula os atributos da classe */
EventoEntrada::EventoEntrada(const char *s_event)
{
    string str = string(s_event);
    stringstream stream(str); 
    string::iterator it;
    struct tm tm;
    int yy=0, mm=0, dd=0, H=0, M=0, S=0;

    getline(stream, nome_EventoEntrada, '(');
    getline(stream, data_EventoEntrada, '(');
    
    it = data_EventoEntrada.end()-1;
    data_EventoEntrada.erase(it);
    
    sscanf(data_EventoEntrada.c_str(), "%d/%d/%d-%d:%d:%d" ,&mm,&dd,&yy,&H,&M,&S);
    
    tm.tm_year = yy - 1900;
    tm.tm_mon = mm - 1;
    tm.tm_mday = dd;
    tm.tm_hour = H;
    tm.tm_min = M;
    tm.tm_sec = S;

    tm.tm_isdst = 0;
    
    epoch = mktime(&tm);
    epoch = epoch + (GMT*3600); //ajusta ao GMT corrente
}

const char* EventoEntrada::retorna_nome_EventoEntrada()
{
      return nome_EventoEntrada.c_str();
}

const char* EventoEntrada::retorna_data_EventoEntrada()
{
      return data_EventoEntrada.c_str();
}

time_t EventoEntrada::retorna_epoch()
{
       return epoch;
}

void EventoEntrada::define_nome_EventoEntrada(const char *nome)
{
     nome_EventoEntrada = string(nome);
}

void EventoEntrada::define_data_EventoEntrada(const char *data)
{
     data_EventoEntrada = string(data);     
}

void EventoEntrada::define_epoch(time_t e)
{
     epoch = e;
}
