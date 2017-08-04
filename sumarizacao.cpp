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

#include "sumarizacao.h"

Sumarizacao::Sumarizacao(Database* _bd):bd(_bd)
{
   iniciarCausasFRQ();
}


/* Aproveitasse nessa etapa para gerar os candidatos à candidatos 1-C**, ou "causas frequentes",
   estas são aquelas que aparecem frequentemente associadas a uma mesma RDt. Basicamente
   é criado um vector de tamanho |dicionario| * |RDt| (na realidade ele tem os itens inseridos
   dinamicamente, porém jamais ultrapassa este tamanho), e a cada inserção na sumarizada é 
   atualizado a quantidade de causa (que esta inserida)*. 
      * nos casos em que é inserido mais de uma causa para uma mesma SCE é contada apenas
   uma vez, apesar do algoritmo suportar múltiplos eventos no grupo de causa, nesse caso,
   é incorreto contar mais de 1 ocorrência do mesmo evento no mesmo grupo causa, pois esta 
   sendo gerada a lista de candidatos à candidatos 1-C**
      ** Candidatos à candidatos 1-C, pois uma causa apesar de frequente em uma RDt pode
   não ser frequente quando associada a um evento, esta é uma lista intermediaria, aprovei-
   tasse que esta sendo realizado esse "full scan" na tabela Traduzida para minimizar a 
   carga na hora da descoberta, e de ínicio já ter uma lista de causas para associar com os
   efeitos frequentes (ver descoberta.cpp).*/
void Sumarizacao::sumarizar()
{
   MSG << "Iniciando sumarizacao... " << endl;
   /* vector com todas as RDts */
   vector<RDt> rdts;
   bd->selecionar_RDts(rdts);
   MSG << "Quantidade de RDts = " << rdts.size() << endl;
   /* vector com uma sequencia de SE */
   vector<Evento> SE;
   /* Os ID_SCE das Sequencias de Causa e Efeito na tabela Sumarizada são
      inseridos, baseado neste contador */
   int contadorID_SCE = 1;
   /* Quando altera a SCE o contatoID_SCE deve ser incrementado */
   int maxID_SE = bd->selecionar_int(TRADUZIDA_MAX_ID_SE);
   bool novaSCE = false;
   /* impedir que se conte uma mesma causa mais de uma vez. */
   vector<int> causas_inseridas;

   /* O loop mais interno é realizado sobre o conjunto de RDt e o mais externo sobre a tabela SE 
      isso significa que é realizada apenas uma passagem na tabela SE, para todas */


   /* para cada conjunto de SE em traduzida... */
   for(int j = 0 ; j <= maxID_SE ; ++j)
   {
      bd->selecionar(j, SE);
      /* se por um acaso um ID_SE não esta na sequencia (foi 'pulado' na inserção)... */
      if(SE.empty())
         continue;

      /* para todas as RDts definidas... */
      for(int i = 0 ; i < rdts.size() ; ++i)
      {
         /* percorre o conjunto SE em ordem decrescente... */
         for(int e = (int) SE.size()-1 ; e > 0 ; --e)
         {
            /* percorre o conjunto SE em ordem crescente... */
            for(int c = 0 ; c < SE.size() ; ++c)
            {
               /* calcula a distancia do efeito para a causa */
               int distancia = SE[e].TS - SE[c].TS;
               /* se a distancia for maior que a DtMi e menor que a DtMa para
               a RDt, ela deve ser inserida na sumarizada... */
               if(rdts[i].DtMi <= distancia && rdts[i].DtMa >= distancia)
               {
                  /* ID_SCE, ID_causa, ID_efeito, ID_RDt */
                  bd->inserir("sumarizada (ID_SCE, ID_causa, ID_efeito, ID_RDt) ", \
                     contadorID_SCE, SE[c].ID_evento, SE[e].ID_evento, rdts[i].ID_RDt);
                  
                  /* SALVANDO A SUMARIZADA NO DISCO */
                  
                  stringstream ss;
                  ss << "INSERT INTO SUMARIZADA (ID_SCE, ID_causa, ID_efeito, ID_RDt) VALUES (" << contadorID_SCE <<
                     ", " << SE[c].ID_evento << ", " << SE[e].ID_evento << ", " << rdts[i].ID_RDt << ");";
                  bd->executar(ss.str());

                  novaSCE = true;

                  /* atualizacao de 'causas_qtd' */  
                  int _id_causa = SE[c].ID_evento;  
                  bool contar = true;
                  for(int ll = 0 ; ll < causas_inseridas.size() ; ++ll)
                     if(causas_inseridas[ll] == _id_causa)
                     {
                        contar = false; 
                        break;
                     }
                  if(contar)
                     for(int kk = 0 ; kk < causas_qtd.size() ; ++kk)
                        if(causas_qtd[kk].ID_RDt == rdts[i].ID_RDt && causas_qtd[kk].ID_causa == _id_causa)
                        {
                           ++causas_qtd[kk].quantidade;
                           causas_inseridas.push_back(_id_causa);
                        }
               }
            }
            if(novaSCE)
            {
               ++rdts[i].quantidade;
               ++contadorID_SCE;
               novaSCE = false;
               /* mudou a SCE, deve eliminar as restrições de contagem */
               causas_inseridas.clear();
            }
         }
         bd->atualizar("RDt", "quantidade", rdts[i].quantidade, rdts[i].ID_RDt);
         //
      }
      bd->deletar("traduzida", "ID_SE", j);
   }
   // bd->executar_debug("SELECT * FROM bd_memoria.rdt");
   /* Executa os comandos que ainda estiverem no buffer */

   bd->persistir();
   //bd->executar_debug("SELECT * FROM bd_memoria.sumarizada");
   MSG << "Finalizando sumarizacao... " << endl << endl;
}


void Sumarizacao::iniciarCausasFRQ()
{
   /* apenas os ID_evento do dicionario */
   vector<int> dicionario_ids; 
   bd->selecionar_dicionario(dicionario_ids);
   vector<int> rdts_ids;
   bd->selecionar(RDT_SELECT_TODAS_SOMENTE_ID , rdts_ids);
   
   for(int i = 0 ; i < rdts_ids.size() ; ++i)
   {
      for(int j = 0 ; j < dicionario_ids.size() ; ++j)
      {
         CausaQtd x;
         x.ID_RDt = rdts_ids[i];
         x.ID_causa = dicionario_ids[j];
         x.quantidade = 0;
         causas_qtd.push_back(x);
      }
   }
}

/* remove as causas não frequentes. Se uma causa não atende ao SMCE (filtro de SupCE)
   ela não sera capaz de compor uma RCE. */
void Sumarizacao::removerCausasNFRQ(float SMCE)
{
   vector<RDt> rdts;
   bd->selecionar_RDts(rdts);
   vector<int> idx_remocao;
   for(int i = 0 ; i < rdts.size() ; ++i)
   {
      for(int j = 0 ; j < causas_qtd.size() ; ++j)
      {  
         if(causas_qtd[j].ID_RDt == rdts[i].ID_RDt)
         {
            float frq_causa = (float)causas_qtd[j].quantidade / (float)rdts[i].quantidade * 100;
            if(frq_causa < SMCE)
               idx_remocao.push_back(j);
         }
      }
   }
   for(int i = (int) idx_remocao.size()-1 ; i >= 0 ; --i)
      causas_qtd.erase(causas_qtd.begin()+idx_remocao[i]);
}
