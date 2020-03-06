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

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "vfs.h"
#include "fileaccess.h"
#include "minmax.h"

#include "db_support.h"



/**
 *
 */
int
db_one_statement(sqlite3 *db, const char *sql, const char *src)
{
  int rc;
  char *errmsg;

  rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if(rc) {
    printf( "SQLITE(one_statement) - %s: %s failed -- %s",
	  src ?: sql, sql, errmsg);
    sqlite3_free(errmsg);
  }
  return rc;
}


/**
 *
 */
int
db_get_int64_from_query(sqlite3 *db, const char *query, int64_t *v)
{
  int rc;
  int64_t rval = -1;
  sqlite3_stmt *stmt;

  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
  if(rc != SQLITE_OK)
	  return -1;
 

  rc = sqlite3_step(stmt);
  

  if(rc == SQLITE_ROW) {
    *v = sqlite3_column_int64(stmt, 0);
    rval = 0;
  } else {
    rval = -1;
  }

  sqlite3_finalize(stmt);
  return rval;
}


/**
 *
 */
int
db_get_int_from_query(sqlite3 *db, const char *query, int *v)
{
  int64_t i64;
  int r = db_get_int64_from_query(db, query, &i64);
  if(r == 0)
    *v = i64;
  return r;
}



int
db_begin0(sqlite3 *db, const char *src)
{
  return db == NULL || db_one_statement(db, "BEGIN;", src);
}


int
db_commit0(sqlite3 *db, const char *src)
{
  return  db == NULL || db_one_statement(db, "COMMIT;", src);
}


int
db_rollback0(sqlite3 *db, const char *src)
{
  return  db == NULL || db_one_statement(db, "ROLLBACK;", src);
}

int
db_rollback_deadlock0(sqlite3 *db, const char *src)
{
  int r = db == NULL || db_one_statement(db, "ROLLBACK;", src);
  printf("TRACE_DEBUG - DB: Rollback due to deadlock, and retrying");
  usleep(100000);
  return r;
}




/**
 *
 */
sqlite3 *db_open(const char *path, int flags)
{
  int rc;
  sqlite3 *db;

  rc = sqlite3_open_v2(path, &db,
			   SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
		       SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE,
		       NULL);

  if(rc) {
    printf("DB TRACE - %s: Unable to open database: %s\n",
	  path, sqlite3_errmsg(db));
    sqlite3_close(db);
    return NULL;
  }

  db_one_statement(db, "PRAGMA synchronous = normal", path);
  if(flags & DB_OPEN_CASE_SENSITIVE_LIKE)
    db_one_statement(db, "PRAGMA case_sensitive_like=1", path);
  db_one_statement(db, "PRAGMA foreign_keys=1", path);

  int freelist_count = 0;
  int page_count = 0;

  db_get_int_from_query(db, "PRAGMA freelist_count", &freelist_count);
  db_get_int_from_query(db, "PRAGMA page_count",     &page_count);

  printf("DB TRACE - Opened database %s pages: free=%d total=%d\n",
        path, freelist_count, page_count);

  return db;
}




/**
 *
 */
rstr_t *
db_rstr(sqlite3_stmt *stmt, int col)
{
  return rstr_alloc((const char *)sqlite3_column_text(stmt, col));
}


/**
 *
 */
int
db_posint(sqlite3_stmt *stmt, int col)
{
  if(sqlite3_column_type(stmt, col) == SQLITE_INTEGER)
    return sqlite3_column_int(stmt, col);
  return -1;
}


/**
 *
 */
void
db_escape_path_query(char *dst, size_t dstlen, const char *src)
{
  if(*src == 0) {
    *dst++ = '%';
    *dst = 0;
    return;
  }
  for(; *src && dstlen > 4; dstlen--) {
    if(*src == '%' || *src == '_') {
      *dst++ = '\\';
      *dst++ = *src++;
      dstlen--;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '/';
  *dst++ = '%';
  *dst = 0;
}


/*
** Argument pStmt is a prepared SQL statement. This function compiles
** an EXPLAIN QUERY PLAN command to report on the prepared statement,
** and prints the report to stdout using printf().
*/
int
db_explain(sqlite3_stmt *pStmt)
{
  const char *zSql;               /* Input SQL */
  char *zExplain;                 /* SQL with EXPLAIN QUERY PLAN prepended */
  sqlite3_stmt *pExplain;         /* Compiled EXPLAIN QUERY PLAN command */
  int rc;                         /* Return code from sqlite3_prepare_v2() */

  zSql = sqlite3_sql(pStmt);
  if( zSql==0 ) return SQLITE_ERROR;

  printf("TRACE_DEBUG - EXPLAIN %s\n", zSql);

  zExplain = sqlite3_mprintf("EXPLAIN QUERY PLAN %s", zSql);
  if( zExplain==0 ) return SQLITE_NOMEM;

  rc = sqlite3_prepare_v2(sqlite3_db_handle(pStmt), zExplain, -1, &pExplain, 0);
  sqlite3_free(zExplain);
  if( rc!=SQLITE_OK ) return rc;

  while( SQLITE_ROW==sqlite3_step(pExplain) ){
    int iSelectid = sqlite3_column_int(pExplain, 0);
    int iOrder = sqlite3_column_int(pExplain, 1);
    int iFrom = sqlite3_column_int(pExplain, 2);
    const char *zDetail = (const char *)sqlite3_column_text(pExplain, 3);

    printf("TRACE_DEBUG - EXPLAIN %d %d %d %s\n", iSelectid, iOrder, iFrom, zDetail);
          
  }

  return sqlite3_finalize(pExplain);
}





static void
db_log(void *aux, int code, const char *str)
{
  int non_extended_code = code & 0xff;
  // Some codes are nothing to worry about as we or sqlite retries internally
  if(non_extended_code == SQLITE_CONSTRAINT ||
     code == SQLITE_LOCKED_SHAREDCACHE ||
     non_extended_code == SQLITE_SCHEMA)
    return;

  printf("DB LOG - SQLITE: %s (code: 0x%x)", str, code);
}

/*#include "callout.h"

static callout_t memlogger;

static void
memlogger_fn(callout_t *co, void *aux)
{
  callout_arm(&memlogger, memlogger_fn, NULL, 1);

  int dyn_current, dyn_highwater;
  int pgc_current, pgc_highwater;
  int scr_current, scr_highwater;

  sqlite3_status(SQLITE_STATUS_MEMORY_USED,
                 &dyn_current, &dyn_highwater, 0);
  sqlite3_status(SQLITE_STATUS_PAGECACHE_USED,
                 &pgc_current, &pgc_highwater, 0);
  sqlite3_status(SQLITE_STATUS_SCRATCH_USED,
                 &scr_current, &scr_highwater, 0);

  printf("DEBUG - SQLITE: Mem %d (%d)  PGC: %d (%d) Scratch: %d (%d)",
        dyn_current, dyn_highwater,
        pgc_current, pgc_highwater,
        scr_current, scr_highwater);

}*/

int db_init(char *cache_path)
{
  int i;
  printf("cache_path: %s\n", cache_path);
  sqlite3_temp_directory = cache_path;
  vfs_set_cache_path(cache_path);
#if ENABLE_SQLITE_LOCKING
  sqlite3_config(SQLITE_CONFIG_MUTEX, &sqlite_mutexes);
#endif
  sqlite3_config(SQLITE_CONFIG_LOG, &db_log, NULL);

  i = sqlite3_initialize();
#ifdef PS3
  sqlite3_soft_heap_limit(10000000);
#endif
  //if(0)
    //callout_arm(&memlogger, memlogger_fn, NULL, 1);
return i;
}

/*extern db_pool_t *metadb_pool;

void *
metadb_get(void)
{
  return db_pool_get(metadb_pool);
}

void 
metadb_close(void *db)
{
  db_pool_put(metadb_pool, db);
}*/
