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

#include <glib.h>
#include <glib/gstdio.h>
 
#include "checkmore.h"
#include <libmafw/mafw-db.h>

#define TEST_TABLE "testtable"

START_TEST(test_error_statements)
{
	gint i = 0;
	sqlite3_stmt *stmt_test, *stmt_insert, *stmt_delete_keys;
	stmt_test = mafw_db_prepare("SELECT id, key "
					"FROM " TEST_TABLE " WHERE id = :id");
	stmt_insert = mafw_db_prepare("INSERT "
					"INTO " TEST_TABLE "(id, "
						"key) "
					"VALUES(:id, :key)");
	stmt_delete_keys = mafw_db_prepare("DELETE FROM "
					TEST_TABLE " WHERE id = :id AND "
					"key = :key");
	fail_if(!stmt_test || !stmt_insert || !stmt_delete_keys);
	fail_if (!mafw_db_begin());
	mafw_db_bind_int(stmt_insert, 0, 32);
	mafw_db_bind_text(stmt_insert, 1, "text");
	fail_if (mafw_db_change(stmt_insert, FALSE) != SQLITE_DONE);
	fail_if (!mafw_db_commit());
	
	mafw_db_bind_int(stmt_test, 0, 30);
	while (mafw_db_select(stmt_test, FALSE) == SQLITE_ROW)
	{
		i++;
	}
	fail_if(i != 0);
	sqlite3_reset(stmt_test);
	
	mafw_db_bind_int64(stmt_delete_keys, 0, 30);
	mafw_db_bind_text(stmt_delete_keys, 1, "text");
	fail_if (mafw_db_delete(stmt_delete_keys) != SQLITE_DONE);
	sqlite3_reset(stmt_delete_keys);
	
	mafw_db_bind_int(stmt_test, 0, 32);
	while (mafw_db_select(stmt_test, FALSE) == SQLITE_ROW)
	{
		i++;
	}
	fail_if(i != 1);
	sqlite3_reset(stmt_test);
	
	sqlite3_finalize(stmt_test);
	sqlite3_finalize(stmt_insert);
	sqlite3_finalize(stmt_delete_keys);
}
END_TEST

START_TEST(test_statements)
{
	gint i = 0;
	sqlite3_stmt *stmt_test, *stmt_insert, *stmt_delete_keys;
	stmt_test = mafw_db_prepare("SELECT id, key "
					"FROM " TEST_TABLE);
	stmt_insert = mafw_db_prepare("INSERT "
					"INTO " TEST_TABLE "(id, "
						"key) "
					"VALUES(:id, :key)");
	stmt_delete_keys = mafw_db_prepare("DELETE FROM "
					TEST_TABLE " WHERE id = :id AND "
					"key = :key");

	fail_if(!stmt_test || !stmt_insert || !stmt_delete_keys);
	fail_if (!mafw_db_begin());
	mafw_db_bind_int(stmt_insert, 0, 32);
	mafw_db_bind_text(stmt_insert, 1, "text");
	fail_if (mafw_db_change(stmt_insert, FALSE) != SQLITE_DONE);
	fail_if (!mafw_db_commit());
	
	while (mafw_db_select(stmt_test, FALSE) == SQLITE_ROW)
	{
		i++;
	}
	fail_if(i != 1);
	sqlite3_reset(stmt_test);
	
	mafw_db_bind_int64(stmt_delete_keys, 0, 32);
	mafw_db_bind_text(stmt_delete_keys, 1, "text");
	fail_if(mafw_db_delete(stmt_delete_keys) != SQLITE_DONE);
	sqlite3_reset(stmt_delete_keys);
	
	sqlite3_finalize(stmt_test);
	sqlite3_finalize(stmt_insert);
	sqlite3_finalize(stmt_delete_keys);
}
END_TEST

START_TEST(test_basic)
{
	sqlite3 *db;

	db = mafw_db_get();
	fail_if(db == NULL);
	
	fail_if(!mafw_db_begin());
	fail_if(mafw_db_nchanges() != 0);
	fail_if(!mafw_db_rollback());

	mafw_db_exec(
		"CREATE TABLE IF NOT EXISTS " TEST_TABLE "(\n"
		"id		INTEGER		NUT NULL,\n"
		"key		TEXT		NOT NULL)");

}
END_TEST

int main(void)
{
	TCase *tc;
	Suite *suite;

	g_unlink("test-db.db");
	g_setenv("MAFW_DB", "test-db.db", TRUE);

	suite = suite_create("MafwDB");
	tc = tcase_create("DB");
	suite_add_tcase(suite, tc);

	if (1) tcase_add_test(tc, test_basic);
	if (1) tcase_add_test(tc, test_statements);
	if (1) tcase_add_test(tc, test_error_statements);

	return checkmore_run(srunner_create(suite), FALSE);
}
