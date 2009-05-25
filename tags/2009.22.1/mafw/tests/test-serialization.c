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

#include <check.h>
#include <glib.h>
#include <glib-object.h>

#include <libmafw/mafw-metadata.h>

#include "checkmore.h"
#include <libmafw/mafw-metadata-serializer.h>

static void compare_gvals(GValue *val1, GValue *val2)
{
	GType type;

	type = G_VALUE_TYPE(val1);
	fail_unless(G_VALUE_TYPE(val2) == type);
	switch (type) {
	case G_TYPE_INT:
		fail_unless(g_value_get_int(val1) == g_value_get_int(val2));
		break;
	case G_TYPE_UINT:
		fail_unless(g_value_get_uint(val1) == g_value_get_uint(val2));
		break;
	case G_TYPE_LONG:
		fail_unless(g_value_get_long(val1) == g_value_get_long(val2));
		break;
	case G_TYPE_ULONG:
		fail_unless(g_value_get_ulong(val1) == g_value_get_ulong(val2));
		break;
	case G_TYPE_INT64:
		fail_unless(g_value_get_int64(val1) == g_value_get_int64(val2));
		break;
	case G_TYPE_UINT64:
		fail_unless(g_value_get_uint64(val1) == g_value_get_uint64(val2));
		break;
	case G_TYPE_DOUBLE:
		/* The two representations should match bit-by-bit. */
		fail_unless(g_value_get_double(val1) == g_value_get_double(val2));
		break;
	case G_TYPE_STRING:
		fail_unless(!strcmp(g_value_get_string(val1),
				    g_value_get_string(val2)));
		break;
	default:
		fail();
	}
}

static void compare_cb(const gchar *key, gpointer val1, GHashTable *md2)
{
	gpointer val2;

	val2 = g_hash_table_lookup(md2, key);
	fail_unless(val2 != NULL);

	if (G_IS_VALUE(val1)) {
		fail_unless(G_IS_VALUE(val2));
		compare_gvals(val1, val2);
	} else {
		guint nvalues, i;

		fail_unless(!G_IS_VALUE(val2));
		nvalues = ((GValueArray *)val1)->n_values;
		fail_unless(((GValueArray *)val1)->n_values == nvalues);
		for (i = 0; i < nvalues; i++)
			compare_gvals(g_value_array_get_nth(val1, i),
				      g_value_array_get_nth(val2, i));
	}
}

START_TEST(test_serialization)
{
	gchar *stream;
	gsize sstream;
	GHashTable *src, *dst;

	stream = mafw_metadata_freeze(NULL, &sstream);
	fail_if(sstream != 0);

	dst = mafw_metadata_thaw(stream, sstream);
	fail_if(dst != NULL);
	g_free(stream);

	src = mafw_metadata_new();
	mafw_metadata_add_int(src, "blood",  10);
	mafw_metadata_add_int(src, "scream", 20);
	mafw_metadata_add_int(src, "pain",   9);
	mafw_metadata_add_int(src, "pain",   9);
	mafw_metadata_add_int(src, "pain",   9);
	mafw_metadata_add_int(src, "death",  1, 9, 9, 7);

	mafw_metadata_add_uint(src,   "uau",     2, 6, 0, 0);
	mafw_metadata_add_long(src,   "luau",    1, 3, 0, 0);
	mafw_metadata_add_ulong(src,  "uluau",   0, 0, 2, 6);
	mafw_metadata_add_int64(src,  "lluau",   0LL, 2LL, 6LL, 0LL);
	mafw_metadata_add_uint64(src, "ulluuau", 2LL, 3LL, 1LL, 6LL);
	mafw_metadata_add_double(src, "duau",
				 2.7182818284590452354,
				 1.41421356237309504880);

	mafw_metadata_add_str(src, ":-)",    ":-E");
	mafw_metadata_add_str(src, "8-]",    "#-|~~~");
	mafw_metadata_add_str(src, "*_*",    "kiss");
	mafw_metadata_add_str(src, "*_*",    "me");
	mafw_metadata_add_str(src, "*_*",    "now");
	mafw_metadata_add_str(src, "bimm",   "bamm", "bumm");

	stream = mafw_metadata_freeze(src, &sstream);
	dst = mafw_metadata_thaw(stream, sstream);
	g_hash_table_foreach(src, (GHFunc)compare_cb, dst);

	g_free(stream);
	g_hash_table_unref(src);
	g_hash_table_unref(dst);
}
END_TEST

int main(void)
{
	Suite *suite;

	suite = suite_create("metadata serialization");
	checkmore_add_tcase(suite, "freeze & thaw", test_serialization);
	return checkmore_run(srunner_create(suite), FALSE);
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
