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
#include <glib.h>
#include <check.h>
#include <gmodule.h>
#include <string.h>

#include <libmafw/mafw.h>
#include <libmafw/mafw-registry.h>

#include "checkmore.h"
#include "nopsource.h"
#include "noprenderer.h"


gboolean renderer_added_ok = FALSE;
gboolean source_added_ok = FALSE;
gboolean renderer_removed_ok = FALSE;
gboolean source_removed_ok = FALSE;

static void _deinitialize(GError **error)
{
}

static gboolean _initialize_error(MafwRegistry *registry, GError **error)
{
	g_set_error(error, 2101, 31337, "All your base are belong to us");
	return FALSE;
}

G_MODULE_EXPORT MafwPluginDescriptor libmafw_r_test_error_plugin_description = {
        { .name = "Registry error test plugin" },
        .initialize = _initialize_error,
        .deinitialize = _deinitialize,
};

static void renderer_added(MafwRegistry * self,
		       gpointer added_renderer, gpointer userdata)
{
	renderer_added_ok = TRUE;
}

static void source_added(MafwRegistry * self,
			 gpointer added_source, gpointer userdata)
{
	source_added_ok = TRUE;
}

static void renderer_removed(MafwRegistry * self,
			 gpointer removed_renderer, gpointer userdata)
{
	renderer_removed_ok = TRUE;
}

static void source_removed(MafwRegistry * self,
			 gpointer removed_source, gpointer userdata)
{
	source_removed_ok = TRUE;
}

START_TEST(test_registry_error_signaling)
{
	MafwRegistry *reg;
	GError *err;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Registry construction failed");
	err = NULL;
	mafw_registry_load_plugin(reg, "libmafw_r_test_error", &err);
	fail_if(err->domain != 2101);
	fail_if(err->code != 31337);
	fail_if(strcmp(err->message, "All your base are belong to us"));
	g_error_free(err);
	g_object_unref(reg);
}
END_TEST

/*****************************************************************************
 * Add renderer
 *****************************************************************************/

START_TEST(test_add_renderer)
{
	MafwRegistry *reg = NULL;
	MafwRenderer *renderer = NULL;
	GList *renderers = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "renderer_added", (GCallback) renderer_added, NULL);

	renderer = MAFW_RENDERER(g_object_new(nop_renderer_get_type(), "uuid", "SomeRenderer",
				      NULL));

	/* Add a renderer */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer));
	fail_unless(renderer_added_ok, "Renderer add failed\n");

	/* Get renderers */
	renderers = mafw_registry_get_renderers(reg);
	fail_unless(g_list_length(renderers) == 1, "Renderers list size wrong\n");

	g_object_unref(reg);
}

END_TEST START_TEST(test_add_same_renderer_twice)
{
	MafwRegistry *reg = NULL;
	MafwRenderer *renderer = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "renderer_added", (GCallback) renderer_added, NULL);

	renderer = MAFW_RENDERER(g_object_new(nop_renderer_get_type(), "uuid", "SomeRenderer",
				      NULL));

	/* Add a renderer */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer));
	fail_unless(renderer_added_ok, "Renderer add failed\n");

	/* Add it again... This should assert */
	expect_assert(mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer)));
}

END_TEST START_TEST(test_add_3_renderers)
{
	MafwRegistry *reg = NULL;
	MafwRenderer *renderer1 = NULL;
	MafwRenderer *renderer2 = NULL;
	MafwRenderer *renderer3 = NULL;
	GList *renderers = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "renderer_added", (GCallback) renderer_added, NULL);

	renderer1 = MAFW_RENDERER(g_object_new(nop_renderer_get_type(), "uuid", "SomeRenderer1",
				       NULL));
	renderer2 = MAFW_RENDERER(g_object_new(nop_renderer_get_type(), "uuid", "SomeRenderer2",
				       NULL));
	renderer3 = MAFW_RENDERER(g_object_new(nop_renderer_get_type(), "uuid", "SomeRenderer3",
				       NULL));

	/* Add renderer1 */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer1));
	fail_unless(renderer_added_ok, "Renderer add failed\n");
	renderers = mafw_registry_get_renderers(reg);
	fail_unless(g_list_length(renderers) == 1, "Renderers list size wrong\n");
	renderer_added_ok = FALSE;

	/* Add renderer2 */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer2));
	fail_unless(renderer_added_ok, "Renderer add failed\n");
	renderers = mafw_registry_get_renderers(reg);
	fail_unless(g_list_length(renderers) == 2, "Renderers list size wrong\n");
	renderer_added_ok = FALSE;

	/* Add renderer3 */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer3));
	fail_unless(renderer_added_ok, "Renderer add failed\n");
	renderers = mafw_registry_get_renderers(reg);
	fail_unless(g_list_length(renderers) == 3, "Renderers list size wrong\n");

	g_object_unref(reg);
}

END_TEST START_TEST(test_add_null_renderer)
{
	MafwRegistry *reg = NULL;
	MafwRenderer *renderer = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "renderer_added", (GCallback) renderer_added, NULL);

	/* Add a NULL renderer. This should assert */
	expect_assert(mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer)));
}
END_TEST

/*****************************************************************************
 * Remove renderer
 *****************************************************************************/
START_TEST(test_remove_renderer)
{
	MafwRegistry *reg = NULL;
	MafwRenderer *renderer = NULL;
	GList *renderers = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "renderer_removed", (GCallback) renderer_removed, NULL);

	renderer = MAFW_RENDERER(g_object_new(nop_renderer_get_type(), "uuid", "SomeRenderer",
				      NULL));

	/* Add a renderer */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer));

	/* Get renderers */
	renderers = mafw_registry_get_renderers(reg);
	fail_unless(g_list_length(renderers) == 1, "Renderers list size wrong\n");

	/* Remove the renderer */
	mafw_registry_remove_extension(reg, MAFW_EXTENSION(renderer));
	fail_unless(renderer_removed_ok, "Renderer remove failed\n");

	renderers = mafw_registry_get_renderers(reg);
	fail_unless(g_list_length(renderers) == 0, 
		    "Renderers list size wrong after remove\n");

	g_object_unref(reg);
}
END_TEST

START_TEST(test_remove_non_existing_renderer)
{
	MafwRegistry *reg = NULL;
	MafwRenderer *renderer1 = NULL;
	MafwRenderer *renderer2 = NULL;
	GList *renderers = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "renderer_removed", (GCallback) renderer_removed, NULL);

	renderer1 = MAFW_RENDERER(g_object_new(nop_renderer_get_type(), "uuid", "SomeRenderer",
				       NULL));
	renderer2 = MAFW_RENDERER(g_object_new(nop_renderer_get_type(), "uuid", "SomeRenderer2",
				       NULL));

	/* Add a renderer */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer1));

	/* Get renderers */
	renderers = mafw_registry_get_renderers(reg);
	fail_unless(g_list_length(renderers) == 1, "Renderers list size wrong\n");

	/* Remove the renderer (which shouldn't exist in the registry */
	expect_assert(mafw_registry_remove_extension(reg, MAFW_EXTENSION(renderer2)));
	fail_unless(renderer_removed_ok == FALSE, 
		    "Shouldn't signal non-existing renderer removal\n");

	renderers = mafw_registry_get_renderers(reg);
	fail_unless(g_list_length(renderers) == 1, "Renderers list size wrong\n");

	g_object_unref(reg);
}
END_TEST 

START_TEST(test_remove_null_renderer)
{
	MafwRegistry *reg = NULL;
	MafwRenderer *renderer = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "renderer_removed", (GCallback) renderer_removed, NULL);

	/* Add a NULL renderer. This should assert */
	expect_assert(mafw_registry_remove_extension(reg, MAFW_EXTENSION(renderer)));
}
END_TEST 

/*****************************************************************************
 * Get renderers
 *****************************************************************************/
/* TODO... */

START_TEST(test_get_renderer_by_uuid)
{
	MafwRegistry *reg = NULL;
	MafwRenderer *renderer = NULL;
	MafwRenderer *renderer_by_uuid = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "renderer_added", (GCallback) renderer_added, NULL);

	renderer = g_object_new(nop_renderer_get_type(),
			    "name", "test_renderer_name",
			    "uuid", "test_renderer_uuid",
			    NULL);

	/* Get renderer by uuid when it doesn't exist */
	renderer_by_uuid = MAFW_RENDERER(
		mafw_registry_get_extension_by_uuid(reg, "test_renderer_uuid"));
	fail_unless(renderer_by_uuid == NULL, 
		    "There was not supposed to be any renderer with the uuid\n");

	/* Add a renderer */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(renderer));
	fail_unless(renderer_added_ok, "Renderer add failed\n");

	/* Get renderer by uuid */
	renderer_by_uuid = MAFW_RENDERER(mafw_registry_get_extension_by_uuid(reg, "test_renderer_uuid"));
	fail_unless(renderer_by_uuid != NULL, "Get renderer by uuid failed\n");

	/* Get renderer by uuid when it doesn't exist */
	renderer_by_uuid = MAFW_RENDERER(mafw_registry_get_extension_by_uuid(reg, "wrong_renderer_uuid"));
	fail_unless(renderer_by_uuid == NULL, 
		    "There was not supposed to be any renderer with the uuid\n");

	/* Get renderer by NULL uuid */
	expect_fallback(mafw_registry_get_extension_by_uuid(reg, NULL), NULL);

	g_object_unref(reg);
} END_TEST


/*****************************************************************************
 * Add source
 *****************************************************************************/
START_TEST(test_add_source)
{
	MafwRegistry *reg = NULL;
	MafwSource *source = NULL;
	GList *sources = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "source_added", (GCallback) source_added, NULL);

	source = g_object_new(nop_source_get_type(),
			      "uuid", "SomeSource",
			      "name", "nop",
			      NULL);

	/* Add a source */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(source));
	fail_unless(source_added_ok, "Source add failed\n");

	/* Get sources */
	sources = mafw_registry_get_sources(reg);
	fail_unless(g_list_length(sources) == 1, "Sources list size wrong\n");

	g_object_unref(reg);
}

END_TEST START_TEST(test_add_same_source_twice)
{
	MafwRegistry *reg = NULL;
	MafwSource *source = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "source_added", (GCallback) source_added, NULL);

	source = g_object_new(nop_source_get_type(),
			      "uuid", "SomeSource",
			      "name", "nop",
			      NULL);

	/* Add a source */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(source));
	fail_unless(source_added_ok, "Source add failed\n");

	/* Add it again... This should assert */
	expect_assert(mafw_registry_add_extension(reg, MAFW_EXTENSION(source)));
}

END_TEST START_TEST(test_add_3_sources)
{
	MafwRegistry *reg = NULL;
	MafwSource *source1 = NULL;
	MafwSource *source2 = NULL;
	MafwSource *source3 = NULL;
	GList *sources = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "source_added", (GCallback) source_added, NULL);

	source1 = g_object_new(nop_source_get_type(),
			       "uuid", "SomeSource1",
			       "name", "nop",
			       NULL);
	source2 = g_object_new(nop_source_get_type(),
			       "uuid", "SomeSource2",
			       "name", "nop",
			       NULL);
	source3 = g_object_new(nop_source_get_type(),
			       "uuid", "SomeSource3",
			       "name", "nop",
			       NULL);

	/* Add source1 */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(source1));
	fail_unless(source_added_ok, "Source add failed\n");
	sources = mafw_registry_get_sources(reg);
	fail_unless(g_list_length(sources) == 1, "Sources list size wrong\n");
	source_added_ok = FALSE;

	/* Add source2 */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(source2));
	fail_unless(source_added_ok, "Source add failed\n");
	sources = mafw_registry_get_sources(reg);
	fail_unless(g_list_length(sources) == 2, "Sources list size wrong\n");
	source_added_ok = FALSE;

	/* Add source3 */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(source3));
	fail_unless(source_added_ok, "Source add failed\n");
	sources = mafw_registry_get_sources(reg);
	fail_unless(g_list_length(sources) == 3, "Sources list size wrong\n");

	g_object_unref(reg);
}

END_TEST START_TEST(test_add_null_source)
{
	MafwRegistry *reg = NULL;
	MafwSource *source = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "source_added", (GCallback) source_added, NULL);

	/* Add a NULL source. This should assert */
	expect_assert(mafw_registry_add_extension(reg, MAFW_EXTENSION(source)));
}

END_TEST
/*****************************************************************************
 * Remove source
 *****************************************************************************/

START_TEST(test_remove_source)
{
	MafwRegistry *reg = NULL;
	MafwSource *source = NULL;
	GList *sources = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "source_removed", (GCallback) source_removed, NULL);

	source = g_object_new(nop_source_get_type(),
			      "uuid", "SomeSource",
			      "name", "nop",
			      NULL);

	/* Add a source */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(source));

	/* Get sources */
	sources = mafw_registry_get_sources(reg);
	fail_unless(g_list_length(sources) == 1, "Sources list size wrong\n");

	/* Remove the source */
	mafw_registry_remove_extension(reg, MAFW_EXTENSION(source));
	fail_unless(source_removed_ok, "Source remove failed\n");

	sources = mafw_registry_get_sources(reg);
	fail_unless(g_list_length(sources) == 0, 
		    "Sources list size wrong after remove\n");

	g_object_unref(reg);
}
END_TEST

START_TEST(test_remove_non_existing_source)
{
	MafwRegistry *reg = NULL;
	MafwSource *source1 = NULL;
	MafwSource *source2 = NULL;
	GList *sources = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "source_removed", (GCallback) source_removed, NULL);

	source1 = g_object_new(nop_source_get_type(),
			       "uuid", "SomeSource",
			       "name", "nop",
			       NULL);
	source2 = g_object_new(nop_source_get_type(),
			       "uuid", "SomeSource2",
			       "name", "nop",
			       NULL);

	/* Add a source */
	mafw_registry_add_extension(reg, MAFW_EXTENSION(source1));

	/* Get sources */
	sources = mafw_registry_get_sources(reg);
	fail_unless(g_list_length(sources) == 1, "Sources list size wrong\n");

	/* Remove the source (which shouldn't exist in the registry */
	expect_assert(mafw_registry_remove_extension(reg, MAFW_EXTENSION(source2)));
	fail_unless(source_removed_ok == FALSE, 
		    "Shouldn't signal non-existing source removal\n");

	sources = mafw_registry_get_sources(reg);
	fail_unless(g_list_length(sources) == 1, "Sources list size wrong\n");

	g_object_unref(reg);
}
END_TEST 

START_TEST(test_remove_null_source)
{
	MafwRegistry *reg = NULL;
	MafwSource *source = NULL;

	reg = g_object_new(MAFW_TYPE_REGISTRY, NULL);
	fail_unless(reg != NULL, "Object construction failed");

	g_signal_connect(reg, "source_removed", (GCallback) source_removed, NULL);

	/* Add a NULL source. This should assert */
	expect_assert(mafw_registry_remove_extension(reg, MAFW_EXTENSION(source)));
}
END_TEST 

/*****************************************************************************
 * Test case execution
 *****************************************************************************/
int main(void)
{
	TCase *tc;
	Suite *suite;

	suite = suite_create("MafwRegistry");

	tc = tcase_create("Add renderer");
	tcase_add_test(tc,		test_add_renderer);
	checkmore_add_aborting_test(tc,	test_add_same_renderer_twice);
	tcase_add_test(tc,		test_add_3_renderers);
	checkmore_add_aborting_test(tc,	test_add_null_renderer);
	tcase_add_test(tc,		test_get_renderer_by_uuid);
	suite_add_tcase(suite, tc);

	tc = tcase_create("Remove renderer");
	tcase_add_test(tc,		test_remove_renderer);
	checkmore_add_aborting_test(tc,	test_remove_non_existing_renderer);
	checkmore_add_aborting_test(tc,	test_remove_null_renderer);
	suite_add_tcase(suite, tc);

	tc = tcase_create("Add source");
	tcase_add_test(tc,		test_add_source);
	checkmore_add_aborting_test(tc,	test_add_same_source_twice);
	tcase_add_test(tc,		test_add_3_sources);
	checkmore_add_aborting_test(tc, test_add_null_source);
	suite_add_tcase(suite, tc);

	tc = tcase_create("Remove source");
	tcase_add_test(tc,		test_remove_source);
	checkmore_add_aborting_test(tc,	test_remove_non_existing_source);
	checkmore_add_aborting_test(tc,	test_remove_null_source);
	suite_add_tcase(suite, tc);

	tc = tcase_create("Registry error signaling");
	tcase_add_test(tc,		test_registry_error_signaling);
	suite_add_tcase(suite, tc);

	return checkmore_run(srunner_create(suite), FALSE);
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
