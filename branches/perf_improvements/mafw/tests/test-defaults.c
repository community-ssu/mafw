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

#include "checkmore.h"
#include <libmafw/mafw.h>

/* Test extensions */

static GType fsrc_get_type(void);
typedef struct { MafwSourceClass parent; } FsrcClass;
typedef struct { MafwSource parent; } Fsrc;
G_DEFINE_TYPE(Fsrc, fsrc, MAFW_TYPE_SOURCE);

static void fsrc_class_init(FsrcClass *x)
{
}

static void fsrc_init(Fsrc *y)
{
	mafw_extension_add_property(MAFW_EXTENSION(y), "testp", G_TYPE_DOUBLE);
}


static GType frenderer_get_type(void);
typedef struct { MafwRendererClass parent; } FrendererClass;
typedef struct { MafwRenderer parent; } Frenderer;
G_DEFINE_TYPE(Frenderer, frenderer, MAFW_TYPE_RENDERER);

static void frenderer_class_init(FrendererClass *x)
{
}

static void frenderer_init(Frenderer *y)
{
}

static void browse_cb(MafwSource *self,
					  guint browse_id,
					  gint remaining_count,
					  guint index,
					  const gchar *object_id,
					  GHashTable *metadata,
					  gboolean *user_data,
					  const GError *error)
{
	fail_if(!error);
	fail_if(browse_id != MAFW_SOURCE_INVALID_BROWSE_ID);
	fail_if(*user_data);
	*user_data = TRUE;
}

static void metadata_cb(MafwSource *self,
					   const gchar *object_id,
					   GHashTable *metadata,
					   gboolean *user_data,
					   const GError *error)
{
	fail_if(!error);
	fail_if(*user_data);
	*user_data = TRUE;
}

static void metadata_set_cb(MafwSource *self,
					const gchar *object_id,
					const gchar **failed_keys,
					gboolean *user_data,
					const GError *error)
{
	fail_if(!error);
	fail_if(*user_data);
	*user_data = TRUE;
}

static void ob_created_cb(MafwSource *self,
					  const gchar *object_id,
					  gboolean *user_data,
					  const GError *error)
{
	fail_if(!error);
	fail_if(*user_data);
	*user_data = TRUE;
}

static void ob_destroyed_cb(MafwSource *self,
					    const gchar *object_id,
					    gboolean *user_data,
					    const GError *error)
{
	fail_if(!error);
	fail_if(*user_data);
	*user_data = TRUE;
}

static void propcb(MafwExtension *extension, const gchar *name,
		   GValue *value, gboolean *udata, const GError *error)
{
	fail_if(!error);
	*udata = TRUE;
}

START_TEST(test_source)
{
	MafwSource *src = g_object_new(fsrc_get_type(),
			    "uuid", "srcuuid",
			    NULL);
	GHashTable *mdata = mafw_metadata_new();
	GError *error = NULL;
	gboolean cb_called = FALSE;

	mafw_metadata_add_int(mdata, "testkey", 30);
	fail_if(!src);
	
	fail_if(!mafw_extension_set_property_double(MAFW_EXTENSION(src),
			"testp", 2.34));
	mafw_extension_get_property(MAFW_EXTENSION(src), "testp",
					(gpointer)propcb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	
	fail_if(mafw_source_browse(src, "srcuuid:://", FALSE, NULL,
				"wrongfilter > 0", NULL, 0, 0,
				(gpointer)browse_cb, &cb_called)
					!= MAFW_SOURCE_INVALID_BROWSE_ID);
	fail_if(!cb_called);
	cb_called = FALSE;
	fail_if(mafw_source_browse(src, "srcuuid:://", FALSE, NULL,
				NULL, NULL, 0, 0,
				(gpointer)browse_cb, &cb_called)
					!= MAFW_SOURCE_INVALID_BROWSE_ID);
	fail_if(!cb_called);
	cb_called = FALSE;
	fail_if(mafw_source_cancel_browse(src, 1, &error) != FALSE);
	fail_if(!error);
	g_error_free(error);
	mafw_source_get_metadata(src, "srcuuid://", _mafw_source_all_keys,
				(gpointer)metadata_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_source_set_metadata(src, "srcuuid://", mdata,
					(gpointer)metadata_set_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_source_create_object(src, "srcuuid://", NULL,
					(gpointer)ob_created_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_source_destroy_object(src, "srcuuid://",
					(gpointer)ob_destroyed_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	
	mafw_metadata_release(mdata);
	
	fail_if(!mafw_source_all_keys(MAFW_SOURCE_ALL_KEYS));
	
	

	g_object_unref(src);
}
END_TEST

static void play_cb(MafwRenderer *self, gboolean *user_data,
			const GError *error)
{
	fail_if(!error);
	fail_if(*(gboolean*)user_data);
	*user_data = TRUE;
}

static void stat_cb(MafwRenderer *self, MafwPlaylist *playlist,
			guint index, MafwPlayState state,
				  const gchar *object_id, gboolean *user_data,
				  const GError *error)
{
	fail_if(!error);
	fail_if(*(gboolean*)user_data);
	*user_data = TRUE;
}

static void pos_cb(MafwRenderer *self, gint position,
			gboolean *user_data, const GError *error)
{
	fail_if(!error);
	fail_if(*(gboolean*)user_data);
	*user_data = TRUE;
}

static void buffering_info_cb(MafwRenderer *self, gfloat status,
				gboolean *user_data)
{
	fail_if(status != 3.3f);
	fail_if(*(gboolean*)user_data);
	*user_data = TRUE;
}

static void renderer_mdata_chd_cb(MafwRenderer *self, gchar *name,
					GValueArray   *value,
					gboolean *user_data)
{
	fail_if(*(gboolean*)user_data);
	*(gboolean*)user_data = TRUE;
}

START_TEST(test_renderer)
{
	MafwRenderer *renderer = g_object_new(frenderer_get_type(),
			    "uuid", "srcuuid",
			    NULL);
	GError *error = NULL;
	gboolean cb_called = FALSE;
	
	g_signal_connect(renderer, "buffering-info",
			(GCallback)buffering_info_cb, &cb_called);
	g_signal_connect(renderer, "metadata-changed",
			(GCallback)renderer_mdata_chd_cb, &cb_called);
	
	mafw_renderer_play(renderer, (gpointer)play_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_play_object(renderer, NULL, (gpointer)play_cb,
					&cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_play_uri(renderer, "", (gpointer)play_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_stop(renderer, (gpointer)play_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_pause(renderer, (gpointer)play_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_resume(renderer, (gpointer)play_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_next(renderer, (gpointer)play_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_previous(renderer, (gpointer)play_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_goto_index(renderer, 3, (gpointer)play_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	
	mafw_renderer_get_status(renderer, (gpointer)stat_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_assign_playlist(renderer, NULL, &error);
	fail_if(!error);
	g_error_free(error);
	
	mafw_renderer_get_position(renderer, (gpointer)pos_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_set_position(renderer, SeekAbsolute, 3,
					(gpointer)pos_cb, &cb_called);
	fail_if(!cb_called);
	cb_called = FALSE;
	
	/* signalings */
	mafw_renderer_emit_buffering_info(renderer, 3.3);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_ulong(renderer, "ulong", 3);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_long(renderer, "long", 3);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_int(renderer, "int", 3);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_uint(renderer, "uint", 3);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_uint64(renderer, "int64", 3);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_int64(renderer, "int64", 3);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_string(renderer, "string", "str");
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_double(renderer, "double", 3.3);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_boolean(renderer, "bool", TRUE);
	fail_if(!cb_called);
	cb_called = FALSE;
	mafw_renderer_emit_metadata_boolean(renderer, "bool");
	fail_if(cb_called);
	
	g_object_unref(renderer);
}
END_TEST


int main(void)
{
	TCase *tc;
	Suite *suite;

	suite = suite_create("MafwDefaults");
	tc = tcase_create("Defaults");
	suite_add_tcase(suite, tc);

	if (1) tcase_add_test(tc, test_source);
	if (1) tcase_add_test(tc, test_renderer);

	return checkmore_run(srunner_create(suite), FALSE);
}
