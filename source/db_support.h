/*
 *  Copyright (C) 2007-2015 Lonelycoder AB
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  This program is also available under a commercial proprietary license.
 *  For more information, contact andreas@lonelycoder.com
 */
#pragma once
#include "sqlite3.h"
#include "rstr.h"

int db_one_statement(sqlite3 *db, const char *sql, const char *src);

int db_get_int_from_query(sqlite3 *db, const char *query, int *v);

int db_get_int64_from_query(sqlite3 *db, const char *query, int64_t *v);

int db_begin0(sqlite3 *db, const char *src);

int db_commit0(sqlite3 *db, const char *src);

int db_rollback0(sqlite3 *db, const char *src);

int db_rollback_deadlock0(sqlite3 *db, const char *src);

int db_step(sqlite3_stmt *pStmt);





#define db_begin(db)    db_begin0(db, __FUNCTION__)
#define db_commit(db)   db_commit0(db, __FUNCTION__)
#define db_rollback(db) db_rollback0(db, __FUNCTION__)
#define db_rollback_deadlock(db) db_rollback_deadlock0(db, __FUNCTION__)


#define DB_OPEN_CASE_SENSITIVE_LIKE 0x1

sqlite3 *db_open(const char *path, int flags);



rstr_t *db_rstr(sqlite3_stmt *stmt, int col);

int db_posint(sqlite3_stmt *stmt, int col);

static __inline void db_bind_rstr(sqlite3_stmt *stmt, int col, rstr_t *rstr)
{
  sqlite3_bind_text(stmt, col, (void *)rstr_get(rstr), -1, SQLITE_STATIC);

}

void db_escape_path_query(char *dst, size_t dstlen, const char *src);

int db_init(char *cache_path);


