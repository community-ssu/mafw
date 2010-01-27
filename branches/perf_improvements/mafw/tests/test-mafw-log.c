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

#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <check.h>

#include "libmafw/mafw-log.h"

#include "checkmore.h"

/* Test that mafw_log_init() interprets the filter correctly. */
START_TEST(test_logging)
{
	FILE *st;
	char line[128];

	/* Set the filter to test with. */
	mafw_log_init(":message,foo:-,bar:debug");

	/* Print some messages to the indicated file. */
	checkmore_redirect("test.log");

	g_debug   ("dddddddddddddddddddddddddddddddd");
	g_info    ("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii");
	g_message ("mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm");
	g_warning ("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww");
	g_critical("cccccccccccccccccccccccccccccccc");

#undef  G_LOG_DOMAIN
#define G_LOG_DOMAIN "foo"

	g_debug   ("dddddddddddddddddddddddddddddddd");
	g_info    ("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii");
	g_message ("mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm");
	g_warning ("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww");
	g_critical("cccccccccccccccccccccccccccccccc");

#undef  G_LOG_DOMAIN
#define G_LOG_DOMAIN "bar"

	g_debug   ("dddddddddddddddddddddddddddddddd");
	g_info    ("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii");
	g_message ("mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm");
	g_warning ("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww");
	g_critical("cccccccccccccccccccccccccccccccc");

	/* Check that the file contains what it should
	 * according to the logging filter. */
	st = fopen("test.log", "r");
	while (fgets(line, sizeof(line), st) &&
	       line[0] == '\n');
	fail_if(!strstr(line, "MESSAGE") &&
		!strstr(line, "Message"));
	while (fgets(line, sizeof(line), st) &&
	       line[0] == '\n');
	fail_if(!strstr(line, "WARNING"));
	while (fgets(line, sizeof(line), st) &&
	       line[0] == '\n');
	fail_if(!strstr(line, "CRITICAL"));

	while (fgets(line, sizeof(line), st) &&
	       line[0] == '\n');
	fail_if(!strstr(line, "DEBUG"));
	while (fgets(line, sizeof(line), st) &&
	       line[0] == '\n');
	fail_if(!strstr(line, "INFO"));

	while (fgets(line, sizeof(line), st) &&
	       line[0] == '\n');
	fail_if(!strstr(line, "MESSAGE") &&
		!strstr(line, "Message"));
	while (fgets(line, sizeof(line), st) &&
	       line[0] == '\n');
	fail_if(!strstr(line, "WARNING"));
	while (fgets(line, sizeof(line), st) &&
	       line[0] == '\n');
	fail_if(!strstr(line, "CRITICAL"));
	fclose(st);

	unlink("test.log");
}
END_TEST

int main(void)
{
	Suite *suite;

	unlink("test.log");
	suite = suite_create("Mafw logging");
	checkmore_add_tcase(suite, "Mafw logging", test_logging);

	return checkmore_run(srunner_create(suite), FALSE);
}
