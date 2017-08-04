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
#include "utils.h"

using namespace std;

string Utils::hora() {
   string ctime;
   struct tm *local;
   time_t t;
   t = time(NULL);
   local = localtime(&t);
   
   stringstream ss;
   ss << local->tm_hour << "h" << local->tm_min << "m" << local->tm_sec << "s";
   return ss.str();
}

string Utils::hoje() {
   struct tm *local;
   time_t t;
   t = time(NULL);
   local = localtime(&t);
   stringstream ss;
   ss << local->tm_year + 1900 << "-" << local->tm_mon + 1 << "-" << local->tm_mday;
   return ss.str();
}

string Utils::converte_TS(int TS)
{
   string aux;
   stringstream saux;
   int anos = 0, meses = 0, dias = 0, horas = 0, minutos = 0, segundos = 0;
   aux.clear();
   saux.str(std::string()); 
   
   if(TS >= 31104000)
   {
      anos = int(TS/31104000);
      TS = TS % 31104000;
   }
   if(TS >= 2592000)
   {
      meses = int(TS/2592000);
      TS = TS % 2592000;
   }
   if(TS >= 86400)
   {
       dias = int(TS/86400);
       TS = TS % 86400;
   }
   if(TS >= 3600)
   {
      horas = int(TS/3600);
      TS = TS % 3600;
   }
   if(TS >= 60)
   {
      minutos = int(TS/60);
      TS = TS % 60;
   }
   segundos = TS;
   if(anos)
   {
      saux << anos;
      aux = saux.str();
      aux += "ano(s)";
   }
   
   if(meses)
   {
      saux.str(std::string());
      saux << meses;
      aux += saux.str();
      aux += "mes(es)";
   }

   if(dias)
   {
      saux.str(std::string());
      saux << dias;
      aux += saux.str();
      aux += "dia(s)";
   }

   if(horas)
   {
      saux.str(std::string()); 
      saux << horas;    
      aux += saux.str();
      aux += "hs";
   }

   if(minutos)
   {
      saux.str(std::string()); 
      saux << minutos;    
      aux += saux.str();
      aux += "min";
   }

   if(segundos)
   {
      saux.str(std::string()); 
      saux << segundos;    
      aux += saux.str();
      aux += "seg";
   }
   
   return aux;
}
