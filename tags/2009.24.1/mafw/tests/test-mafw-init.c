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
/* Plugin loading tests. */
#include <glib.h>
#include <glib/gstdio.h>
#include <check.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <gmodule.h>

#include "libmafw/mafw-registry.h"
#include "libmafw/mafw-errors.h"

#include "checkmore.h"

/* Private variables. */
static MafwRegistry *Reg;
static GError *Err;
static gboolean Ret;
static guint Extensions_present = 0;

/* Some fake (`mock' is soo yesterday) plugins. */
static guint Initializer_callcount = 0;

static gboolean d_success(MafwRegistry *registry, GError **error)
{
	Initializer_callcount++;
	return TRUE;
}

static gboolean d_failing(MafwRegistry *registry, GError **error)
{
	Initializer_callcount++;
	g_set_error(error, MAFW_ERROR, MAFW_ERROR_PLUGIN_INIT_FAILED,
		    "CPU explodes in $@sde");
	return FALSE;
}

static gboolean Uninit_1_called, Uninit_2_called, Uninit_3_called;

static void plugin_1_uninit(GError **error)
{
	Uninit_1_called = TRUE;
}

static void plugin_2_uninit(GError **error)
{
	Uninit_2_called = TRUE;
}

static void plugin_3_uninit(GError **error)
{
	Uninit_3_called = TRUE;
}

MafwPluginDescriptor descriptor1_plugin_description =
{
	{ .name = "should not fail 1" },
	.initialize = d_success,
	.deinitialize = plugin_1_uninit,
};

MafwPluginDescriptor descriptor2_plugin_description =
{
	{ .name = "should not fail 2" },
	.initialize = d_success,
	.deinitialize = plugin_2_uninit,
};

MafwPluginDescriptor descriptor3_plugin_description =
{
	{ .name = "should fail 3" },
	.initialize = d_failing,
	.deinitialize = plugin_3_uninit,
};

static void extension_added(MafwRegistry *self, gpointer src, gpointer _)
{
	Extensions_present++;
}

static void extension_removed(MafwRegistry *self, gpointer src, gpointer _)
{
	Extensions_present--;
}

/* See if $regi has exactly the plugins whose names are in the arguments. */
static void check_plugin_list(const gchar *first_name, ...)
{
	va_list names;
	const gchar *name;
	GList *elist, *li;

	va_start(names, first_name);
	li = elist = mafw_registry_list_plugins(Reg);
	name = first_name;
	while (name) {
		const MafwPluginDescriptorPublic *pdp;

		fail_if(li == NULL, "expected more plugins");
		fail_if(li->data == NULL, "NULL pdp, sounds like a bug.");
		pdp = li->data;
		fail_if(pdp->name == NULL, "plugin name NULL");
		fail_if(strcmp(pdp->name, name), "expected different plugin");

		li = li->next;
		name = va_arg(names, const gchar *);
	}
	g_list_free(elist);
	fail_if(li != NULL, "more plugins than expected");
	va_end(names);
}

static void load(const gchar *name)
{
	Ret = mafw_registry_load_plugin(Reg, name, &Err);
}

static void unload(const gchar *name)
{
	Ret = mafw_registry_unload_plugin(Reg, name, &Err);
}

/* Test cases. */
START_TEST(test_singletonism)
{
	MafwRegistry *registry, *unidentified_mafwtastic_object;

	registry = mafw_registry_get_instance();
	/* Resistrance is futile!  You will be mafwimilated! */
	fail_unless(NULL != registry, "Mafw get resistry failed"
		    "(getting registry)\n");
	unidentified_mafwtastic_object = mafw_registry_get_instance();
	fail_unless(unidentified_mafwtastic_object == registry,
		    "BEEP BEEP, SINGLETON PATTERN VIOLATED!");
}
END_TEST

START_TEST(test_load_from_this)
{
	load("descriptor1");
	fail_unless(Ret && !Err);
	load("descriptor1");
	fail_unless(!Ret && Err &&
		    Err->domain == MAFW_ERROR &&
		    Err->code == MAFW_ERROR_PLUGIN_NAME_CONFLICT);
}
END_TEST

START_TEST(test_load_bogus)
{
	load("fna03ubifdlbvzie[g0abfaw4fb2");
	fail_unless(!Ret && Err &&
		    Err->domain == MAFW_ERROR &&
		    Err->code == MAFW_ERROR_PLUGIN_LOAD_FAILED);
}
END_TEST

START_TEST(test_load_absolute)
{
	gchar *cwd, *plugin;

	/* `make check' runs us in tests/. */
	cwd = g_get_current_dir();
	plugin = g_strconcat(cwd, "/.libs/test-extension.", G_MODULE_SUFFIX, NULL);
	g_free(cwd);
	load(plugin);
	fail_unless(Ret && !Err);
	g_free(plugin);
}
END_TEST

/* Sets MAFW_PLUGIN_DIR to $PWD/.libs */
static void override_plugindir(void)
{
	gchar *cwd, *plugindir;

	cwd = g_get_current_dir();
	plugindir = g_strconcat(cwd, "/.libs", NULL);
	g_setenv("MAFW_PLUGIN_DIR", plugindir, TRUE);
	g_free(cwd);
	g_free(plugindir);
}

START_TEST(test_load_from_plugindir)
{
	override_plugindir();
	g_rename("test-extension.la", "test-extension.libfool");
	load("test-extension");
	fail_unless(Ret && !Err);
	check_plugin_list("Test extension", NULL);
	g_rename("test-extension.libfool", "test-extension.la");
}
END_TEST

START_TEST(test_load_all)
{
	override_plugindir();
	mafw_registry_load_plugins(Reg);
	check_plugin_list("Test extension", NULL);
}
END_TEST

START_TEST(test_load_a_bunch)
{
	override_plugindir();
	load("descriptor1");
	load("descriptor2");
	load("descriptor3");
	load("test-extension");
	check_plugin_list("Test extension",
			  "should not fail 2",
			  "should not fail 1",
			  NULL);
}
END_TEST

START_TEST(test_unload)
{
	load("descriptor1");
	check_plugin_list("should not fail 1",
			  NULL);
	unload("descriptor1");
	fail_unless(Ret && !Err);
	check_plugin_list(NULL);
}
END_TEST

START_TEST(test_unload_all)
{
	load("descriptor1");
	check_plugin_list("should not fail 1",
			  NULL);
	load("descriptor2");
	check_plugin_list("should not fail 2",
			  "should not fail 1",
			  NULL);
	mafw_registry_unload_plugins(Reg);
	check_plugin_list(NULL);
}
END_TEST

START_TEST(test_extensions)
{
	g_signal_connect(Reg, "source-added", G_CALLBACK(extension_added), NULL);
	g_signal_connect(Reg, "source-removed", G_CALLBACK(extension_removed), NULL);
	g_rename("test-extension.la", "test-extension.libfool");
	override_plugindir();
	load("test-extension");
	fail_unless(Ret && !Err);
	check_plugin_list("Test extension", NULL);
	fail_unless(Extensions_present == 1);
	unload("test-extension");
	fail_unless(Ret && !Err);
	fail_unless(Extensions_present == 0);
	check_plugin_list(NULL);
	g_rename("test-extension.libfool", "test-extension.la");
}
END_TEST

START_TEST(test_initializers)
{
	Initializer_callcount = 0;
	check_plugin_list(NULL);

	load("descriptor1");
	fail_unless(Ret && !Err);
	g_clear_error(&Err);
	check_plugin_list("should not fail 1", NULL);

	load("descriptor2");
	fail_unless(Ret && !Err);
	g_clear_error(&Err);
	check_plugin_list("should not fail 2", "should not fail 1", NULL);

	load("descriptor3");
	fail_unless(!Ret && Err &&
		    Err->domain == MAFW_ERROR &&
		    Err->code == MAFW_ERROR_PLUGIN_INIT_FAILED);
	g_clear_error(&Err);
	check_plugin_list("should not fail 2", "should not fail 1", NULL);

	fail_unless(3 == Initializer_callcount);
	/* No ->deinitialized may have been called. */
	g_assert(!Uninit_1_called);
	g_assert(!Uninit_2_called);
	g_assert(!Uninit_3_called);

	/* The third plugin is not to be ->deinitialize()d
	 * because it never ->initialize()d in the first place. */
	mafw_registry_unload_plugins(Reg);
	check_plugin_list(NULL);
	fail_if(!Uninit_1_called);
	fail_if(!Uninit_2_called);
	fail_if( Uninit_3_called);
}
END_TEST

/* Fixtures. */
static void setup(void)
{
	Reg = mafw_registry_get_instance();
	Err = NULL;
}

static void teardown(void)
{
	g_clear_error(&Err);
}

int main(int argc, char *argv[])
{
	Suite *suite;
	TCase *tc;
	SRunner *runner;

	suite = suite_create("plugin loading");
	tc = tcase_create("still plugin loading?");
	tcase_add_unchecked_fixture(tc, setup, teardown);
	suite_add_tcase(suite, tc);

	if (1) tcase_add_test(tc, test_singletonism);
	if (1) tcase_add_test(tc, test_load_from_this);
	if (1) tcase_add_test(tc, test_load_bogus);
	if (1) tcase_add_test(tc, test_load_absolute);
	if (1) tcase_add_test(tc, test_load_from_plugindir);
	if (1) tcase_add_test(tc, test_load_all);
	if (1) tcase_add_test(tc, test_load_a_bunch);
	if (1) tcase_add_test(tc, test_unload);
	if (1) tcase_add_test(tc, test_unload_all);
	if (1) tcase_add_test(tc, test_extensions);
	if (1) tcase_add_test(tc, test_initializers);

	/* NOTE: these tests are not supposed to work in CK_NOFORK mode,
	 * because there is no convenient way (if any) to clear all loaded
	 * plugins from the single registry instance.  So, forking gives a
	 * clean sheet.  However, let's leave a debugging aid: if the CK_FORK
	 * environment variable is set, don't override it. */
	runner = srunner_create(suite);
	if (!g_getenv("CK_FORK"))
		srunner_set_fork_status(runner, CK_FORK);
	return checkmore_run(runner, FALSE);
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
