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

#ifndef _TRADUCAO_H_
#define _TRADUCAO_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "database.h"
#include "evento_entrada.h"

class Traducao {
    private:
        Database *bd;
        string nome_arquivo;
        fstream arquivo_entrada;         
    public:
        Traducao(Database *);
        ~Traducao();
        
        int preparar_Traducao(const char *);
        void definir_nome_arquivo(const char *);
        
        int ler_evento_entrada();
        int inserir_evento_entrada(EventoEntrada *, int);
};
#endif
