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
#include "arvore_candidatos.h"

int ArvoreCandidatos::MAX_COMANDOS = 100000;
int ArvoreCandidatos::qtd_comandos = 0;
string ArvoreCandidatos::comandos = "BEGIN TRANSACTION;";

void ArvoreCandidatos::ativar(float SMCE, float CMCE)
{
   contarOcorrencias();
   calculoSupConfCE(SMCE, CMCE);
   novaGeracao();
   validar();
}

/* Se a árvore não esta válida, ela é removida a partir da classe Descoberta */
void ArvoreCandidatos::validar()
{
   /* Assume que a árvore não esta válida, até que um filho confirme a validade */
   valida = false;
   for(int i = 0 ; i < candidatos.size() ; ++i)
      validar(candidatos[i], 0);
}

void ArvoreCandidatos::validar(NoCandidato& no, int profundidade)
{
   /* se um filho esta no nivel igual a geracao-1 a arvore é válida */
   if(profundidade == (geracao-1))
   {
      valida = true;
      return;
   }
   else
      for(int i = 0 ; i < no.candidatos.size() ; ++i)
         validar(no.candidatos[i], ++profundidade);
}

/* Remover SCE da tabela sumarizada, apesar de diminuir o espaço de busca, se mostrou ineficiente
pois o comando de "delete", mesmo quando efetuado contra um banco residente na memória principal
é muito ineficiente, em outras palavras, demora muito para executar. */
void ArvoreCandidatos::contarOcorrencias()
{
   //MSG_ID << "Iniciando selecao e contagem ocorrencias de candidatos, geracao = " << geracao << endl;

   SQLite::preparar(bd, SUMARIZADA_SELECT_DESCOBERTA, &stmtSumarizada);
   vector<int> causas;

   SQLite::anexar_int(stmtSumarizada, 1, rdt.ID_RDt);
   SQLite::anexar_int(stmtSumarizada, 2, efeito.ID_efeito);
   
   SQLite::passo(stmtSumarizada);

   int ID_SCE = SQLite::valor_coluna_int(stmtSumarizada, 0);
   causas.push_back(SQLite::valor_coluna_int(stmtSumarizada, 1));
   while(SQLite::passo(stmtSumarizada) == SQLITE_ROW)
   {
      if(ID_SCE == SQLite::valor_coluna_int(stmtSumarizada, 0))
         causas.push_back(SQLite::valor_coluna_int(stmtSumarizada, 1));
      else
      {
         if(causas.size() >= geracao)
            match(causas);
         /*else
            deletar_SCE(ID_SCE);*/
         ID_SCE = SQLite::valor_coluna_int(stmtSumarizada, 0);
         causas.clear();
         causas.push_back(SQLite::valor_coluna_int(stmtSumarizada, 1));
      }
   }
   /* Ultima SCE retornada... */
   if(causas.size() >= geracao)
      match(causas); 
   /*else
      deletar_SCE(ID_SCE);*/

   SQLite::finalizar(stmtSumarizada);
   
   //MSG_ID << "Finalizando selecao e contagem ocorrencias de candidatos, geracao = " << geracao << endl << endl;
}

void ArvoreCandidatos::calculoSupConfCE(float SMCE, float CMCE)
{
   if(geracao == 1)
   {
      vector<int> idx_remocao;
      for(int i = 0 ; i < candidatos.size() ; ++i)
      {
         float supCE = candidatos[i].quantidade / (float) rdt.quantidade * 100;
         if(supCE >= SMCE)
         {
            float confCE = candidatos[i].quantidade / (float) efeito.quantidade * 100;
            if(confCE >= CMCE)
            {
               string causa = "'" + (dicionario->find(candidatos[i].ID_causa))->second + "'";
               inserir_RCE(rdt.ID_RDt, efeito.ID_efeito, candidatos[i].quantidade, geracao, causa, supCE, confCE);
            }
            else
               idx_remocao.push_back(i);
         }
         else
            idx_remocao.push_back(i);
      }
      /* Muito importante: remove de trás pra frente os itens do nó. 
         Isso é preciso pois utiliza-se um 'offset', logo remover os
         primeiros antes, invalida os indices em idx_remocao */
      for(int i = (int) idx_remocao.size()-1 ; i >= 0 ; --i)
         candidatos.erase(candidatos.begin() + idx_remocao[i]);
   }
   else
   {
      vector<int> dummy;
      for(int i = 0 ; i < candidatos.size() ; ++i)
         calculoSupConfCE(SMCE, CMCE, candidatos[i], dummy, 0, rdt, efeito);
   }
}

/* causas anteriores é passada por cópia pois os irmãos precisam de uma versão igual a
   repassada pelo pai, e.g. sem alterações feitas por um irmão à esquerda. */
void ArvoreCandidatos::calculoSupConfCE(float SMCE, float CMCE, NoCandidato& no, \
   vector<int> causas_anteriores, int profundidade, RDt& rdt, Efeito& efeito)
{
   causas_anteriores.push_back(no.ID_causa);
   /* se esta na geracao anterior a ultima geracao */
   if(++profundidade == (geracao-1))
   {
      vector<int> idx_remocao;
      for(int i = 0 ; i < no.candidatos.size() ; ++i)
      {
         /* faz uma copia para cada filho */
         vector<int> causas_filho = causas_anteriores;
         float supCE = no.candidatos[i].quantidade / (float) rdt.quantidade * 100;
         if(supCE >= SMCE)
         {
            float confCE = no.candidatos[i].quantidade / (float) efeito.quantidade * 100;
            if(confCE >= CMCE)
            {
               causas_filho.push_back(no.candidatos[i].ID_causa);
               string causa = traduzCausa(causas_filho);
               inserir_RCE(rdt.ID_RDt, efeito.ID_efeito, no.candidatos[i].quantidade, \
                  geracao, causa, supCE, confCE);
            }
            else
               idx_remocao.push_back(i);
         }
         else
            idx_remocao.push_back(i);
      }
      for(int i = (int) idx_remocao.size()-1 ; i >= 0 ; --i)
         no.candidatos.erase(no.candidatos.begin() + idx_remocao[i]);
   }
   else
      for(int i = 0 ; i < no.candidatos.size() ; ++i)
         calculoSupConfCE(SMCE, CMCE, no.candidatos[i], \
            causas_anteriores, profundidade, rdt, efeito);
}


/* Criação de geração n-C. Um caso especial é a 2-C que é gerada diretamente aqui,
   a partir desta, é feita uma chamada recursiva a novaGeracao(NoCandidato&, int) */
void ArvoreCandidatos::novaGeracao()
{
   //MSG_ID << "Iniciando nova geracao de candidatos, geracao = " << geracao << endl;
   if(++geracao == 2)
   {
      vector<NoCandidato> copiaCandidatos = candidatos;
      for(int i = 0 ; i < candidatos.size() ; ++i)
      {
         candidatos[i].candidatos = copiaCandidatos;
         copy(copiaCandidatos.begin()+1, copiaCandidatos.end(), copiaCandidatos.begin());
          // Por algum motivo fazia esse erase do end(), claramente invalido.
          // Foi retirado na versao do mac por causar um erro em tempo de execucao (justo...)
          // Pode ser que houvesse alguma logica relacionada ao remover este elemento,
          // isso nao foi checado... Porem, visto que nas versoes antigas do windows
          // nao havia como isso remover algo (end nao aponta pra um objeto),
          // aparentemente as versoes estao em sincronia e esse codigo foi apenas um
          // delirio de um desenvolvedor com sono.
//         copiaCandidatos.erase(copiaCandidatos.end());
      }
   }
   else
      for(int i = 0 ; i < candidatos.size() ; ++i)
         novaGeracao(candidatos[i], 0);
   //MSG_ID << "Finalizando nova geracao de candidatos, geracao = " << geracao << endl << endl;
}

/* Quando a profundidade (nível atual na árvore) é igual a dois níveis antes daonde
   se espera que a nova geração passe a existir (no nível 'geracao') é feita a 
   inserção dos novos candidatos (combinando os candidatos da geração anterior). */
void ArvoreCandidatos::novaGeracao(NoCandidato& no, int profundidade)
{
   if(++profundidade == (geracao-2))
      for(int i = 0 ; i < no.candidatos.size() ; ++i)
         for(int j = i ; j < no.candidatos.size() ; ++j)
            no.candidatos[i].candidatos.push_back( no.candidatos[j] );
   else
      for(int i = 0 ; i < no.candidatos.size() ; ++i)
         novaGeracao(no.candidatos[i], profundidade);
}

/* Recebe as causas de uma SCE (já filtrada pela mesma RDt e Efeito desta árvore)
   e atualiza a quantidade de ocorrências para os candidatos que realizarem
   'match' com a SCE. */
void ArvoreCandidatos::match(vector<int>& _SCE)
{
   /* Para todos os candidatos na raiz... */
   for(int i = 0 ; i < candidatos.size() ; ++i)
      match(candidatos[i], _SCE);
}

/* Se a "causa" (nó) atual não existir na SCE, esta SCE não é capaz de realizar match
   com nenhum candidato desta árvore (não vale a pena continuar descendo). */
/* Caso ela realize o match é preciso remover da SCE a causa que combinou com
   o nó atual, para evitar realizar match entre um candidato de comprimento 
   (qtd de causas) maior que o da SCE e também situações em que o candidato 
   repete o mesmo evento (o algoritmo opera com múltiplas ocorrências de eventos). */
void ArvoreCandidatos::match(NoCandidato& no, vector<int> _SCE)
{
    for(int i = 0 ; i < _SCE.size() ; ++i) {
      if(no.ID_causa == _SCE[i])
         if(no.candidatos.empty()) {
            ++no.quantidade;
            return;
         }
         else {
            vector<int> copiaSCE = _SCE;
            copiaSCE.erase(copiaSCE.begin() + i);
            for(int j = 0 ; j < no.candidatos.size() ; ++j)
               match(no.candidatos[j], copiaSCE);
            return;
         }
    }
}

/****************************** TEST METHODS ******************************/
void ArvoreCandidatos::mostrar()
{
   MSG_ID << "Mostrando arvore RDt " << rdt.ID_RDt << " Efeito = " << (dicionario->find(efeito.ID_efeito))->second << endl;
   if(candidatos.empty())
   {
      MSG_ID << "Apenas no raiz!" << endl;
      return;
   }
   for(int i = 0 ; i < candidatos.size() ; ++i)
   {
      cout << "[" << (dicionario->find(candidatos[i].ID_causa))->second << "](" << candidatos[i].quantidade << ")-";
      mostrar(candidatos[i], "\t");
      cout << "\n";
   }
}

void ArvoreCandidatos::mostrar(NoCandidato& c, string tab)
{
   if(c.candidatos.empty())
      return;
   for(int i = 0 ; i < c.candidatos.size() ; ++i)
   {
      cout << tab << (dicionario->find(candidatos[i].ID_causa))->second << "(" << c.candidatos[i].quantidade << ")-\n";
      mostrar(c.candidatos[i], tab + "\t");
   }
}

/* Quantos NoCandidato em cada geração */
void ArvoreCandidatos::status()
{
   cout << "Geracao[1] = " << candidatos.size();
   getchar();
   if(candidatos.empty()) return;
   for(int i = 0 ; i < candidatos.size() ; ++i)
   {
      status(candidatos[i], 1, i, "\t");
   }
}

void ArvoreCandidatos::status(NoCandidato& c, int geracaoPassada, int no, string tab)
{
   int geracao = geracaoPassada + 1;
   if(c.candidatos.empty()) 
      return;
   cout << tab << "No[" << no << "]. Geracao[" << geracao <<"] = " << c.candidatos.size() << endl;
   for(int i = 0 ; i < c.candidatos.size() ; ++i)
      status(c.candidatos[i], geracao, i, tab + "\t");
}

/****************************** MÉTODOS DE MANIPULAÇÃO DA BASE ******************************/

string ArvoreCandidatos::traduzCausa(vector<int> causas)
{
   stringstream c;
   c << "'";
   for(int i = 0 ; i < causas.size() ; ++i)
   {
      c << (dicionario->find(causas[i]))->second << "-";
   }
   c << "'";
   return c.str();
}

void ArvoreCandidatos::inserir_RCE(int ID_RDt, int ID_efeito, int quantidade, int comprimento, string causa, float supCE, float confCE)
{
   stringstream str;
   str << "INSERT INTO RCE (ID_RDt, ID_efeito, causa, quantidade, comprimento, supcE, confCE) VALUES ("\
      <<ID_RDt<<","<<ID_efeito<<","<<causa<<","<<quantidade<<","<<comprimento<<","<<supCE<<","<<confCE<<");\n";
   enfileirar(str.str(), bd);
}

/* este metodo foi desabilitado, pois apesar de diminuir o espaco de busca 
removendo registros da tabela sumarizada, o tempo gasto para realizar a remocao nao
vale a pena os beneficios que ela tras. */
/*void ArvoreCandidatos::deletar_SCE(int ID_SCE)
{
   stringstream str;
   str << "DELETE FROM bd_memoria.sumarizada WHERE ID_SCE = " << ID_SCE << ";\n";
   enfileirar(str.str(), bd);
}*/

void ArvoreCandidatos::enfileirar(string comando, sqlite3* bd)
{
   comandos += comando;
   qtd_comandos++;
   if(qtd_comandos > MAX_COMANDOS)
      persistir(bd);
}

void ArvoreCandidatos::persistir(sqlite3* bd)
{
   comandos += "END TRANSACTION;\n";
   //cout << comandos; getchar();
   SQLite::executar(bd, comandos.c_str());
   qtd_comandos = 0;
   comandos = "BEGIN TRANSACTION;\n";
}
