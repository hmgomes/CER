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

#ifndef __UTILS_H__
#define __UTILS_H__
#include <sstream>
#include "sqlite3.h"
#include <iostream>

/* Msg de controle, todas as escritas na saida padrao devem ser precedidas por 
   ela. Se DEBUG estiver setado, entao adiciona informacao como nome do arquivo
   e a linha */
#ifdef DEBUG
#define CTRL_MSG Utils::hora() << " [" << __FILE__ << ", " << __LINE__ << "] "
#endif
#ifndef DEBUG
#define CTRL_MSG Utils::hora() << " "
#endif
#define MSG cout << CTRL_MSG
#define MSG_ERRO cout << "* [ERRO] " << CTRL_MSG
#define MSG_AVISO cout << CTRL_MSG << "[AVISO] " 

#define GMT -3

/* TODAS as SQLs... Nomenclatura segue o padrão <nome da tabela> + <instrução SQL> + <descrição> */

#define DICIONARIO_SELECT_POR_DESCRICAO "SELECT * FROM bd_memoria.dicionario WHERE upper(descricao) = upper('"
#define DICIONARIO_VERIFICA "SELECT count(*) FROM bd_memoria.sqlite_master WHERE upper(name)=upper('Dicionario');"
#define TRADUZIDA_VERIFICA "SELECT count(*) FROM bd_memoria.sqlite_master WHERE upper(name)=upper('Traduzida');"
#define DICIONARIO_MAX_IDEVENTO "SELECT max(ID_Evento) FROM bd_memoria.dicionario;"

/* Queries Definicao */
#define DEFINICAO_VERIFICA "SELECT count(*) FROM bd_memoria.sqlite_master WHERE upper(name)=upper('distancia');"
#define TRADUZIDA_SELECT_TS "SELECT TS FROM bd_memoria.Traduzida WHERE ID_SE = "
#define DISTANCIA_SELECT_COUNT_DISTANCIA "SELECT count(*) FROM bd_memoria.distancia WHERE distancia = "
#define DISTANCIA_SELECT_QUANTIDADE_DISTANCIA "SELECT quantidade FROM bd_memoria.distancia WHERE distancia = "
#define DISTANCIA_SELECT_ROWID_DISTANCIA "SELECT ROWID FROM bd_memoria.distancia WHERE distancia = "
#define DISTANCIA_MIN_DISTANCIA "SELECT min(distancia) FROM bd_memoria.distancia;"
#define DISTANCIA_MAX_DISTANCIA "SELECT max(distancia) FROM bd_memoria.distancia;"
#define DISTANCIA_SELECT_DISTANCIAS "SELECT distancia FROM bd_memoria.distancia;"
#define DISTANCIA_SELECT_QTDE "SELECT quantidade FROM bd_memoria.distancia WHERE distancia = "
#define DISTANCIA_SUM_DISTANCIA "SELECT SUM(distancia) FROM bd_memoria.distancia WHERE cluster = "
#define DISTANCIA_SELECT_COUNT_CLUSTER "SELECT count(*) FROM bd_memoria.distancia WHERE cluster = "
#define DISTANCIA_MAX_CLUSTER "SELECT MAX(distancia) FROM bd_memoria.distancia  WHERE cluster = "
#define DISTANCIA_MIN_CLUSTER "SELECT MIN(distancia) FROM bd_memoria.distancia WHERE cluster = "




#define RDT_SELECT_TODAS "SELECT oid, DtMi, DtMa, quantidade FROM bd_memoria.RDt;"
#define RDT_SELECT_TODAS_SOMENTE_ID "SELECT oid FROM bd_memoria.RDt;"
#define EFEITO_SELECT_TODOS "SELECT ID_efeito, ID_Rdt, quantidade, frequencia FROM bd_memoria.efeito;"
#define DICIONARIO_SELECT_TODAS_ID_EVENTO "SELECT ID_evento FROM bd_memoria.dicionario;"
#define DICIONARIO_SELECT_TODAS "SELECT ID_evento, descricao FROM bd_memoria.dicionario;"
#define SUMARIZADA_SELECT_POR_ID_RDT "SELECT ID_SCE, ID_efeito, ID_causa FROM bd_memoria.sumarizada WHERE ID_RDt = "
#define TRADUZIDA_SELECT_POR_ID "SELECT ID_SE, ID_evento, TS FROM bd_memoria.traduzida WHERE ID_SE = "

//#define TRADUZIDA_SELECT_QTD_SE "SELECT count(1) FROM (SELECT ID_SE FROM bd_memoria.traduzida GROUP BY ID_SE) LIMIT 1;"
//#define SUMARIZADA_MAX_ID_SCE "SELECT max(ID_SCE) FROM bd_memoria.sumarizada LIMIT 1;"
#define TRADUZIDA_MAX_ID_SE "SELECT max(ID_SE) FROM bd_memoria.traduzida LIMIT 1;"

/* Utilizadas para a geração de efeitos frequentes e remoção dos não frequentes (callback_filtrar_FME(..) em database.cpp.)*/
#define SUMARIZADA_COUNT_EFEITOS "SELECT id_efeito, id_rdt, count(1) FROM (SELECT ID_efeito, ID_RDt FROM bd_memoria.sumarizada GROUP BY ID_Rdt, ID_SCE) GROUP BY id_efeito, id_rdt;"
#define EFEITO_INSERT "INSERT INTO bd_memoria.efeito (ID_efeito, ID_rdt, quantidade, frequencia) VALUES ("
#define SUMARIZADA_DELETE_EFEITOS_NAO_FRQ_PREFIXO "DELETE FROM bd_memoria.sumarizada WHERE ID_efeito = "
#define SUMARIZADA_DELETE_EFEITOS_NAO_FRQ_SUFIXO " AND ID_RDt = "

/* Descoberta de RCE. */
#define SUMARIZADA_SELECT_DESCOBERTA "SELECT ID_SCE, ID_causa FROM bd_memoria.sumarizada WHERE ID_RDt = ? AND ID_efeito = ?;"
#define EFEITO_SELECT_AGRUPA_RDT_EFEITO "SELECT ID_efeito, ID_RDt FROM bd_memoria.efeito GROUP BY ID_RDt, ID_efeito;"
#define RCE_INSERT_SAIDA_MINERACAO "INSERT INTO bd_memoria.RCE (ID_RDt, ID_efeito, causa, quantidade, supCE, confCE) VALUES ("

/* Todo o schema é criado no disco também, porém apenas a tabela RCE é utilizada 
   O comando ATTACH DATABASE cria um banco 'in memory' e anexa ele com o alias de bd_memoria */
#define DATABASE_CREATE "\
CREATE TABLE efeito (ID_efeito INTEGER NOT NULL, ID_RDt INTEGER NOT NULL, quantidade INTEGER NOT NULL, frequencia REAL NOT NULL); \
CREATE TABLE RCE (causa TEXT NOT NULL, ID_efeito INTEGER NOT NULL, ID_RDt INTEGER NOT NULL, quantidade INTEGER NOT NULL, comprimento INTEGER NOT NULL, SupCE REAL NOT NULL, ConfCE REAL NOT NULL); \
CREATE TABLE RDt (ID_RDt INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, DtMi VARCHAR(250) NOT NULL, DtMa VARCHAR(250) NOT NULL, quantidade INTEGER NOT NULL); \
CREATE TABLE dicionario (ID_evento INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, descricao VARCHAR(255) NOT NULL); \
CREATE TABLE sumarizada (ID_SCE INTEGER NOT NULL, ID_causa INTEGER NOT NULL, ID_efeito INTEGER NOT NULL, ID_RDt INTEGER NOT NULL); \
CREATE VIEW  resultado AS SELECT RCE.comprimento as Comprimento, RCE.causa as [Causa(s)], dicionario.descricao as Efeito, efeito.frequencia as [Frequencia de Efeito], RCE.supCE, RCE.confCE, RDt.DtMi as [Dist. Minima], RDt.DtMa as [Dist. Maxima] FROM RCE INNER JOIN Dicionario ON (Dicionario.ID_evento = RCE.ID_efeito) INNER JOIN Efeito ON (RCE.ID_efeito = Efeito.ID_efeito AND RCE.ID_RDt = Efeito.ID_RDt) INNER JOIN RDt ON (RDt.ID_RDt = Efeito.ID_RDt); \
CREATE VIEW  qtd_regras_por_RDt AS SELECT [dist. Minima] as DtMi, [Dist. Maxima] as DtMa, count(1) as Total FROM RESULTADO group by [dist. Minima], [Dist. Maxima];\
ATTACH DATABASE ':memory:' AS bd_memoria;\
CREATE TABLE bd_memoria.sumarizada (ID_SCE INTEGER NOT NULL, ID_causa INTEGER NOT NULL, ID_efeito INTEGER NOT NULL, ID_RDt INTEGER NOT NULL); \
CREATE TABLE bd_memoria.traduzida (ID_SE INTEGER NOT NULL, ID_evento INTEGER NOT NULL, TS INTEGER NOT NULL); \
CREATE TABLE bd_memoria.efeito (ID_efeito INTEGER NOT NULL, ID_RDt INTEGER NOT NULL, quantidade INTEGER NOT NULL, frequencia REAL NOT NULL); \
CREATE TABLE bd_memoria.causa_efeito (ID_RCE INTEGER NOT NULL, ID_causa INTEGER NOT NULL, ID_efeito INTEGER NOT NULL, ID_RDt INTEGER NOT NULL, quantidade INTEGER NOT NULL); \
CREATE TABLE bd_memoria.RDt (ID_RDt INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, DtMi INTEGER NOT NULL, DtMa INTEGER NOT NULL, quantidade INTEGER NOT NULL); \
CREATE TABLE bd_memoria.dicionario (ID_evento INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, descricao VARCHAR(255) NOT NULL); \
CREATE TABLE bd_memoria.distancia (distancia INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, quantidade INTEGER NOT NULL, cluster INTEGER NOT NULL); \
CREATE TABLE bd_memoria.RCE (causa TEXT NOT NULL, ID_efeito INTEGER NOT NULL, ID_RDt INTEGER NOT NULL, quantidade INTEGER NOT NULL, SupCE REAL NOT NULL, ConfCE REAL NOT NULL); \
CREATE INDEX bd_memoria.ordena_por_TS ON traduzida (ID_SE ASC, TS ASC); \
CREATE INDEX bd_memoria.ordena_por_dist ON distancia (distancia ASC, quantidade ASC); \
CREATE INDEX bd_memoria.indice_dicionario ON dicionario (ID_evento ASC);"

using namespace std;

class Utils
{
   public:
      static string hora();
      static string hoje();
      static string converte_TS(int);
};

#endif
