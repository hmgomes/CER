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

#include "descoberta.h"

Descoberta::Descoberta(Database* _bd, vector<CausaQtd>& _causas_FRQ, float _SMCE, float _CMCE, \
   float _FME, int _MC, int _MAX_COMANDOS):bd(_bd), causas_FRQ(_causas_FRQ), FME(_FME), SMCE(_SMCE), CMCE(_CMCE), \
   MC(_MC), geracao(0)
{
   ArvoreCandidatos::MAX_COMANDOS = _MAX_COMANDOS;
   bd->selecionar_RDts(rdts);
   bd->persistir_RDts(rdts);
   bd->selecionar_dicionario(dicionario);
   bd->persistir_dicionario();
}

/* Efetua a contagem e aplica o filtro de frequencia de efeito quando aplicavel.*/
int Descoberta::filtroEfeito()
{
   MSG << "Iniciando filtro de Frequencia de Efeito..." << endl;
   //bd->selecionar_RDts(rdts); é efetuado durante a criação do objeto (ver construtor) 
   bd->inserir_efeitos_FRQ(FME, &rdts);
   bd->selecionar_efeitos(efeitos_FRQ);
   /* esse método insere uma "cópia" da tabela Efeito em memória para o Disco */
   bd->persistir_efeito();
   MSG << "Encerrando filtro de Frequencia de Efeito..." << endl << endl;
    return 0;
}

/* Une as causas frequentes + os efeitos_FRQ frequentes, formando a primeira
   geração de candidatos ( 1-C ). */
void Descoberta::gerar1C()
{
   for(int i = 0 ; i < efeitos_FRQ.size() ; ++i)
   {
      int k;
      for(k = 0 ; k < rdts.size() ; ++k)
         if(rdts[k].ID_RDt == efeitos_FRQ[i].ID_RDt)
            break;
      
      ArvoreCandidatos ac(bd->getBD(), rdts[k], efeitos_FRQ[i], &dicionario);
      for(int j = 0 ; j < causas_FRQ.size() ; ++j)
      {
         if(causas_FRQ[j].ID_RDt == efeitos_FRQ[i].ID_RDt)
         {
            NoCandidato nc(causas_FRQ[j].ID_causa);
            ac.candidatos.push_back(nc);
         }
      }
      arvoresCandidatos.push_back(ac);
   }
   geracao = 1;
}

int Descoberta::descobertaRCE()
{
   filtroEfeito();
   MSG << "Iniciando Descoberta de RCE..." << endl;
   gerar1C();
   MSG << "Quantidade inicial de arvores de candidatos = " << arvoresCandidatos.size() << endl;
   MSG << "Quantidade de RDts = " << rdts.size() << endl;
   MSG << "Quantidade de causas_FRQ = " << causas_FRQ.size() << endl;
   MSG << "Quantidade de efeitos_FRQ = " << efeitos_FRQ.size() << endl;
   MSG << "Quantidade de registros na Sumarizada = " << bd->selecionar_int("SELECT count(1) FROM bd_memoria.sumarizada") << endl;
   while(geracao <= MC && !arvoresCandidatos.empty())
   {
      MSG << "Geracao = " << geracao << endl;
      MSG << "Quantidade de arvores de candidatos = " << arvoresCandidatos.size() << endl;
      //MSG << "Quantidade de registros na Sumarizada = " << qtd_reg_sumarizada << endl;
      vector<int> idx_remocao;
      MSG << "Calculando arvore... ";
      for(int i = 0 ; i < arvoresCandidatos.size() ; ++i)
      {
         /*retirar os comentarios de arvores...mostrar() permite a visualização das árvores 
            durante a execução */
         //arvoresCandidatos[i].mostrar();
         /* ativar uma árvore significa "solicitar" uma nova contagem, cálculo SupConfCE e geração */
         arvoresCandidatos[i].ativar(SMCE, CMCE);
         /* Checa se esta árvore esta válida ainda (contém novos candidatos) */
         if(!arvoresCandidatos[i].valida)
            idx_remocao.push_back(i);
         //arvoresCandidatos[i].mostrar();
         //getchar();
         cout << i+1 << ",";
      }
      cout << endl << endl;
      for(int i = (int) idx_remocao.size()-1 ; i >= 0 ; --i)
         arvoresCandidatos.erase(arvoresCandidatos.begin() + idx_remocao[i]);
      geracao++;
      // Insere as regras 
      ArvoreCandidatos::persistir(arvoresCandidatos[0].bd);
   }
   MSG << "Finalizando Descoberta de RCE..." << endl << endl;
    return 0;
}
