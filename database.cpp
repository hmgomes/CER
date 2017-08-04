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

#include "database.h"

using namespace std;

/* Cria o database, inicia o buffer de comandos com "BEGIN TRANSACTION;" */
Database::Database(int _MAX_COMANDOS, string _nomeBD):comandos("BEGIN TRANSACTION;"), qtd_comandos(0),
   MAX_COMANDOS(_MAX_COMANDOS), nomeBD(_nomeBD)
{
   criar();
}

/* Encerra a conexão com o database */
Database::~Database()
{
   /* encerra o stmt, pois se houverem stmt 'abertos', o banco não pode ser fechado com sucesso 
      mesmo o banco estando apenas em memória é aconselhavel abri-lo e fecha-lo de maneira controlada */
   SQLite::fechar(bd);
}

/* Imprimi o resultado de uma query na tela.
   Esta função é passada para o método SQLite::executar. A função sqlite3_exec
   (encapsulada dentro de SQLite::executar) chama esta função de callback para cada 
   row resultante da query submetida */
int Database::callback_debug(void *naoUsado, int argc, char **argv, char **coluna)
{
   for(int i = 0; i < argc; i++)
   {
     if(argv[i])
       cout << argv[i] << "\t";
     else
       cout << "NULL" << "\t";
   }
   cout << endl;
   return 0;
}

/* Aplica o filtro de frequencia_FME, este método é chamado para cada resultado
   da query SUMARIZADA_COUNT_EFEITOS (ver utils.h), a qual retorna uma linha para
   cada efeito contendo: o id_efeito, id_rdt e a quantidade de ocorrencia do efeito na rdt
   o parametro argv contendo os valores retornados, cada qual representando: 
      argv[0] = id_efeito, 
      argv[1] = id_rdt, 
      argv[2] = count(1) (quantidade) 
   o parametro *parametros é um ponteiro para a struct ParametroFME, esta struct
   tem um ponteiro para um vector com todas as RDts e o valor do filtro FME. 
   As SCE que contém efeitos não frequentes são removidas da tabela Sumarizada. 
   NOTA: como essa é uma função estatica ela não tem acesso aos metodos nao estaticos
      da classe Database senão por meio de uma instancia, para sobrepujar isso um 
      ponteiro para o objeto Database ja instanciado é passado para função por
      meio da struct ParametroFME */
/* TODO: colocar um vector<RDt>::iterator para a posição da RDt deste efeito em ParametroFME */
int Database::callback_filtrar_FME(void *parametros, int argc, char **argv, char **coluna)
{
   ParametroFME* p = (ParametroFME*)parametros;
   /* Encontra a RDt associada ao evento efeito analisado... */
   int i = 0;
   for( ; i < (p->rdts)->size() ; ++i)
   {
      if((*(p->rdts))[i].ID_RDt == atoi(argv[1]))
         break;
   }
   /* Calcula a frequencia do efeito para a RDT a qual ele esta associado */
   float frequencia_efeito = (float)atoi(argv[2]) / (float)(*(p->rdts))[i].quantidade * 100;
   /* Se a frequencia do efeito for >= do que a Frequencia Minima de Efeito (FME) ele é inserido em Efeito */
   if(frequencia_efeito >= p->FME)
   {
      stringstream comando_in;
      /* argv[0] (ID_efeito), argv[1] (ID_RDt), argv[2] (quantidade) */
      comando_in << EFEITO_INSERT << argv[0] << "," << argv[1] << "," << argv[2] << "," \
         << frequencia_efeito << ");";
      /* O ponteiro para um objeto Database é necessário pois este é um método estático e
         não pode chamar diretamente um método membro (como Database::enfileirar) */
      (p->bd)->enfileirar(comando_in.str());
   }
   else
   {
      /* O efeito não é frequente, logo é melhor remover as SCE associadas a ele 
         para a RDt onde ele não é frequente. Isso melhora o tempo de descoberta de RCE. */
      stringstream comando_rm;
      /* DELETE FROM bd_memoria.sumarizada WHERE ID_efeito = 'argv[0]' AND ID_RDt = 'argv[1]'; */
      comando_rm << SUMARIZADA_DELETE_EFEITOS_NAO_FRQ_PREFIXO << argv[0] << \
         SUMARIZADA_DELETE_EFEITOS_NAO_FRQ_SUFIXO << argv[1] << ";";
      (p->bd)->enfileirar(comando_rm.str());
   }
   return 0;
}

/* Cria o banco de dados */
void Database::criar()
{
   /* se nao foi informado um nome pra saida, é utilizado "entrada + data_hora" */
   nomeBD = nomeBD == "" ? nomeArquivoEntrada : nomeBD;
   nomeBD += " " + Utils::hoje()+ "_" + Utils::hora() + ".sqlite";
   MSG << "Iniciando criacao da base de dados... " << endl;
   /* Esta primeira chamada cria o arquivo do banco, assume-se que ele não existe.
      Para evitar essa situação é concatenada a data+hora atual no nome do arquivo do banco */
   SQLite::abrir(&bd, nomeBD.c_str(), SQLITE_OPEN_CREATE);
   SQLite::fechar(bd);
   /* Abri o arquivo no modo leitura e escrita */
   SQLite::abrir(&bd, nomeBD.c_str(), SQLITE_OPEN_READWRITE);

   /* 1. Cria a estrutura da base de dados na memoria secundaria ('main') 
      2. Cria um banco de dados na memoria principal ('db_memoria') e
         anexa ('attach') ele ao banco de dados da memoria secundaria ('main') 
      Obs: durante a criação do banco o método executar é utilizado diretamente. 
      Outros métodos "enfileram" os comandos */
   SQLite::executar(bd, DATABASE_CREATE);

   MSG << "Finalizando criacao da base de dados... " << endl << endl;
}

/* Executa comando(s) no BD (mais de 1 devem ser separados por ; e 
      "query" deve obrigatoriamente iniciar por "BEGIN TRANSACTION;"
      e terminar com "END TRANSACTION;" 
   NOTA: ao executar vários comandos */
int Database::executar(string query)
{
   return SQLite::executar(bd, query.c_str());
}

/* Executa uma query no BD, a qual retorna apenas uma única linha/coluna (tipo de dado "int"). 
   Útil para utilização de funções como COUNT(*), SUM(X), MAX(X), MIN(X). */
int Database::selecionar_int(string query)
{
   sqlite3_stmt *stmt;
   int ret = 0;

   if( SQLite::preparar(bd, query.c_str(), &stmt) != SQLITE_OK )
      return 1;
   if( SQLite::passo(stmt) == SQLITE_ROW)
      ret = SQLite::valor_coluna_int(stmt, 0);
   
   SQLite::finalizar(stmt);
   return ret;
}

/* mesma funcionalidade que o método selecionar_int(string) porém 
   este aceita um valor para restringir a sql presente na query */
int Database::selecionar_int(string query, int value){
   stringstream dint;
   sqlite3_stmt *stmt;
   int ret = 0;

   dint.str(std::string()); 
   dint << value;    
   
   query += dint.str();
   query += ";";


   if( SQLite::preparar(bd, query.c_str(), &stmt) != SQLITE_OK )
      return 1;
      
   if( SQLite::passo(stmt) == SQLITE_ROW)
      ret = SQLite::valor_coluna_int(stmt, 0);

   SQLite::finalizar(stmt);
   return ret;
    
}

/* Executa uma query no BD, a qual retorna apenas uma única linha/coluna (tipo de dado "long long int"). 
   Útil para utilização de funções como COUNT(*), SUM(X), MAX(X), MIN(X). */
unsigned long long int Database::selecionar_llint(string query)
{
   sqlite3_stmt *stmt;
   unsigned long long int ret = 0;

   if( SQLite::preparar(bd, query.c_str(), &stmt) != SQLITE_OK )
      return 1;
      
   if( SQLite::passo(stmt) == SQLITE_ROW)
      ret = SQLite::valor_coluna_int64(stmt, 0);

   SQLite::finalizar(stmt);
   
   return ret;
}

/* Executa um comando de inserção na base de dados (não é imediatamente)
   Monta a SQL a partir da entrada: nome da tabela, um vetor de campos e de valores.
   NOTA: valores "TEXT" devem estar entre aspas simples no vetor de valores. 
   NOTA: atentar a ordem em que os valores e as colunas são inseridas 
   NOTA: Os comandos são enfileirados e só são executados quando sua quantidade 
      ultrapassa MAX_COMANDOS, ou quando é feita uma chamada explicita
      a "persistir()" */
int Database::inserir(string tabela, vector<string> campos, vector<string> valores)
{
   string prefixo = "INSERT INTO bd_memoria." + tabela + " (", sufixo;
   for (int i = 0 ; i < campos.size() ; ++i)
   {
      prefixo += campos[i] + ",";
      sufixo  += valores[i] + ",";
   }
   prefixo = prefixo.substr(0, prefixo.size()-1) + ") VALUES (";
   sufixo = sufixo.substr(0, sufixo.size()-1) + ");";

   return enfileirar(prefixo + sufixo);
}

/* Insere registros em uma tabela, recebendo os valores e a string contendo
   a tabela e em quais colunas tais valores devem ser inseridos. 
   Se forem utilizados todos os campos da tabela, não há necessidade de 
   adiciona-los no parametro tabelaCampos (atentar a ordem que eles serão inseridos).
   Exemplo:
      bd->inserir("RDt (DtMi, DtMa, quantidade) ", valores); 
      bd->persistir(); */
int Database::inserir(string tabelaCampos, vector<int> valores)
{
   //char buffer[40];
   string prefixo = "INSERT INTO bd_memoria." + tabelaCampos + " VALUES (", sufixo;
   for (int i = 0 ; i < valores.size() ; ++i)
   {
       stringstream ss;
       ss << valores[i] << ",";
     // sufixo += itoa(valores[i], buffer, 10);
     // sufixo += ",";
       sufixo += ss.str();
   }
   sufixo = sufixo.substr(0, sufixo.size()-1);
   sufixo += ");";

   return enfileirar(prefixo + sufixo);
}

/* Analogo a inserir(string, vector<int>) porem especifico para 4 valores apenas (mais rapida) */
int Database::inserir(string tabelaCampos, int vlr1, int vlr2, int vlr3, int vlr4)
{
   string str = "INSERT INTO bd_memoria." + tabelaCampos + " VALUES (";
   
    stringstream ss;
    ss << vlr1 << "," << vlr2 << "," << vlr3 << "," << vlr4 << ");";
    /*
   str += itoa(vlr1, buffer, 10);
   str += ",";
   str += itoa(vlr2, buffer, 10);
   str += ",";
   str += itoa(vlr3, buffer, 10);
   str += ",";
   str += itoa(vlr4, buffer, 10);
   str += ");";
     */
    str += ss.str();
   return enfileirar(str);
}

/* Executa uma inserção no BD. Tem como entrada o nome da tabela, a coluna
   e o valor, respectivamente.
  NOTA: valores "TEXT" devem estar entre aspas simples (valor). */
int Database::inserir(string tabela, string campo, string valor)
{
   string stm = "INSERT INTO bd_memoria." + tabela + " (" + campo + ") VALUES (" + valor + ");";
   return enfileirar(stm);
}

/* Efetua a contagem de ocorrência de efeito por RDt */
/* É utilizada uma função de callback para efetuar o filtro (ver callback_filtrar_FME()) */
int Database::inserir_efeitos_FRQ(float FME, vector<RDt>* rdts)
{
   ParametroFME pr;
   pr.bd = this;
   pr.FME = FME;
   pr.rdts = rdts;
   int rc = SQLite::executar(bd, SUMARIZADA_COUNT_EFEITOS, callback_filtrar_FME, (void*)&pr);
   /* caso algum comando ainda esteja no buffer (comandos) */
   persistir();
   return rc;
}

/* Atualiza valores para a tabela especificada. Recebe a tabela, quais campos devem
   ser atualizados, com quais valores. A restrição para o update é o valor do rowid. 
   Este método não permite utilizar outra restrição além do rowid, logo este deve 
   ser recuperado em um select anterior a chamada desta. Por exemplo: se for para atualizar
   os valores da tabela RDt, atualizando a quantidade de SCE por RDt (campo 'quantidade')
   primeiro recupera as RDts (selecionar(..., vector<RDt>) o qual já retorna o rowid)
   depois chama o método atualizar para cada uma das RDt, informando a tabela e 
   os campos a serem atualizados (no caso apenas quantidade) */
int Database::atualizar(string tabela, vector<string> campos, vector<int> valores, \
   int rowid)
{
    
   string prefixo = "UPDATE bd_memoria." + tabela + " SET ", sufixo;
   for(int i = 0 ; i < campos.size() ; ++i)
   {
       stringstream ss;
       ss << campos[i] << " = " << valores[i] << " AND ";
      // prefixo += campos[i] + " = ";
   //   prefixo += itoa(valores[i], buffer, 10);
   //   prefixo += " AND ";
       prefixo += ss.str();
   }
   /* remove o ultimo AND */
   prefixo = prefixo.substr(0, prefixo.size()-4);
   prefixo +=sufixo += " WHERE rowid = ";
    stringstream ss;
    ss << rowid;
    prefixo += ss.str();
//   prefixo += itoa(rowid, buffer, 10);
   prefixo += ";";
   
   return enfileirar(prefixo);
}

/* Atualizar de apenas um campo */
int Database::atualizar(string tabela, string campo, int valor, int rowid)
{
   string prefixo = "UPDATE bd_memoria." + tabela + " SET " + campo + " = ";
    stringstream ss;
    ss << valor << " WHERE rowid = " << rowid << ";";
    prefixo += ss.str();
   return enfileirar(prefixo);
}

/* Enfileira no arquivo de "comandos" uma nova query as ser executada. */
int Database::enfileirar(string comando)
{
   /* Adiciona um comando ao final do comandos, ter mais de 1 ; não causa problemas
      agora não ter nenhum faz com que a transação falhe! */
   comandos += comando + ";";
   qtd_comandos++;
   /* Se for ultrapassar o limite maximo de comandos, 
      então persiste as atualizações/inserções no banco */
   if(qtd_comandos > MAX_COMANDOS)
      return persistir();
    return 0;
}

/* Executa os comandos presentes no buffer. */
int Database::persistir()
{
   comandos += "END TRANSACTION;";
   int rc = executar(comandos.c_str());
   /* Prepara 'comandos' para chamadas posteriores */
   qtd_comandos = 0;
   comandos = "BEGIN TRANSACTION;";
   return rc;
}

int Database::persistir_dicionario()
{
   string str = "INSERT INTO dicionario (ID_evento, descricao) SELECT ID_evento, descricao FROM bd_memoria.dicionario";
   SQLite::executar(bd, str.c_str());
    return 0;
}

int Database::persistir_efeito()
{
   string str = "INSERT INTO efeito SELECT * FROM bd_memoria.efeito";
   SQLite::executar(bd, str.c_str());
    return 0;
}

int Database::persistir_RDts(vector<RDt>& rdts)
{
   stringstream str;
   str << "INSERT INTO RDt (ID_RDt, DtMi, DtMa, quantidade) VALUES (";
   for(int i = 0 ; i < rdts.size() ; ++i)
   {
      stringstream temp;
      temp << str.str() << rdts[i].ID_RDt << ",'" << Utils::converte_TS(rdts[i].DtMi) << "','" << \
         Utils::converte_TS(rdts[i].DtMa) << "'," << rdts[i].quantidade << ");";
      SQLite::executar(bd, temp.str().c_str());
   }
    return 0;
}

/* Executa query no BD e insere os resultados no vector 'ret' (string) */
void Database::selecionar(string query, vector<string> &resultado)
{
   sqlite3_stmt *stmt;
   resultado.clear();
   SQLite::preparar(bd, query.c_str(), &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
   {
      const char *val = (const char*)SQLite::valor_coluna_texto(stmt,0);
      resultado.push_back(val);
   }
   SQLite::finalizar(stmt);
}

/* Executa query no BD e insere os resultados no vector 'ret' (int) 
   NOTA: o vector 'ret' tem quaisquer itens inseridos anteriormente 
      removidos por este método */
void Database::selecionar(string query, vector<int> &resultado)
{
   sqlite3_stmt *stmt;
   resultado.clear();
   SQLite::preparar(bd, query.c_str(), &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
      resultado.push_back(SQLite::valor_coluna_int(stmt,0));
   SQLite::finalizar(stmt);
}

/* retorna todas as RDts */
void Database::selecionar_RDts(vector<RDt>& rdts)
{
   sqlite3_stmt *stmt;
   rdts.clear();
   /* oid, DtMi, DtMa, quantidade */
   SQLite::preparar(bd, RDT_SELECT_TODAS, &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
   {
      /* SELECT oid, DtMi, DtMa, quantidade FROM bd_memoria.RDt; */
      /* RDt(int _ID_RDt, int _DtMi, int _DtMa, int _quantidade):\ */
      RDt rdt(SQLite::valor_coluna_int(stmt, 0), SQLite::valor_coluna_int(stmt, 1),\
         SQLite::valor_coluna_int(stmt, 2), SQLite::valor_coluna_int(stmt, 3));
      rdts.push_back(rdt);


   }
   SQLite::finalizar(stmt);
}

/* retorna todos os efeitos frequentes (na tabela Efeito) */
void Database::selecionar_efeitos(vector<Efeito>& efeitos)
{
   sqlite3_stmt *stmt;
   efeitos.clear();
   SQLite::preparar(bd, EFEITO_SELECT_TODOS, &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
   {
      Efeito efeito(0, SQLite::valor_coluna_int(stmt, 0), SQLite::valor_coluna_int(stmt, 1),\
         SQLite::valor_coluna_int(stmt, 2), SQLite::valor_coluna_float(stmt, 3));
      efeitos.push_back(efeito);
   }
   SQLite::finalizar(stmt);
}


/* Apenas os ID_evento do dicionario. */
void Database::selecionar_dicionario(vector<int>& dicionario)
{
   sqlite3_stmt *stmt;
   dicionario.clear();
   SQLite::preparar(bd, DICIONARIO_SELECT_TODAS_ID_EVENTO, &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
      dicionario.push_back(SQLite::valor_coluna_int(stmt, 0));
   SQLite::finalizar(stmt);   
}

/* ID_evento e descricao do dicionario. */
void Database::selecionar_dicionario(map<int, string>& dicionario)
{
   sqlite3_stmt *stmt;
   dicionario.clear();
   SQLite::preparar(bd, DICIONARIO_SELECT_TODAS, &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
      dicionario.insert(pair<int, string>(SQLite::valor_coluna_int(stmt, 0), \
         (const char*)SQLite::valor_coluna_texto(stmt, 1)));
   SQLite::finalizar(stmt);   
}

/* registros da sumarizada limitados pelo ID_RDt */
void Database::selecionar_sumarizada(vector<RSCE>& pSumarizada, int ID_RDt)
{
   sqlite3_stmt *stmt;
   pSumarizada.clear();
   /* ID_SCE, ID_efeito, ID_causa */
   string sql = SUMARIZADA_SELECT_POR_ID_RDT;
    stringstream ss;
    ss << ID_RDt << ";";
   //sql += itoa(ID_RDt, buffer, 10);
//   sql += ";";
    sql += ss.str();
   SQLite::preparar(bd, sql.c_str() , &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
   {
      RSCE rsce(0, SQLite::valor_coluna_int(stmt, 0), SQLite::valor_coluna_int(stmt, 1), \
         SQLite::valor_coluna_int(stmt, 2), ID_RDt);
      pSumarizada.push_back(rsce);
   }
   SQLite::finalizar(stmt); 
}

void Database::selecionar(string _descricao, vector<Dicionario>& DI)
{
   sqlite3_stmt *stmt;
   DI.clear();
   string query = DICIONARIO_SELECT_POR_DESCRICAO;
   query += _descricao.c_str();
   query += "');";
  //MSG << query << endl;
   SQLite::preparar(bd, query.c_str(), &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
   {
      /* primeiro valor no construtor se refere ao rowid, que não é recuperado 
         por essa query. A ordem das colunas recuperadas é ID_evento, descricao */
      Dicionario dicionario(SQLite::valor_coluna_int(stmt, 0), SQLite::valor_coluna_texto(stmt, 1));
      DI.push_back(dicionario);
   }
   SQLite::finalizar(stmt);
}

/* Seleciona itens na tabela Traduzida limitando a busca pelo ID_SE */
void Database::selecionar(int _ID_SE, vector<Evento>& SE)
{
   sqlite3_stmt *stmt;
   SE.clear();
   string query = TRADUZIDA_SELECT_POR_ID;
    stringstream ss;
    ss << _ID_SE << " ORDER BY TS;";
    query += ss.str();
//   query += itoa(_ID_SE, buffer, 10);
//   query += " ORDER BY TS;";
   SQLite::preparar(bd, query.c_str(), &stmt);
   while(SQLite::passo(stmt) == SQLITE_ROW)
   {
      /* primeiro valor no construtor se refere ao rowid, que não é recuperado 
         por essa query. A ordem das colunas recuperadas é ID_SE, ID_evento, TS */
      Evento evento(0, SQLite::valor_coluna_int(stmt, 0), SQLite::valor_coluna_int(stmt, 1),\
         SQLite::valor_coluna_int(stmt, 2));
      SE.push_back(evento);
   }
   SQLite::finalizar(stmt);
}

/* Deleta registros de uma tabela segundo uma restrição (campo = valor) */
int Database::deletar(string tabela, string campo, int valor)
{
   string str = "DELETE FROM bd_memoria." + tabela + " WHERE " + campo + " = ";
    stringstream ss;
    ss << valor << ";";
   // str += itoa(valor, buffer, 10);
   // str += ";";
    str += ss.str();
   return enfileirar(str);
}

/* Deleta todos os registros de uma tabela (sem restrição) */
int Database::deletar(string tabela)
{
   string str = "DELETE FROM bd_memoria." + tabela + ";";
   return enfileirar(str);
}

/* Executa uma query no BD e imprimi o resultado (utiliza o método "callback_debug" para isso) */
int Database::executar_debug(string query)
{
   sqlite3_stmt* stmt;
   query+=";";
   sqlite3_prepare(bd, query.c_str(), -1, &stmt, NULL);
   cout << "************************************************" << endl;
   //cout << "Banco de dados: " << sqlite3_column_database_name(stmt, 0) << endl;
   //cout << "Tabela: " << sqlite3_column_table_name(stmt, 0) << endl << endl;
   if(sqlite3_step(stmt) == SQLITE_ROW)
   {
      const char* s;
      for(int i = 0 ;  ; ++i)
      {
         s = sqlite3_column_name(stmt, i);
         if(s == NULL)
            break;
         cout << s << "\t";
      }
   }
   cout << "\n";
   sqlite3_finalize(stmt);
   SQLite::executar(bd, query.c_str(), callback_debug);
   cout << "************************************************" << endl;
    return 0;
}

int Database::atualizar_distancia(int rowid)
{
   string prefixo = "UPDATE bd_memoria.distancia";
   prefixo += " SET quantidade = quantidade + 1 ";
   prefixo += " WHERE rowid = ";
    stringstream ss;
    ss << rowid;
    prefixo += ss.str();
   prefixo += ";";
   
   return enfileirar(prefixo);
}

unsigned long long int Database::selecionar_llint(string query, int value)
{
   stringstream dint;        
   sqlite3_stmt *stmt;
   unsigned long long int ret = 0;
 
   dint.str(std::string()); 
   dint << value;    
   
   query += dint.str();
   query += ";";


   if( SQLite::preparar(bd, query.c_str(), &stmt) != SQLITE_OK )
      return 1;
      
   if( SQLite::passo(stmt) == SQLITE_ROW)
      ret = SQLite::valor_coluna_int64(stmt, 0);

   SQLite::finalizar(stmt);
   
   return ret;
}
