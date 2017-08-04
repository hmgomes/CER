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

#ifndef _RDts_H_
#define _RDts_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "database.h"

class Definicao 
{
    private:
        Database *bd;
        string nome_arquivo;
        fstream arquivo_rdts;
        
        int ncentroides;
    public:
        Definicao(Database *);
        int prepara_Definicao(const char *);
        int prepara_Definicao(int);
        int ler_Distancias();
        long double sqrtll(unsigned long long int value, long double);
        int rand_ll();        
        int ler_rdts();
};
#endif 
