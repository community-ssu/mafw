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

/* Database access utilities */
#ifndef _MAFW_DB_H
#define _MAFW_DB_H

/* Include files */
#include <sqlite3.h>
#include <glib.h>

/* Macros */
/*
 * These macros are to make interference with SQLite prepared statements
 * easier.  Specifically:
 *
 * mafw_db_bind_text:		Binds a text value to a placeholder
 *				with the most commonplace parameters.
 *				It relies on the text value remaining
 *				valid until the statement is executed.
 * mafw_db_column_text:	Suppresses gcc's warnings about signed
 *				vs. unsigned pointers.
 *
 */
/**
 * mafw_db_bind_null:
 * @stmt: statement
 * @col: column to set the value
 *
 * Binds a NULL value to a placeholder with the most commonplace parameters.
 * <note> It starts counting columns from 0.  This is different
 * from the sqlite_bind_*() counterparts, which start indexing from 1.
 * </note>
 *
 */
#define mafw_db_bind_null(stmt, col)	\
	sqlite3_bind_null(stmt, (col)+1)

/**
 * mafw_db_column_null:
 * @stmt: statement
 * @col: column to check the value
 *
 * Checks, whether the value at col is NULL or not.
 *
 */
#define mafw_db_column_null(stmt, col)		\
	(sqlite3_column_type(stmt, col) == SQLITE_NULL)

/**
 * mafw_db_bind_text:
 * @stmt: statement
 * @col: column to set the value
 * @val: new value
 *
 * Binds a text value to a placeholder with the most commonplace parameters.
 * It relies on the text value remaining valid until the statement is executed.
 * <note> It starts counting columns from 0.  This is different
 * from the sqlite_bind_*() counterparts, which start indexing from 1.
 * </note>
 *
 */
#define mafw_db_bind_text(stmt, col, val)				\
	sqlite3_bind_text(stmt, (col)+1, val, -1, SQLITE_STATIC)

/**
 * mafw_db_column_text:
 * @stmt: statement
 * @col: column to get the value
 *
 * Calls sqlite3_column_text, suppresses gcc's warnings about signed vs. 
 * unsigned pointers. Returns the text at column col.
 *
 */
#define mafw_db_column_text(stmt, col)		\
	(gchar const *)sqlite3_column_text(stmt, col)

/**
 * mafw_db_bind_blob:
 * @stmt: statement
 * @col: column to set the value
 * @val: new value
 * @size: size of the value
 *
 * Binds a byte value to a placeholder with the most commonplace parameters.
 * It relies on the data value remaining valid until the statement is executed.
 * <note> It starts counting columns from 0.  This is different
 * from the sqlite_bind_*() counterparts, which start indexing from 1.
 * </note>
 *
 */
#define mafw_db_bind_blob(stmt, col, val, size)			\
	sqlite3_bind_blob(stmt, (col)+1, val, size, SQLITE_STATIC)

/**
 * mafw_db_column_blob:
 * @stmt: statement
 * @col: column to get the value
 *
 * Calls sqlite3_column_blob. Returns the data at column col.
 *
 */
#define mafw_db_column_blob(stmt, col)	\
		sqlite3_column_blob(stmt, col)

/**
 * mafw_db_bind_int:
 * @stmt: statement
 * @col: column to set the value
 * @val: new value
 *
 * Binds an integer value to a placeholder.
 * <note> It starts counting columns from 0.  This is different
 * from the sqlite_bind_*() counterparts, which start indexing from 1.
 * </note>
 *
 */
#define mafw_db_bind_int(stmt, col, val)	\
	sqlite3_bind_int(stmt, (col)+1, val)

/**
 * mafw_db_column_int:
 *
 * Calls sqlite3_column_int. Returns the integer value at column col.
 *
 */
#define mafw_db_column_int		sqlite3_column_int

/**
 * mafw_db_bind_int64:
 * @stmt: statement
 * @col: column to set the value
 * @val: new value
 *
 * Binds a int64 value to a placeholder.
 * <note> It starts counting columns from 0.  This is different
 * from the sqlite_bind_*() counterparts, which start indexing from 1.
 * </note>
 *
 */
#define mafw_db_bind_int64(stmt, col, val)	\
	sqlite3_bind_int64(stmt, (col)+1, val)
/**
 * mafw_db_column_int64:
 *
 * Calls sqlite3_column_int64. Returns the integer value at column col.
 *
 */
#define mafw_db_column_int64		sqlite3_column_int64

/* Function prototypes */
G_BEGIN_DECLS

extern sqlite3      *mafw_db_get(void);
extern void          mafw_db_trace(void);
extern sqlite3_stmt *mafw_db_prepare(gchar const *query);

extern gint mafw_db_exec(gchar const *query);
extern gint mafw_db_do(sqlite3_stmt *stmt);
extern gint mafw_db_nchanges(void);

extern gint mafw_db_select(sqlite3_stmt *stmt, gboolean expect_row);
extern gint mafw_db_change(sqlite3_stmt *stmt, gboolean csint_may_fail);
extern gint mafw_db_delete(sqlite3_stmt *stmt);

extern gboolean mafw_db_begin(void);
extern gboolean mafw_db_commit(void);
extern gboolean mafw_db_rollback(void);

G_END_DECLS
#endif /* ! _MAFW_DB_H */
