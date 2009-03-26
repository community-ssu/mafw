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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <check.h>

#include <libmafw/mafw-filter.h>

#include "checkmore.h"

START_TEST(test_simple)
{
	MafwFilter *fi;

	fi = mafw_filter_parse("(artist=belga)");
	fail_unless(fi->type == mafw_f_eq);
	fail_unless(!strcmp(fi->key, "artist"));
	fail_unless(!strcmp(fi->value, "belga"));
	mafw_filter_free(fi);
}
END_TEST

START_TEST(test_bad)
{
	fail_unless(!mafw_filter_parse("=belga)"));
	fail_unless(!mafw_filter_parse("(&(foo=bar()(xxx>yyy"));
	fail_unless(!mafw_filter_parse("(title!=something)"));
	fail_unless(!mafw_filter_parse("(!=titlesomething)"));
	fail_unless(!mafw_filter_parse("!(title=something)"));
	fail_unless(!mafw_filter_parse("((title=something))"));
}
END_TEST

START_TEST(test_simple_2)
{
	MafwFilter *fi;

	fi = mafw_filter_parse("(publication-year<1999)");
	fail_unless(fi->type == mafw_f_lt);
	fail_unless(!strcmp(fi->key, "publication-year"));
	fail_unless(!strcmp(fi->value, "1999"));
	mafw_filter_free(fi);

	fi = mafw_filter_parse("(album?)");
	fail_unless(fi->type == mafw_f_exists);
	fail_unless(!strcmp(fi->key, "album"));
	fail_unless(!strcmp(fi->value, ""));
	mafw_filter_free(fi);

	/* Empty key is an error. */
	fi = mafw_filter_parse("(=artist=*foobar)");
	fail_unless(fi == NULL);
}
END_TEST

START_TEST(test_complex_not)
{
	MafwFilter *fi;

	fi = mafw_filter_parse("(!(year>2004))");
	fail_unless(fi->type == mafw_f_not);
	fail_unless(fi->parts[0]->type == mafw_f_gt);
	fail_unless(!strcmp(fi->parts[0]->key, "year"));
	fail_unless(!strcmp(fi->parts[0]->value, "2004"));
	fail_unless(fi->parts[1] == NULL);
	mafw_filter_free(fi);

	/* Accept only one sub-expr for `NOT'. */
	fi = mafw_filter_parse("(!(year>2004)(foo=bar))");
	fail_unless(fi == NULL);
}
END_TEST

START_TEST(test_complex)
{
	MafwFilter *fi;

	fi = mafw_filter_parse("(&(artist~belga)(year>2004))");
	fail_unless(fi->type == mafw_f_and);
	fail_unless(fi->parts[0]->type == mafw_f_approx);
	fail_unless(!strcmp(fi->parts[0]->key, "artist"));
	fail_unless(!strcmp(fi->parts[0]->value, "belga"));
	fail_unless(fi->parts[1]->type == mafw_f_gt);
	fail_unless(!strcmp(fi->parts[1]->key, "year"));
	fail_unless(!strcmp(fi->parts[1]->value, "2004"));
	fail_unless(fi->parts[2] == NULL);
	mafw_filter_free(fi);
}
END_TEST

START_TEST(test_complex_2)
{
	MafwFilter *fi;

	fi = mafw_filter_parse("(&(|(artist~belga)(artist=betlehem))(year>2004))");
	fail_unless(fi->type == mafw_f_and);
	fail_unless(fi->parts[0]->type == mafw_f_or);
	fail_unless(fi->parts[0]->parts[0]->type == mafw_f_approx);
	fail_unless(!strcmp(fi->parts[0]->parts[0]->key, "artist"));
	fail_unless(!strcmp(fi->parts[0]->parts[0]->value, "belga"));
	fail_unless(fi->parts[0]->parts[1]->type == mafw_f_eq);
	fail_unless(!strcmp(fi->parts[0]->parts[1]->key, "artist"));
	fail_unless(!strcmp(fi->parts[0]->parts[1]->value, "betlehem"));
	fail_unless(fi->parts[0]->parts[2] == NULL);
	fail_unless(fi->parts[1]->type == mafw_f_gt);
	fail_unless(!strcmp(fi->parts[1]->key, "year"));
	fail_unless(!strcmp(fi->parts[1]->value, "2004"));
	fail_unless(fi->parts[2] == NULL);
	mafw_filter_free(fi);
}
END_TEST

/* Returns if $lhs and $rhs are the same string-wise and frees $lhs,
 * which is supposed to be allocated by mafw_filter_{un,}quote(). */
static gboolean compare(gchar *lhs, const gchar *rhs)
{
	gboolean ret;

	ret = !strcmp(lhs, rhs);
	free(lhs);
	return ret;
}

START_TEST(test_quote)
{
	fail_unless(compare(mafw_filter_quote(""), ""));
	fail_unless(compare(mafw_filter_quote("foobar"), "foobar"));
	fail_unless(compare(mafw_filter_quote("f*obar"), "f\\2Aobar"));
	fail_unless(compare(mafw_filter_quote("f**bar"), "f\\2A\\2Abar"));
	fail_unless(compare(mafw_filter_quote("little (big) adventure"),
			    "little \\28big\\29 adventure"));
	fail_unless(compare(mafw_filter_quote("\\back\nand\nforward/"),
			    "\\5Cback\nand\nforward/"));
}
END_TEST

START_TEST(test_unquote)
{
	unsigned i, o;
	char *s, longs[1 + 256*3 + 1];

	fail_unless(compare(mafw_filter_unquote("\\5C"), "\\"));
	fail_unless(compare(mafw_filter_unquote("foobar\\5C\\29\\2A"),
			    "foobar\\)*"));
	fail_unless(compare(mafw_filter_unquote("\\41\\42\\43"), "ABC"));

	/* Test that all possible escape sequences are decoded correctly. */
	longs[0] = 'X';
	for (i = o = 0; i < 256; i++) {
		unsigned u, l;

		u = i >> 4;
		l = i & 0x0F;

		longs[++o] = '\\';
		longs[++o] = u < 10 ? '0' + u : 'A' + u - 10;
		longs[++o] = l < 10 ? '0' + l : 'A' + l - 10;
	}
	longs[++o] = '\0';

	s = mafw_filter_unquote(longs);
	fail_unless(s != NULL);
	fail_unless(s[0] == 'X');
	for (i = 1; i < 1 + 256; i++)
		fail_unless(s[i] == (char)(i - 1));
	fail_unless(s[i] == '\0');
	free(s);

	/* Test that invalid escape seques are rejected.
	 * Replace the first digit of the last one with
	 * an invalid character. */
	longs[1 + 255*3 + 1] = 'X';
	fail_unless(!mafw_filter_unquote(longs));
} END_TEST

START_TEST(test_new)
{
	MafwFilter *f;

	f = MAFW_FILTER_AND(MAFW_FILTER_EQ("foo", "bar"),
		  MAFW_FILTER_APPROX("album", "moo"));

	fail_unless(f->type == mafw_f_and);
	fail_unless(f->parts[0]->type == mafw_f_eq);
	fail_unless(!strcmp(f->parts[0]->key, "foo"));
	fail_unless(!strcmp(f->parts[0]->value, "bar"));
	fail_unless(f->parts[1]->type == mafw_f_approx);
	fail_unless(!strcmp(f->parts[1]->key, "album"));
	fail_unless(!strcmp(f->parts[1]->value, "moo"));
	fail_unless(f->parts[2] == NULL);
	mafw_filter_free(f);
}
END_TEST

START_TEST(test_add_child)
{
	MafwFilter *f;

	f = MAFW_FILTER_AND();
	fail_unless(f->type == mafw_f_and);
	fail_unless(f->parts[0] == NULL);

	f = mafw_filter_add_children(f, MAFW_FILTER_EQ("att", "vvv"));

	fail_unless(f->type == mafw_f_and);
	fail_unless(f->parts[0]->type == mafw_f_eq);
	fail_unless(!strcmp(f->parts[0]->key, "att"));
	fail_unless(!strcmp(f->parts[0]->value, "vvv"));
	fail_unless(f->parts[1] == NULL);

	f = mafw_filter_add_children(f, MAFW_FILTER_LT("yyy", "kkk"));

	fail_unless(f->type == mafw_f_and);
	fail_unless(f->parts[0]->type == mafw_f_eq);
	fail_unless(!strcmp(f->parts[0]->key, "att"));
	fail_unless(!strcmp(f->parts[0]->value, "vvv"));
	fail_unless(f->parts[1]->type == mafw_f_lt);
	fail_unless(!strcmp(f->parts[1]->key, "yyy"));
	fail_unless(!strcmp(f->parts[1]->value, "kkk"));
	fail_unless(f->parts[2] == NULL);

	mafw_filter_free(f);
}
END_TEST

/*
 * build_sql:
 * @filter: a #MafwFilter.
 *
 * Builds an SQL `WHERE' expression from the passed filter.
 * It is not really useful.
 */
static void build_sql(MafwFilter *filter, GString *p)
{
	if (MAFW_FILTER_IS_SIMPLE(filter)) {
		g_string_append(p, filter->key);
		switch (filter->type) {
		case mafw_f_eq:
		       	g_string_append(p, " = ");
			break;
		case mafw_f_lt:
		       	g_string_append(p, " < ");
		       	break;
		case mafw_f_gt:
		       	g_string_append(p, " > ");
		       	break;
		case mafw_f_approx:
		       	g_string_append(p, " LIKE ");
		       	break;
		case mafw_f_exists:
		       	g_string_append(p, " IS NOT NULL");
		       	break;
		default: break;
		}
		if (filter->type != mafw_f_exists)
			g_string_append_printf(p, "\"%s\"", filter->value);
	} else {
		MafwFilter **parts;
		for (parts = filter->parts; *parts; parts++) {
			if (filter->type == mafw_f_not)
				g_string_append(p, "NOT ");
			g_string_append(p, "(");
			build_sql(*parts, p);
			g_string_append(p, ")");
			if (filter->type == mafw_f_and)
				g_string_append(p, " AND ");
			else if (filter->type == mafw_f_or)
				g_string_append(p, " OR ");
		}
		/* Append some idempotent expression to finish it. */
		if (filter->type == mafw_f_and)
			g_string_append(p, "(1 = 1)");
		else if (filter->type == mafw_f_or)
			g_string_append(p, "(0 = 0)");
	}
}

START_TEST(test_build_sql)
{
	GString *sql;
	MafwFilter *f;

	f = MAFW_FILTER_AND(MAFW_FILTER_NOT(MAFW_FILTER_EQ("xxx", "YYY")),
		  MAFW_FILTER_EQ("foo", "bar"));
	sql = g_string_new("");
	build_sql(f, sql);
	fail_unless(!strcmp(sql->str,
		"(NOT (xxx = \"YYY\")) AND (foo = \"bar\") AND (1 = 1)"));
	mafw_filter_free(f);

	f = MAFW_FILTER_EXISTS("a");
	g_string_truncate(sql, 0);
	build_sql(f, sql);
	fail_unless(!strcmp(sql->str, "a IS NOT NULL"));
	mafw_filter_free(f);
	g_string_free(sql, TRUE);
}
END_TEST

static void build_url(MafwFilter *f, GString *url)
{
	if (MAFW_FILTER_IS_SIMPLE(f)) {
		g_string_append_printf(url, "%s=%s", f->key, f->value);
	} else {
		if (f->type == mafw_f_and) {
			MafwFilter **part;
			for (part = f->parts; *part; part++) {
				build_url(*part, url);
				if (*(part + 1) == NULL) break;
				g_string_append_c(url, '&');
			}
		}
	}
}

START_TEST(test_build_url)
{
	GString *u;
	MafwFilter *f;

	f = MAFW_FILTER_AND(MAFW_FILTER_EQ("album", "korte"),
		  MAFW_FILTER_EQ("year", "1982"));
	u = g_string_new("");
	build_url(f, u);
	fail_unless(!strcmp(u->str, "album=korte&year=1982"));
	mafw_filter_free(f);
	g_string_free(u, TRUE);
}
END_TEST

START_TEST(test_parse_to_string_copy)
{
	MafwFilter *filter, *copy;
	const gchar *original;
	gchar *result, *result_copy;

	original = "(&(!(artist=\\28belga\\29))(|(genre=rock)(album?)))";
	filter = mafw_filter_parse(original);
	copy = mafw_filter_copy(filter);
	result = mafw_filter_to_string(filter);
	result_copy = mafw_filter_to_string(filter);
	fail_if(g_strcmp0(original, result) != 0, "Original (%s) and result of "
		"to_string (%s) are different", original, result);
	fail_if(g_strcmp0(original, result_copy) != 0, "Original (%s) and "
		"result of to_string of copy (%s) are different", original,
		result);
	g_free(result);
	g_free(result_copy);
	mafw_filter_free(filter);
	mafw_filter_free(copy);

	original = "(\\28artist\\29?)";
	filter = MAFW_FILTER_EXISTS("(artist)");
	result = mafw_filter_to_string(filter);
	fail_if(g_strcmp0(original, result) != 0, "Original (%s) and result of "
		"to_string (%s) are different", original, result);
	g_free(result);
	mafw_filter_free(filter);

	original = "(!(|(genre=rock)(album=some)))";
	filter = mafw_filter_parse(original);
	filter->parts[0]->type = mafw_f_not;
	copy = mafw_filter_copy(filter);
	result = mafw_filter_to_string(filter);
	fail_if(copy != NULL,
		"Copy of incorrect filters should be null and it is not");
	fail_if(result != NULL,
		"Result of to_string should be null and it is not: %s", result);
	mafw_filter_free(filter);

	original = "(&(genre=rock))";
	filter = mafw_filter_parse(original);
	filter->parts[0]->type = MAFW_F_COMPLEX;
	copy = mafw_filter_copy(filter);
	result = mafw_filter_to_string(filter);
	fail_if(copy != NULL,
		"Copy of incorrect filters should be null and it is not");
	fail_if(result != NULL,
		"Result of to_string should be null and it is not: %s", result);
	mafw_filter_free(filter);

	original = "(|(genre=rock))";
	filter = mafw_filter_parse(original);
	filter->parts[0]->type = mafw_f_exists;
	copy = mafw_filter_copy(filter);
	result = mafw_filter_to_string(filter);
	fail_if(copy != NULL,
		"Copy of incorrect filters should be null and it is not");
	fail_if(result != NULL,
		"Result of to_string should be null and it is not: %s", result);
	mafw_filter_free(filter);
}
END_TEST

int main(void)
{
	TCase *tc;
	Suite *suite;

	suite = suite_create("Mafwilter");

	tc = tcase_create("Mafwilter");
	tcase_add_test(tc, test_simple);
	tcase_add_test(tc, test_bad);
	tcase_add_test(tc, test_quote);
	tcase_add_test(tc, test_unquote);
	tcase_add_test(tc, test_complex_not);
	tcase_add_test(tc, test_simple_2);
	tcase_add_test(tc, test_complex);
	tcase_add_test(tc, test_complex_2);
	tcase_add_test(tc, test_new);
	tcase_add_test(tc, test_add_child);
	tcase_add_test(tc, test_build_sql);
	tcase_add_test(tc, test_build_url);
	tcase_add_test(tc, test_parse_to_string_copy);
	suite_add_tcase(suite, tc);

	return checkmore_run(srunner_create(suite), FALSE);
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0 foldmethod=marker: */
