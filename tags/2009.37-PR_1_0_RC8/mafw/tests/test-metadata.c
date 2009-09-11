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

#include <string.h>
#include <glib.h>

#include "libmafw/mafw-source.h"
#include "libmafw/mafw-metadata.h"
#include "libmafw/mafw-filter.h"

#include "checkmore.h"

/* Macros {{{ */
/* For test_filter() */
/* Reduce noise.  These must be macros, otherwise file_*()
 * cannot report correct line numbers. */
#define FILTER(md, filter_str, outcome)			\
do {							\
	MafwFilter *filter;				\
							\
	filter = mafw_filter_parse(filter_str);		\
	outcome(mafw_metadata_filter(md, filter, NULL));\
	mafw_filter_free(filter);			\
} while (0)

#define FILTER_ACK(md, filter_str) FILTER(md, filter_str, fail_unless)
#define FILTER_NAK(md, filter_str) FILTER(md, filter_str, fail_if)

/* For test_compare() */
#define COMPARE(md1, rel, md2, sexp)			\
do {							\
	gchar **sorting;				\
							\
	sorting = mafw_metadata_sorting_terms(sexp);	\
	fail_unless(mafw_metadata_compare(md1, md2,	\
					  (const gchar *const *)sorting, \
					  NULL) rel 0);	\
	g_strfreev(sorting);				\
} while (0)
/* }}} */

/* test_metadata() {{{ */
/* Check a single-value tag of @md. */
static GValue *check_single(GHashTable *md, const gchar *key)
{
	GValue *value;

	value = mafw_metadata_first(md, key);
	fail_unless(g_hash_table_lookup(md, key) == value);
	fail_unless(mafw_metadata_nvalues(value) == 1);
	fail_unless(G_IS_VALUE(value));
	return value;
}

/* Check a multi-value tag of @md. */
static GValueArray *check_multi(GHashTable *md, const gchar *key,
			       	guint nexpected)
{
	GValueArray *values;

	values = g_hash_table_lookup(md, key);
	fail_unless(!G_IS_VALUE(values));
	fail_unless(mafw_metadata_nvalues(values) == nexpected);
	fail_unless(mafw_metadata_first(md, key) ==
		    g_value_array_get_nth(values, 0));
	return values;
}

/* Check a single integer value tag of @md. */
static void check_int(GHashTable *md, const gchar *key, gint expected)
{
	gpointer value;

	value = check_single(md, key);
	fail_unless(G_VALUE_HOLDS_INT(value));
	fail_unless(g_value_get_int(value) == expected);
}

/* Check a multiple integer value tag of @md. */
static void check_ints(GHashTable *md, const gchar *key, guint nexpected, ...)
{
	guint i;
	va_list expected;
	GValueArray *values;

	values = check_multi(md, key, nexpected);

	va_start(expected, nexpected);
	for (i = 0; i < nexpected; i++) {
		GValue *value;

		value = g_value_array_get_nth(values, i);
		fail_unless(G_IS_VALUE(value));
		fail_unless(G_VALUE_HOLDS_INT(value));
		fail_unless(g_value_get_int(value) == va_arg(expected, gint));
	}
	va_end(expected);
}

/* Check a single string value tag of @md. */
static void check_str(GHashTable *md, const gchar *key, const gchar *expected)
{
	GValue *value;

	value = check_single(md, key);
	fail_unless(G_VALUE_HOLDS_STRING(value));
	fail_unless(!strcmp(g_value_get_string(value), expected));
}

/* Check a multiple string value tag of @md. */
static void check_strs(GHashTable *md, const gchar *key, guint nexpected, ...)
{
	guint i;
	va_list expected;
	GValueArray *values;

	values = check_multi(md, key, nexpected);

	va_start(expected, nexpected);
	for (i = 0; i < nexpected; i++) {
		GValue *value;

		value = g_value_array_get_nth(values, i);
		fail_unless(G_IS_VALUE(value));
		fail_unless(G_VALUE_HOLDS_STRING(value));
		fail_unless(!strcmp(g_value_get_string(value),
				    va_arg(expected, const gchar *)));
	}
	va_end(expected);
}

/* Test all functions declared in mafw-metadata.h. */
START_TEST(test_metadata)
{
	GHashTable *md;
	GValue v1, v2, v3, v4;

	/* Create and fill $md with test metadata. */
	md = mafw_metadata_new();
	memset(&v1, 0, sizeof(v1));
	memset(&v2, 0, sizeof(v2));
	memset(&v3, 0, sizeof(v3));
	memset(&v4, 0, sizeof(v4));

	/* Integers */
	mafw_metadata_add_int(md, "lofasz",   10);
	mafw_metadata_add_int(md, "joska",    20);
	mafw_metadata_add_int(md, "dread",    1);
	mafw_metadata_add_int(md, "dread",    0);
	mafw_metadata_add_int(md, "dread",    1);
	mafw_metadata_add_int(md, "newspeak", 1, 9, 8, 4);

	/* Integers in GValue:s {{{ */
	g_value_init(&v1, G_TYPE_INT);
	g_value_init(&v2, G_TYPE_INT);
	g_value_init(&v3, G_TYPE_INT);
	g_value_init(&v4, G_TYPE_INT);

	g_value_set_int(&v1, -10);
	mafw_metadata_add_val(md, "miska",    &v1);
	g_value_set_int(&v1, -20);
	mafw_metadata_add_val(md, "pista",    &v1);
	g_value_set_int(&v1, 4);
	g_value_set_int(&v2, 0);
	g_value_set_int(&v3, 4);
	mafw_metadata_add_val(md, "lusta",    &v1);
	mafw_metadata_add_val(md, "lusta",    &v2);
	mafw_metadata_add_val(md, "lusta",    &v3);
	g_value_set_int(&v1, 1);
	g_value_set_int(&v2, 7);
	g_value_set_int(&v3, 8);
	g_value_set_int(&v4, 9);
	mafw_metadata_add_val(md, "fruska",   &v1, &v2, &v3, &v4);

	g_value_unset(&v1);
	g_value_unset(&v2);
	g_value_unset(&v3);
	g_value_unset(&v4);
	/* }}} */

	/* Strings */
	mafw_metadata_add_str(md, "trash",    "metal");
	mafw_metadata_add_str(md, "terror",   "news");
	mafw_metadata_add_str(md, "FUD",      "fear");
	mafw_metadata_add_str(md, "FUD",      "uncertainty");
	mafw_metadata_add_str(md, "FUD",      "doubt");
	mafw_metadata_add_str(md, "miff",     "meff", "maff", "muff");

	/* Strings in GValue:s {{{ */
	g_value_init(&v1, G_TYPE_STRING);
	g_value_init(&v2, G_TYPE_STRING);
	g_value_init(&v3, G_TYPE_STRING);
	g_value_init(&v4, G_TYPE_STRING);

	g_value_set_string(&v1, ":)");
	mafw_metadata_add_val(md, "durva",    &v1);
	g_value_set_string(&v1, ":-)");
	mafw_metadata_add_val(md, "kurva" ,   &v1);
	g_value_set_string(&v1, ":--)");
	g_value_set_string(&v2, "8--)");
	g_value_set_string(&v3, "8--}");
	mafw_metadata_add_val(md, "furja",    &v1);
	mafw_metadata_add_val(md, "furja",    &v2);
	mafw_metadata_add_val(md, "furja",    &v3);

	g_value_set_string(&v1, "8--}~");
	g_value_set_string(&v2, "8--X");
	g_value_set_string(&v3, "X--{");
	mafw_metadata_add_val(md, "pulyka",   &v1, &v2, &v3);

	g_value_unset(&v1);
	g_value_unset(&v2);
	g_value_unset(&v3);
	g_value_unset(&v4);
	/* }}} */

	/* Test that $md contains all the expected keys and values. */
	check_int(md,  "lofasz",      10);
	check_int(md,  "joska",       20);
	check_ints(md, "dread", 3,    1, 0, 1);
	check_ints(md, "newspeak", 4, 1, 9, 8, 4);

	check_int(md,  "miska",       -10);
	check_int(md,  "pista",       -20);
	check_ints(md, "lusta", 3,    4, 0, 4);
	check_ints(md, "fruska", 4,   1, 7, 8, 9);

	check_str(md,  "trash",       "metal");
	check_str(md,  "terror",      "news");
	check_strs(md, "FUD", 3,      "fear", "uncertainty", "doubt");
	check_strs(md, "miff", 3,     "meff", "maff", "muff");

	check_str(md, "durva",        ":)");
	check_str(md, "kurva" ,       ":-)");
	check_strs(md, "furja", 3,    ":--)", "8--)", "8--}");
	check_strs(md, "pulyka", 3,   "8--}~", "8--X", "X--{");

	/* See if mafw_metadata_nvalues() and mafw_metadata_first()
	 * work as expected with non-existent keys. */
	fail_unless(mafw_metadata_nvalues(
		 g_hash_table_lookup(md, "ganxta"))  == 0);
	fail_unless(mafw_metadata_first(md, "zolee") == NULL);

	/*
	 * Dump $md.  Since the traversing order of a hash table
	 * is undefined we cannot verify the output, but you can
	 * examine it visually if checkmore_set_logging() is disabled.
	 */
	mafw_metadata_print(md, NULL);
	mafw_metadata_print(md, "hehe");

	g_hash_table_unref(md);
}
END_TEST
/* }}} */

/* test_multikey_*() {{{ */
static GValue *mkgvint(gint i)
{
	GValue *v;

	v = g_new0(GValue, 1);
	g_value_init(v, G_TYPE_INT);
	g_value_set_int(v, i);
	return v;
}

static GValue *mkgvstr(const gchar *s)
{
	GValue *v;

	v = g_new0(GValue, 1);
	g_value_init(v, G_TYPE_STRING);
	g_value_set_string(v, s);
	return v;
}

static void freevg(GValue *v)
{
	g_value_unset(v);
	g_free(v);
}

/* test_multikey_int_*() {{{ */
START_TEST(test_multikey_int_int)
{
	GHashTable *md = mafw_metadata_new();
	mafw_metadata_add_int(md, "alpha", 10);
	mafw_metadata_add_int(md, "alpha", 20);
	check_ints(md, "alpha", 2, 10, 20);
	mafw_metadata_release(md);
}
END_TEST

START_TEST(test_multikey_int_gvint)
{
	GHashTable *md = mafw_metadata_new();
	GValue *v;
	mafw_metadata_add_int(md, "alpha", 10);
	mafw_metadata_add_val(md, "alpha", v = mkgvint(20));
	check_ints(md, "alpha", 2, 10, 20);
	freevg(v);
	mafw_metadata_release(md);
}
END_TEST

START_TEST(test_multikey_int_str)
{
	checkmore_ignore("*: assertion failed: *");
	GHashTable *md = mafw_metadata_new();
	mafw_metadata_add_int(md, "alpha", 10);
	mafw_metadata_add_str(md, "alpha", "20");
}
END_TEST

START_TEST(test_multikey_int_gvstr)
{
	checkmore_ignore("*: assertion failed: *");
	GHashTable *md = mafw_metadata_new();
	mafw_metadata_add_int(md, "alpha", 10);
	mafw_metadata_add_val(md, "alpha", mkgvstr("20"));
}
END_TEST
/* }}} */

/* test_multikey_gvint_*() {{{ */
START_TEST(test_multikey_gvint_int)
{
	GHashTable *md = mafw_metadata_new();
	GValue *v;
	mafw_metadata_add_val(md, "alpha", v = mkgvint(10));
	mafw_metadata_add_int(md, "alpha", 20);
	check_ints(md, "alpha", 2, 10, 20);
	freevg(v);
	mafw_metadata_release(md);
}
END_TEST

START_TEST(test_multikey_gvint_gvint)
{
	GHashTable *md = mafw_metadata_new();
	GValue *v1, *v2;
	mafw_metadata_add_val(md, "alpha", v1 = mkgvint(10));
	mafw_metadata_add_val(md, "alpha", v2 = mkgvint(20));
	check_ints(md, "alpha", 2, 10, 20);
	freevg(v1);
	freevg(v2);
	mafw_metadata_release(md);
}
END_TEST

START_TEST(test_multikey_gvint_str)
{
	checkmore_ignore("*: assertion failed: *");
	GHashTable *md = mafw_metadata_new();
	mafw_metadata_add_val(md, "alpha", mkgvint(10));
	mafw_metadata_add_str(md, "alpha", "20");
}
END_TEST

START_TEST(test_multikey_gvint_gvstr)
{
	checkmore_ignore("*: assertion failed: *");
	GHashTable *md = mafw_metadata_new();
	GValue *v1, *v2;
	mafw_metadata_add_val(md, "alpha", v1 = mkgvint(10));
	mafw_metadata_add_val(md, "alpha", v2 = mkgvstr("20"));
	freevg(v1);
	freevg(v2);
}
END_TEST
/* }}} */

/* test_multikey_str_*() {{{ */
START_TEST(test_multikey_str_int)
{
	checkmore_ignore("*: assertion failed: *");
	GHashTable *md = mafw_metadata_new();
	mafw_metadata_add_str(md, "alpha", "10");
	mafw_metadata_add_int(md, "alpha", 20);
}
END_TEST

START_TEST(test_multikey_str_gvint)
{
	checkmore_ignore("*: assertion failed: *");
	GHashTable *md = mafw_metadata_new();
	mafw_metadata_add_str(md, "alpha", "10");
	mafw_metadata_add_val(md, "alpha", mkgvint(20));
}
END_TEST

START_TEST(test_multikey_str_str)
{
	GHashTable *md = mafw_metadata_new();
	mafw_metadata_add_str(md, "alpha", "10");
	mafw_metadata_add_str(md, "alpha", "20");
	check_strs(md, "alpha", 2, "10", "20");
	mafw_metadata_release(md);
}
END_TEST

START_TEST(test_multikey_str_gvstr)
{
	GHashTable *md = mafw_metadata_new();
	GValue *v;
	mafw_metadata_add_str(md, "alpha", "10");
	mafw_metadata_add_val(md, "alpha", v = mkgvstr("20"));
	check_strs(md, "alpha", 2, "10", "20");
	freevg(v);
	mafw_metadata_release(md);
}
END_TEST
/* }}} */

/* test_multikey_gvstr_*() {{{ */
START_TEST(test_multikey_gvstr_int)
{
	checkmore_ignore("*: assertion failed: *");
	GHashTable *md = mafw_metadata_new();
	mafw_metadata_add_val(md, "alpha", mkgvstr("10"));
	mafw_metadata_add_int(md, "alpha", 20);
}
END_TEST

START_TEST(test_multikey_gvstr_gvint)
{
	checkmore_ignore("*: assertion failed: *");
	GHashTable *md = mafw_metadata_new();
	GValue *v1, *v2;
	mafw_metadata_add_val(md, "alpha", v1 = mkgvstr("10"));
	mafw_metadata_add_val(md, "alpha", v2 = mkgvint(20));
	freevg(v1);
	freevg(v2);
}
END_TEST

START_TEST(test_multikey_gvstr_str)
{
	GHashTable *md = mafw_metadata_new();
	GValue *v;
	mafw_metadata_add_val(md, "alpha", v = mkgvstr("10"));
	mafw_metadata_add_str(md, "alpha", "20");
	check_strs(md, "alpha", 2, "10", "20");
	freevg(v);
	mafw_metadata_release(md);
}
END_TEST

START_TEST(test_multikey_gvstr_gvstr)
{
	GHashTable *md = mafw_metadata_new();
	GValue *v1, *v2;
	mafw_metadata_add_val(md, "alpha", v1 = mkgvstr("10"));
	mafw_metadata_add_val(md, "alpha", v2 = mkgvstr("20"));
	check_strs(md, "alpha", 2, "10", "20");
	freevg(v1);
	freevg(v2);
	mafw_metadata_release(md);
}
END_TEST
/* }}} */
/* }}} */

/* test_relevant_keys() {{{ */
static void check_relevant_keys(const gchar *const *keys,
			  const gchar *sorting_str,
			  const gchar *filter_str,
			  const gchar *const *exp)
{
	guint i, o;
	const gchar **ret;
	gchar **sorting;
	MafwFilter *filter;

	filter = filter_str ? mafw_filter_parse(filter_str) : NULL;
	sorting = mafw_metadata_sorting_terms(sorting_str);

	ret = mafw_metadata_relevant_keys(keys, filter,
					  (const gchar *const *)sorting);

	fail_if(g_strv_length((gchar **)ret) != g_strv_length((gchar **)exp));
	for (i = 0; exp[i]; exp++) {
		for (o = 0; ret[o]; o++) {
			fail_if(!ret[o]);
			if (!strcmp(ret[o], exp[i]))
				break;
		}
	}

	g_free(ret);
	mafw_filter_free(filter);
	g_strfreev(sorting);
}

START_TEST(test_relevant_keys)
{
	const gchar *const *keys;

	fail_if(mafw_metadata_relevant_keys(NULL, NULL, NULL) != NULL);

	keys = MAFW_SOURCE_LIST("alpha", "beta", "gamma");
	check_relevant_keys(keys, NULL, NULL, keys);
	check_relevant_keys(keys, "alpha,beta,kappa", NULL,
			    MAFW_SOURCE_LIST("alpha", "beta",
					     "gamma", "kappa"));
	check_relevant_keys(keys, "alpha,beta,kappa", "(|(alpha=0)(zeta=1))",
			    MAFW_SOURCE_LIST("alpha", "beta",
					     "gamma", "kappa", "zeta"));
	check_relevant_keys(keys, "beta", "(|(alpha=0)(zeta=1))",
			    MAFW_SOURCE_LIST("alpha", "beta",
					     "gamma", "zeta"));
	check_relevant_keys(keys, "gamma", "(delta=10)",
			    MAFW_SOURCE_LIST("alpha", "beta",
					     "gamma", "delta"));
}
END_TEST
/* }}} */

/* test_filter() {{{ */
START_TEST(test_filter)
{
	GHashTable *md;

	md = mafw_metadata_new();
	mafw_metadata_add_int(md, "alpha", 10, 20, 30);
	mafw_metadata_add_str(md, "beta", "one", "two", "three");

	/* Simple expressions with integers */
	FILTER_ACK(md, "(alpha=10)");
	FILTER_ACK(md, "(alpha=20)");
	FILTER_NAK(md, "(alpha>30)");
	FILTER_ACK(md, "(alpha<30)");

	/* Simple expressions with strings */
	FILTER_ACK(md, "(beta=one)");
	FILTER_ACK(md, "(beta=two)");
	FILTER_ACK(md, "(beta=TWO)");
	FILTER_NAK(md, "(beta>twoooo)");
	FILTER_ACK(md, "(beta~t*e)");
	FILTER_ACK(md, "(beta~T*E)");
	FILTER_NAK(md, "(beta~t*ko)");
	FILTER_ACK(md, "(beta<threee)");
	FILTER_NAK(md, "(beta=four)");

	/* Complex expressions, all keys are valid */
	FILTER_ACK(md, "(&(alpha=10)(beta=one))");
	FILTER_NAK(md, "(&(alpha=15)(beta=one))");
	FILTER_ACK(md, "(|(alpha=15)(beta=one))");
	FILTER_NAK(md, "(|(alpha=15)(beta=ohne))");
	FILTER_ACK(md, "(!(alpha=15))");
	FILTER_NAK(md, "(!(alpha=10))");

	/* Complex expressions, some of the keys are not valid */
	FILTER_ACK(md, "(&(alpha=10)(berta=one))");
	FILTER_NAK(md, "(&(alpha=15)(berta=one))");
	FILTER_NAK(md, "(|(alpha=15)(berta=one))");
	FILTER_NAK(md, "(|(alpha=15)(berta=ohne))");
	FILTER_ACK(md, "(!(alpha=15))");
	FILTER_NAK(md, "(!(alpha=10))");

	/* Complex expressions, all keys are invalid */
	FILTER_ACK(md, "(&(karhu=10)(berta=one))");
	FILTER_ACK(md, "(|(karhu=15)(berta=one))");
	FILTER_ACK(md, "(!(karhu=15))");

	g_hash_table_unref(md);
}
END_TEST
/* }}} */

/* test_compare() {{{ */
START_TEST(test_compare)
{
	GHashTable *md1, *md2;

	/* Fill the test tables */
	md1 = mafw_metadata_new();
	mafw_metadata_add_int(md1, "alpha",	21, 80, 76, 35, 87);
	mafw_metadata_add_int(md1, "beta",	74, 81, 74, 51, 28);
	mafw_metadata_add_int(md1, "gamma",	14, 11, 14);
	mafw_metadata_add_int(md1, "delta",	35, 35, 35, 33);
	mafw_metadata_add_int(md1, "epsilon",	40, 64, 8, 50, 32);
	mafw_metadata_add_str(md1, "tau",
		      	      "wjkcfjn", "b", "arvagnwrs", "pjleoj", "ejmd2b");
	mafw_metadata_add_str(md1, "chi",
		      	      "kqppof", "ameu", "icpps", "yoagbqlv", "hhvdye");
	mafw_metadata_add_str(md1, "lambda",
		      	      "xix", "qh", "gbuhp", "li", "uamobg");
	mafw_metadata_add_str(md1, "omega",	"DIE");
	mafw_metadata_add_int(md1, "gazsi",	48);

	md2 = mafw_metadata_new();
	mafw_metadata_add_int(md2, "alpha",	89, 6, 7, 44, 88);
	mafw_metadata_add_int(md2, "beta",	74, 81, 74, 51, 28);
	mafw_metadata_add_int(md2, "gamma",	14, 11, 15);
	mafw_metadata_add_int(md2, "delta",	35, 35, 35);
	mafw_metadata_add_int(md2, "sigma",	55, 15, 65, 43, 61);
	mafw_metadata_add_str(md2, "tau",
		      	      "xecuqva", "whg", "tspygh", "xkjhqu", "qgjklw");
	mafw_metadata_add_str(md2, "chi",
		      	      "kqppof", "ameu", "icpps", "yoagbqlv", "hhvdye");
	mafw_metadata_add_str(md2, "zeta",
		      	      "qxxoxtsi", "fyc", "pcdrux", "vujeff", "vbhflr");
	mafw_metadata_add_str(md2, "omega",	"DIE", "DIE", "DIE",
			      			"MY DARLING");
	mafw_metadata_add_int(md2, "gazsi",	25);

	/* A hash table must always be equal with itself. */
	COMPARE(md1, ==, md1, "alpha");
	COMPARE(md1, ==, md1, "alpha,beta,tau,chi");
	COMPARE(md1, ==, md1, "alpha,berta,tata,chip");
	COMPARE(md1, ==, md1, "guvadtszemu,pavianok");

	/* Equality */
	COMPARE(md1, ==, md2, "beta");
	COMPARE(md1, ==, md2, "chi");
	COMPARE(md1, ==, md2, "beta,chi");
	COMPARE(md1, ==, md2, "guvadtszemu,pavianok");

	/* Equality with NULL */
	COMPARE(NULL, ==, NULL, "alpha");
	COMPARE(md1,   <, NULL, "alpha");
	COMPARE(md1,  ==, NULL, "alpha_centari");

	/* See if equal keys change anything */
	COMPARE(md1, <,  md2, "+alpha");
	COMPARE(md1, >,  md2, "-alpha");
	COMPARE(md1, <,  md2, "gamma");
	COMPARE(md1, <,  md2, "+beta,gamma");
	COMPARE(md1, <,  md2, "-beta,gamma");
	COMPARE(md1, >,  md2, "+beta,-gamma");
	COMPARE(md1, >,  md2, "-beta,-gamma");
	COMPARE(md1, >,  md2, "delta");
	COMPARE(md1, >,  md2, "chi,+delta");
	COMPARE(md1, <,  md2, "chi,-delta");

	/* See if the longer value wins;
	 * see if the second key is observed */
	COMPARE(md1, <,  md2, "tau");
	COMPARE(md1, <,  md2, "tau,delta");
	COMPARE(md1, >,  md2, "delta,tau");

	/* See if the one-larger value wins */
	COMPARE(md1, <,  md2, "epsilon");
	COMPARE(md1, <,  md2, "beta,epsilon");
	COMPARE(md1, <,  md2, "epsilon,delta");
	COMPARE(md1, >,  md2, "delta,epsilon");

	/* Single vs single/multi */
	COMPARE(md1, <,  md2, "+omega");
	COMPARE(md1, >,  md2, "-omega");
	COMPARE(md1, >,  md2, "+gazsi");
	COMPARE(md1, <,  md2, "-gazsi");

	g_hash_table_unref(md1);
	g_hash_table_unref(md2);
}
END_TEST
/* }}} */

int main(void)
{ /* {{{ */
	TCase *tc;
	Suite *suite;

	suite = suite_create("metadata accessories");
	checkmore_add_tcase(suite, "set and get metadata", test_metadata);

	tc = tcase_create("multivalued metadata");
	tcase_add_test(tc,		test_multikey_int_int);
	tcase_add_test(tc,		test_multikey_int_gvint);
	checkmore_add_aborting_test(tc,	test_multikey_int_str);
	checkmore_add_aborting_test(tc,	test_multikey_int_gvstr);

	tcase_add_test(tc,		test_multikey_gvint_int);
	tcase_add_test(tc,		test_multikey_gvint_gvint);
	checkmore_add_aborting_test(tc,	test_multikey_gvint_str);
	checkmore_add_aborting_test(tc,	test_multikey_gvint_gvstr);

	checkmore_add_aborting_test(tc,	test_multikey_str_int);
	checkmore_add_aborting_test(tc,	test_multikey_str_gvint);
	tcase_add_test(tc,		test_multikey_str_str);
	tcase_add_test(tc,		test_multikey_str_gvstr);

	checkmore_add_aborting_test(tc,	test_multikey_gvstr_int);
	checkmore_add_aborting_test(tc,	test_multikey_gvstr_gvint);
	tcase_add_test(tc,		test_multikey_gvstr_str);
	tcase_add_test(tc,		test_multikey_gvstr_gvstr);
	suite_add_tcase(suite, tc);

	checkmore_add_tcase(suite, "getting relevant keys", test_relevant_keys);
	checkmore_add_tcase(suite, "filter by metadata",   test_filter);
	checkmore_add_tcase(suite, "sort by metadata",     test_compare);

	return checkmore_run(srunner_create(suite), FALSE);
} /* }}} */

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0 foldmethod=marker: */
