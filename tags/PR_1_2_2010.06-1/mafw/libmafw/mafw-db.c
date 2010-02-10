/*
 * This file is a part of MAFW
 *
 * Copyright (C) 2007, 2008, 2009 Nokia Corporation, all rights reserved.
 *
 * Contact: Visa Smolander <visa.smolander@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/* Include files */
#include <stdlib.h>
#include <ctype.h>

#include <glib.h>

#include "mafw-db.h"

/**
 * SECTION:mafwdb
 * @short_description: wrapper around sqlite
 *
 * This is a set of functions to make like against sqlite a bit
 * easier.
 *
 * NOTE All binding macros starts counting columns from 0.  This is different
 * from the sqlite_bind_*() counterparts, which start indexing from 1, but is
 * consistent with sqlite_column_*().
 */

/* Standard definitions */
#undef  G_LOG_DOMAIN
#define G_LOG_DOMAIN		"mafw-db"

/*
 * MAFW_DFLT_DB_PATH:		basename of the framework database file
 *				unless overridden by $MAFW_DB
 */
#define MAFW_DFLT_DB_FNAME	".mafw.db"

/* Program code */
/* sqlite3_trace() function callback. */
static void tracefun(void *unused, char const *sql)
{
	g_warning("%s", sql);
}

/* These functions are global, but reserved for internal use by MAFW. */
/**
 * mafw_db_get:
 * 
 * Gets a handle to the framework database, taking care of the
 * database creation and error handling.  As this resource is global
 * may not free it.
 *
 * Returns: the handle
 */
sqlite3 *mafw_db_get(void)
{
	static sqlite3 *db = NULL;
	char const *path;
	gboolean path_allocated;

	if (db != NULL)
		return db;

	/* Figure out where to place the database file.
	 * First try $MAFW_DB, then $HOME/MAFW_DFLT_DB_FNAME
	 * and finally /home/user/MAFW_DFLT_DB_FNAME. */
	path_allocated = FALSE;
	if (!(path = getenv("MAFW_DB"))) {
		const char *home;

		if (!(home = getenv("HOME")))
			home = "/home/user";
		path = g_strdup_printf("%s/%s", home, MAFW_DFLT_DB_FNAME);
		path_allocated = TRUE;
	}

	/* Open the database and set a random timeout, so concurrent
	 * writers will have more chance avoiding starvation. */
	if (sqlite3_open(path, &db) != SQLITE_OK)
		g_error("Could not open the database: %s", sqlite3_errmsg(db));
	sqlite3_busy_timeout(db, g_random_int_range(100, 1001));

	if (path_allocated)
		g_free((char *)path);
	return db;
}

/**
 * mafw_db_trace:
 * 
 * Print every SQL statement about to be executed.
 * Host variables are not expanded.
 */
void mafw_db_trace(void)
{
	sqlite3_trace(mafw_db_get(), tracefun, NULL);
}

/**
 * mafw_db_prepare:
 * @query: the query
 *
 * Gets the prepared statement equivalent of @query, which should
 * contain exactly one SQL statement.  Errors (like a syntax errors)
 * are treated fatal. Take into account that TABLES @query refers to
 * must exist before you compile them with this function.
 *
 * Returns: the statement
 */
sqlite3_stmt *mafw_db_prepare(gchar const *query)
{
	sqlite3 *db;
	sqlite3_stmt *stmt;
	gchar const *tail;

	db = mafw_db_get();
	if (sqlite3_prepare_v2(db, query, -1, &stmt, &tail) != SQLITE_OK) {
		g_error("`%s': %s", query, sqlite3_errmsg(db));
		return NULL;
	}

	/* $stmt is left NULL if $query did not contain any SQL commands. */
	g_assert(stmt != NULL);

	/* $tail points right after the end of the first statement.
	 * Since $query is supposed to contain only a single statement
	 * so we accept nothings else here than whitespace at most. */
	g_assert(!tail || !tail[0] || isspace(tail[0]));

	return stmt;
}

/**
 * mafw_db_exec:
 * @query: the query to execute
 * 
 * Executes @query, handling busy database errors, and returns the
 * result code of the operation. This function is mainly intended for
 * CREATE TABLE statements, so you should not execute any query that
 * returns any rows (SELECT), unless you are very sure about it.
 * Please note that any kind of errors are reported, eg. "table
 * already exist".
 *
 * Returns: a sqlite error code
 */
gint mafw_db_exec(gchar const *query)
{
	int ret;
	sqlite3 *db;

	db = mafw_db_get();
	while ((ret = sqlite3_exec(db, query,
				   NULL, NULL, NULL)) == SQLITE_BUSY)
		/* Try again */;
	if (ret != SQLITE_OK)
		g_warning("`%s': %s", query, sqlite3_errmsg(db));
	return ret;
}

/**
 * mafw_db_do:
 * @stmt: statement
 *
 * Tries to execute @stmt until the database is unlocked.
 * Between retries SQLite waits for a random amount of time.
 * Note that you need to sqlite3_reset(@stmt) after done
 * with it.
 *
 * Returns: a sqlite error code.
 */
gint mafw_db_do(sqlite3_stmt *stmt)
{
	int ret;

	while ((ret = sqlite3_step(stmt)) == SQLITE_BUSY)
		/* Try again */;
	return ret;
}

/**
 * mafw_db_nchanges:
 *
 * Convenience function to obtain the number of rows changed due
 * to the last statement, as defined by sqlite3_changes().
 * 
 * Returns: a sqlite error code
 */
gint mafw_db_nchanges(void)
{
	return sqlite3_changes(mafw_db_get());
}

/**
 * mafw_db_select:
 * @stmt: the statement
 * @expect_row: %TRUE if a row is expected
 *
 * Attempts to fetch the next row of a SELECT query.  On error
 * or if a row was expected but no more found an error message
 * is printed.  Returns the same code as #sqlite3_exec.
 *
 * Returns: a sqlite error code
 */
gint mafw_db_select(sqlite3_stmt *stmt, gboolean expect_row)
{
	int ret;

	ret = mafw_db_do(stmt);
	if (ret != SQLITE_DONE && ret != SQLITE_ROW)
		g_warning("SELECT: %s",
			  sqlite3_errmsg(sqlite3_db_handle(stmt)));
	else if (ret == SQLITE_DONE && expect_row)
		g_warning("Another row was expected");
	return ret;
}

/**
 * mafw_db_change:
 * @stmt: the statement
 * @csint_may_fail: %TRUE if operationg can fail
 * 
 * Attempts to INSERT or UPDATE a row.  On error an error message
 * is printed unless it was an expected constraint violation.
 *
 * Returns: a sqlite error code
 */
gint mafw_db_change(sqlite3_stmt *stmt, gboolean csint_may_fail)
{
	int ret;

	ret = mafw_db_do(stmt);
	if (ret != SQLITE_DONE && (ret != SQLITE_CONSTRAINT || !csint_may_fail))
		g_warning("INSERT/UPDATE: %s",
			  sqlite3_errmsg(sqlite3_db_handle(stmt)));
	return ret;
}

/**
 * mafw_db_delete:
 * @stmt: statement
 *
 * Attempts to DELETE a row.  On failure a warning is printed.
 *
 * Returns: a sqlite error code
 */
gint mafw_db_delete(sqlite3_stmt *stmt)
{
	int ret;

	if ((ret = mafw_db_do(stmt)) != SQLITE_DONE)
		g_warning("DELETE: %s",
			  sqlite3_errmsg(sqlite3_db_handle(stmt)));
	return ret;
}

/**
 * mafw_db_begin:
 *
 * Convenience function to begin a new transaction.  Returns whether
 * it succeeded; on failure a warning is printed.  It is an error to
 * start a new transaction while the previous one is not closed.
 * 
 * Returns: %TRUE if ok, %FALSE otherwise
 */
gboolean mafw_db_begin(void)
{
	return mafw_db_exec("BEGIN") == SQLITE_OK;
}

/**
 * mafw_db_commit:
 *
 * Like #mafw_db_begin but COMMITS the active transaction.
 * If it fails you should ROLLBACK your work.
 *
 * Returns: %TRUE if ok, %FALSE otherwise.
 */
gboolean mafw_db_commit(void)
{
	return mafw_db_exec("COMMIT") == SQLITE_OK;
}

/**
 * mafw_db_rollback:
 *
 *  Like #mafw_db_begin but ROLLBACKs the current transaction.
 *
 * Returns: %TRUE if ok, %FALSE otherwise.
 */
gboolean mafw_db_rollback(void)
{
	return mafw_db_exec("ROLLBACK") == SQLITE_OK;
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
