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
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libmafw/mafw-extension.h>
#include <libmafw/mafw-source.h>
#include <libmafw/mafw-errors.h>
#include <libmafw/mafw-uri-source.h>
#include <libmafw/mafw-registry.h>

#include "checkmore.h"

#define MAFW_TEST_NAME "TeSt"
#define MAFW_TEST_NAME2 "TESt"

/* Unit tests:
 *
 * x set property with mafw function
 * x set property with gobject function
 * x monitor the "notify" signal
 */

/* Dummy extension subclass */

static GType xyzzy_get_type(void);
typedef struct { MafwExtensionClass parent; } XyzzyClass;
typedef struct { MafwExtension parent; } Xyzzy;
G_DEFINE_TYPE(Xyzzy, xyzzy, MAFW_TYPE_EXTENSION);

static const GPtrArray *lsp(MafwExtension *self)
{
	mafw_extension_add_property(self, "logo", G_TYPE_STRING);
	return MAFW_EXTENSION_CLASS(xyzzy_parent_class)->list_extension_properties(self);
}

static void ssp(MafwExtension *self, const gchar *name, const GValue *value)
{
	mafw_extension_emit_property_changed(self, name, value);
}

static void gsp(MafwExtension *self, const gchar *name,
		MafwExtensionPropertyCallback cb, gpointer udata)
{
	GValue v = { 0 };

	if (!strcmp(name, "brightness")) {
		g_value_init(&v, G_TYPE_DOUBLE);
		g_value_set_double(&v, 2.1);
		cb(self, name, &v, udata, NULL);
	} else if (!strcmp(name, "saturation")) {
		g_value_init(&v, G_TYPE_DOUBLE);
		g_value_set_double(&v, 14.33);
		cb(self, name, &v, udata, NULL);
	} else if (!strcmp(name, "logo")) {
		g_value_init(&v, G_TYPE_STRING);
		g_value_set_static_string(&v, "baromsag");
		cb(self, name, &v, udata, NULL);
	} else {
		GError *e;

		cb(self, name, NULL, udata,
		   e = g_error_new(MAFW_EXTENSION_ERROR,
				   MAFW_EXTENSION_ERROR_INVALID_PROPERTY,
				   "invalid property: '%s'", name));
		g_error_free(e);
	}
}

static void xyzzy_class_init(XyzzyClass *x)
{
	MAFW_EXTENSION_CLASS(x)->list_extension_properties = lsp;
	MAFW_EXTENSION_CLASS(x)->set_extension_property = ssp;
	MAFW_EXTENSION_CLASS(x)->get_extension_property = gsp;
}
static void xyzzy_init(Xyzzy *y)
{
	mafw_extension_add_property(MAFW_EXTENSION(y), "brightness", G_TYPE_DOUBLE);
	mafw_extension_add_property(MAFW_EXTENSION(y), "saturation", G_TYPE_DOUBLE);
	mafw_extension_add_property(MAFW_EXTENSION(y), "logo", G_TYPE_STRING);
}

/* Test cases. */

static void rtpc(MafwExtension *extension, const gchar *name,
		 const GValue *value, gboolean *user_data)
{
	fail_if(strcmp(name, "logo"));
	fail_if(strcmp(g_value_get_string(value), "on the wrong track"));
	*user_data = TRUE;
}

static void propcb(MafwExtension *extension, const gchar *name,
		   GValue *value, gboolean *udata, const GError *error)
{
	*udata = !strcmp(name, "saturation") && !error;
}

static void unknowncb(MafwExtension *extension, const gchar *name,
		      GValue *value, gboolean *udata, const GError *error)
{
	fail_if(strcmp(name, "nothere"));
	fail_unless(error != NULL);
	fail_unless(value == NULL);
	fail_unless(error->code == MAFW_EXTENSION_ERROR_INVALID_PROPERTY);
	*udata = TRUE;
}

START_TEST(test_runtime_props)
{
	gpointer extension;
	gboolean gotsignal, gotprop;
	const GPtrArray *props;

	extension = g_object_new(xyzzy_get_type(),
			    "uuid", "must be given",
			    NULL);
	/* Check that duplicate properties are handled correctly. */
	props = mafw_extension_list_properties(extension);
	fail_unless(props->len == 3);
	fail_if(strcmp(((MafwExtensionProperty *)props->pdata[0])->name, "brightness"));
	fail_if(strcmp(((MafwExtensionProperty *)props->pdata[1])->name, "saturation"));
	fail_if(strcmp(((MafwExtensionProperty *)props->pdata[2])->name, "logo"));
	/* Check property-changed signal emission. */
	gotsignal = FALSE;
	g_signal_connect(extension, "property-changed",
			 G_CALLBACK(rtpc), &gotsignal);
	mafw_extension_set_property_string(extension, "logo",
				      "on the wrong track");
	fail_unless(gotsignal);
	/* Try to set a wrong type. */
	fail_if(mafw_extension_set_property_double(extension, "logo", 2.34));
	fail_if(mafw_extension_set_property_boolean(extension, "notexist", TRUE));
	fail_if(mafw_extension_set_property_char(extension, "notexist", 34));
	fail_if(mafw_extension_set_property_uchar(extension, "notexist", 34));
	fail_if(mafw_extension_set_property_int(extension, "notexist", 34));
	fail_if(mafw_extension_set_property_int64(extension, "notexist", 34));
	fail_if(mafw_extension_set_property_uint(extension, "notexist", 34));
	fail_if(mafw_extension_set_property_uint64(extension, "notexist", 34));
	fail_if(mafw_extension_set_property_long(extension, "notexist", 34));
	fail_if(mafw_extension_set_property_ulong(extension, "notexist", 34));
	fail_if(mafw_extension_set_property_float(extension, "notexist", 34.0));
	fail_if(mafw_extension_set_property_double(extension, "notexist", 34.0));
	/* Get a property. */
	gotprop = FALSE;
	mafw_extension_get_property(extension, "saturation", (gpointer)propcb, &gotprop);
	fail_unless(gotprop);
	/* Try to get an unknown property. */
	mafw_extension_get_property(extension, "nothere", (gpointer)unknowncb, &gotprop);
	fail_unless(gotprop);

	g_object_unref(extension);
}
END_TEST

static void mafw_test_notify(GObject *gobject, GParamSpec *arg1,
			     gpointer user_data)
{
	*(gboolean *)user_data = TRUE;
}

START_TEST(test_mafw_extension_property)
{
	MafwExtension *object;
	gboolean got_signal;
	gchar *name, *uuid, *plugin;
	
	object = g_object_new(xyzzy_get_type(),
			      "uuid", "1234",
			      "plugin", "bedugaszol",
			      NULL);
	
	fail_if(strcmp("", mafw_extension_get_name(object)));
	fail_if(strcmp("1234", mafw_extension_get_uuid(object)));
	fail_if(strcmp("bedugaszol", mafw_extension_get_plugin(object)));

	checkmore_ignore("*construct property \"%s\"*");
	g_object_set(G_OBJECT(object), "uuid", "should fail", NULL);
	fail_if(strcmp("1234", mafw_extension_get_uuid(object)));
	g_object_set(G_OBJECT(object), "plugin", "this one too", NULL);
	fail_if(strcmp("bedugaszol", mafw_extension_get_plugin(object)));

	g_signal_connect(G_OBJECT(object), "notify",
			 G_CALLBACK(mafw_test_notify), &got_signal);
	got_signal = FALSE;
	g_object_set(G_OBJECT(object), "name", MAFW_TEST_NAME, NULL);
	fail_if(strcmp(MAFW_TEST_NAME, mafw_extension_get_name(object)));
	fail_unless(got_signal == TRUE, "Haven't received the signal");

	got_signal = FALSE;
	mafw_extension_set_name(object, MAFW_TEST_NAME2);
	fail_if(strcmp(MAFW_TEST_NAME2, mafw_extension_get_name(object)));
	fail_unless(got_signal,
		    "Haven't received the signal with the API function");
	g_object_get(object, "name", &name, "uuid", &uuid, "plugin", &plugin,
			NULL);
	fail_if(strcmp(MAFW_TEST_NAME2, name));
	fail_if(strcmp("bedugaszol", plugin));
	fail_if(strcmp("1234", uuid));
	g_free(name);
	g_free(plugin);
	g_free(uuid);
	g_object_unref(object);
}
END_TEST

START_TEST(test_split_object)
{
	gchar *lhs, *rhs;

	/* Normal case */
	fail_if(!mafw_source_split_objectid("alpha::beta/gamma", &lhs, &rhs));
	fail_unless(!strcmp(lhs, "alpha"));
	fail_unless(!strcmp(rhs, "beta/gamma"));
	g_free(lhs);
	g_free(rhs);

	/* $rhs is omitted */
	fail_if(!mafw_source_split_objectid("alpha::beta/gamma", &lhs, NULL));
	fail_unless(!strcmp(lhs, "alpha"));
	g_free(lhs);

	/* $lhs is omitted */
	fail_if(!mafw_source_split_objectid("alpha::beta/gamma", NULL, &rhs));
	fail_unless(!strcmp(rhs, "beta/gamma"));
	g_free(rhs);

	/* itemid contains "::" */
	fail_if(!mafw_source_split_objectid("alpha::beta::gamma", NULL, &rhs));
	fail_unless(!strcmp(rhs, "beta::gamma"));
	g_free(rhs);

	/* itemid is empty */
	fail_if(!mafw_source_split_objectid("alpha::", NULL, &rhs));
	fail_unless(!strcmp(rhs, ""));
	g_free(rhs);

	/* Both are omitted */
	fail_if(!mafw_source_split_objectid("alpha::beta/gamma", NULL, NULL));

	/* No itemid; should not touch $lhs and $rhs */
	lhs = "kiskacsa";
	rhs = "uszik";
	fail_if(mafw_source_split_objectid("alpha", &lhs, &rhs));
	fail_if(lhs != (void *)"kiskacsa");
	fail_if(rhs != (void *)"uszik");

	/* Completely empty */
	fail_if(mafw_source_split_objectid("", NULL, NULL));

	/* Missing objectid */
	expect_fallback(mafw_source_split_objectid(NULL, NULL, NULL), FALSE);
}
END_TEST

START_TEST(test_mkobject)
{
	const gchar *uri;
	gchar *objectid, *source, *item;

	/* Test how mafw_source_create_objectid() performs with real URIs. */
	uri = "aaaa://bbb/cccc/ddddddd/eeee";
	objectid = mafw_source_create_objectid(uri);

	mafw_source_split_objectid(objectid, &source, &item);
	fail_unless(!strcmp(source, MAFW_URI_SOURCE_UUID));
	fail_unless(!strcmp(item, uri));

	g_free(objectid);
	g_free(source);
	g_free(item);

	/* Test absolute paths. */
	objectid = mafw_source_create_objectid("/alpha/beta/gamma");
	fail_unless(~strcmp(objectid,
			    MAFW_URI_SOURCE_UUID"::"
			    "file:///alpha/beta/gamma"));
	g_free(objectid);

	objectid = "filwe";
	objectid = mafw_source_create_objectid(objectid);
	fail_if(!objectid);
	g_free(objectid);
	
	objectid = mafw_source_create_objectid("");
	fail_if(!objectid);
	g_free(objectid);
}
END_TEST

/* Testing the URI source */
/* MafwSourceMetadataResultCb callback when the URI-source is supposed
 * to return a URI. */
static void urisrc_test_cb_uri(MafwSource *src, const gchar *object_id,
			       	GHashTable *metadata, GMainLoop *loop,
			       	const GError *error)
{
	const GValue *value;

	/* Check that $metadata has the URI and it's what we expect. */
	fail_if(error);
	fail_if(!metadata);
	fail_if(!(value = mafw_metadata_first(metadata,
					      MAFW_METADATA_KEY_URI)));
	fail_if(!G_VALUE_HOLDS(value, G_TYPE_STRING));
	fail_unless(!strcmp(g_value_get_string(value), 
			    "aaaa://bbb/cccc/ddddddd/eeee"));

	g_main_loop_quit(loop);
}

/* MafwSourceMetadataResultCb callback when the URI-source is NOT
 * supposed to return the URI because we didn't ask for it. */
static void urisrc_test_cb_nouri(MafwSource *src, const gchar *object_id,
				  GHashTable *metadata, GMainLoop *loop,
				  const GError *error)
{
	/* There should be no error.  It's OK we have some $metadata,
	 * but it must not contain the URI. */
	fail_if(error);
	fail_if(metadata && mafw_metadata_first(metadata,
					       	MAFW_METADATA_KEY_URI));
	g_main_loop_quit(loop);
}

START_TEST(test_urisrc)
{
	GMainLoop *loop;
	gchar *objectid;
	MafwExtension *urisrc;

	/* First off verify that the URI-source is hidden. */
	fail_if(mafw_registry_get_sources(
		MAFW_REGISTRY(mafw_registry_get_instance())) != NULL);

	/* ...but it can be found in other ways. */
	urisrc = mafw_registry_get_extension_by_uuid(
				  MAFW_REGISTRY(mafw_registry_get_instance()),
				  MAFW_URI_SOURCE_UUID);
	fail_if(!urisrc);

	/* This is the URL we expect to be extracted from $objectid. */
	loop = g_main_loop_new(NULL, FALSE);
	objectid = mafw_source_create_objectid("aaaa://bbb/cccc/ddddddd/eeee");

	/* Test if it rejects non-urisrc objectids. */
	expect_ignore(mafw_source_get_metadata(
			      MAFW_SOURCE(urisrc),
			      "objectid",
			      MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
			      (MafwSourceMetadataResultCb)urisrc_test_cb_uri,
			      loop));

	/* Test if it rejects NULL metadata key arrays. */
	expect_ignore(mafw_source_get_metadata(
			      MAFW_SOURCE(urisrc),
			      objectid, NULL,
			      (MafwSourceMetadataResultCb)urisrc_test_cb_uri,
			      loop));

	/* Test if it rejects NULL result callbacks. */
	expect_ignore(mafw_source_get_metadata(
			      MAFW_SOURCE(urisrc),
			      objectid,
			      MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
			      NULL, loop));

	/*
	 * Time for some positive tests.
	 * See if we can get the URI---this is the normal use case.
	 * It also tests that the invocation of the callback is
	 * deferred and that the cbarg ($loop) is passed correctly.
	 */
	mafw_source_get_metadata(MAFW_SOURCE(urisrc),
		objectid, MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
		(MafwSourceMetadataResultCb)urisrc_test_cb_uri, loop);
	g_main_loop_run(loop);

	/* The same, but include some other metadata keys. */
	mafw_source_get_metadata(MAFW_SOURCE(urisrc),
		objectid, MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
					   MAFW_METADATA_KEY_URI),
		(MafwSourceMetadataResultCb)urisrc_test_cb_uri, loop);
	g_main_loop_run(loop);

	/* Test that we do NOT get the URL if we don't ask for it.
	 * This also checks URI-source is OK with empty metadata
	 * key lists. */
	mafw_source_get_metadata(MAFW_SOURCE(urisrc),
		objectid, MAFW_SOURCE_LIST(),
		(MafwSourceMetadataResultCb)urisrc_test_cb_nouri,
	       	loop);
	g_main_loop_run(loop);

	/* The same but use some other key in place of an empty list. */
	mafw_source_get_metadata(MAFW_SOURCE(urisrc),
		objectid, MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE),
		(MafwSourceMetadataResultCb)urisrc_test_cb_nouri,
	       	loop);
	g_main_loop_run(loop);
	g_main_loop_unref(loop);
	g_free(objectid);
}
END_TEST

int main(void)
{
	TCase *tc;
	Suite *suite;

	suite = suite_create("Mafw extension");
	tc = tcase_create("Mafw extension");
	tcase_add_test(tc, test_mafw_extension_property);
	tcase_add_test(tc, test_runtime_props);
	tcase_add_test(tc, test_split_object);
	tcase_add_test(tc, test_mkobject);
	tcase_add_test(tc, test_urisrc);
	suite_add_tcase(suite, tc);

	return checkmore_run(srunner_create(suite), TRUE);
}
