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

#ifndef _EVENTOENTRADA_H_
#define _EVENTOENTRADA_H_

#include <iostream>
#include <time.h>


using namespace std;

class EventoEntrada {
    private:
        string nome_EventoEntrada;
        string data_EventoEntrada;
        time_t epoch;
    public:
        EventoEntrada(const char *);
        EventoEntrada(){};        
        ~EventoEntrada(){};
        
        const char *retorna_nome_EventoEntrada();
        const char *retorna_data_EventoEntrada();
        time_t retorna_epoch();
        
        void define_nome_EventoEntrada(const char *);       
        void define_data_EventoEntrada(const char *);
        void define_epoch(time_t);
};

#endif //_EVENTOENTRADA_H_
