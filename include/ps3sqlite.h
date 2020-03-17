
#pragma once
#include <sqlite3.h>


int db_one_statement(sqlite3 *db, const char *sql, const char *src); //

int db_get_int_from_query(sqlite3 *db, const char *query, int *v);  //

int db_get_int64_from_query(sqlite3 *db, const char *query, int64_t *v);  //

#define DB_OPEN_CASE_SENSITIVE_LIKE 0x1

sqlite3 *db_open(const char *path, int flags);

int db_init(char *cache_path);

int db_posint(sqlite3_stmt *stmt, int col);

