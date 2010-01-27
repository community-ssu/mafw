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
#include <string.h>
#include <unistd.h>

#include <libmafw/mafw.h>
#include "checkmore.h"
#include "nopsource.h"

/* Test case ideas
 *
 * status_changed
 *
 *
 * 	test_sc_limit_max - not enabled
 * 	preconditions: no db
 * 	status_changed with options (G_MAXUINT64, G_MAXUINT, 3, NULL)
 * 	expected: valid db row
 *
 * 	test_sc_invalid_args - not implemented
 * 	preconditions: db with valid row playlist_id, index and objectid not
 *      NULL
 * 	status_changed with self NULL others valid
 * 	... (repeat for all vars)
 * 	expected: db stays unthouched
 *
 *
 *
 */

/* Globals. */

static MafwRegistry *Reg;
static MafwSource *Source;
static MafwRenderer *Renderer;

/* Renderer mock-up */
/* NOTE: Currently it only tests error signaling 
   with play() method... */
typedef struct {
	MafwRendererClass parent_class;
} MockedRendererClass;

typedef struct {
	MafwRenderer parent;
} MockedRenderer;

GType mocked_renderer_get_type(void);
G_DEFINE_TYPE(MockedRenderer, mocked_renderer, MAFW_TYPE_RENDERER);

/*----------------------------------------------------------------------------
  Playback
  ----------------------------------------------------------------------------*/

static void mocked_renderer_play(MafwRenderer *self, MafwRendererPlaybackCB callback,
			     gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

static void mocked_renderer_play_object(MafwRenderer *self, const gchar* object_id,
				    MafwRendererPlaybackCB callback,
				    gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");
	fail_if(strcmp(object_id, "Hello::Gentlemen"), "Invalid object ID");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

static void mocked_renderer_play_uri(MafwRenderer *self, const gchar* uri,
				 MafwRendererPlaybackCB callback,
				 gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");
	fail_if(strcmp(uri, "http://all.your.base.are.belong.to.us/"),
		"Invalid URI");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

static void mocked_renderer_stop(MafwRenderer *self, MafwRendererPlaybackCB callback,
			     gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

static void mocked_renderer_pause(MafwRenderer *self, MafwRendererPlaybackCB callback,
			      gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

static void mocked_renderer_resume(MafwRenderer *self, MafwRendererPlaybackCB callback,
			       gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

/*----------------------------------------------------------------------------
  Playlist
  ----------------------------------------------------------------------------*/

static gboolean mocked_renderer_assign_playlist(MafwRenderer *self,
					     MafwPlaylist *playlist,
					     GError **error)
{
	fail_if(playlist != GINT_TO_POINTER(0x00BADA55),
		"Invalid playlist pointer");

	if (error)
		g_set_error(error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Mock renderer cannot do anything.... Sorry.");
	return FALSE;
}

static void mocked_renderer_next(MafwRenderer *self, MafwRendererPlaybackCB callback,
			     gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

static void mocked_renderer_previous(MafwRenderer *self, MafwRendererPlaybackCB callback,
				 gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

static void mocked_renderer_goto_index(MafwRenderer *self, guint index,
				   MafwRendererPlaybackCB callback,
				   gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");
	fail_if(index != 42, "Invalid index");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");
	
	callback(self, user_data, error);
	g_error_free(error);
}

/*----------------------------------------------------------------------------
  Position
  ----------------------------------------------------------------------------*/

static void mocked_renderer_set_position(MafwRenderer *self, MafwRendererSeekMode mode,
				     gint seconds, MafwRendererPositionCB callback,
				     gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");

	callback(self, seconds, user_data, error);
	g_error_free(error);
}

static void mocked_renderer_get_position(MafwRenderer *self,
				     MafwRendererPositionCB callback,
				     gpointer user_data)
{
	GError* error = NULL;

	fail_if(callback == NULL, "Invalid callback pointer");

	g_set_error(&error,
		    MAFW_EXTENSION_ERROR,
		    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    "Mock renderer cannot do anything.... Sorry.");

	callback(self, 42, user_data, error);
	g_error_free(error);
}

/*----------------------------------------------------------------------------
  Mocked renderer class init
  ----------------------------------------------------------------------------*/

static void mocked_renderer_class_init(MockedRendererClass *klass)
{
	klass->parent_class.play        = mocked_renderer_play;
	klass->parent_class.play_object = mocked_renderer_play_object;
	klass->parent_class.play_uri    = mocked_renderer_play_uri;
	klass->parent_class.stop        = mocked_renderer_stop;
	klass->parent_class.pause       = mocked_renderer_pause;
	klass->parent_class.resume      = mocked_renderer_resume;

	klass->parent_class.assign_playlist   = mocked_renderer_assign_playlist;
	klass->parent_class.next              = mocked_renderer_next;
	klass->parent_class.previous          = mocked_renderer_previous;
	klass->parent_class.goto_index        = mocked_renderer_goto_index;

	klass->parent_class.set_position = mocked_renderer_set_position;
	klass->parent_class.get_position = mocked_renderer_get_position;
}

static void mocked_renderer_init(MockedRenderer *src)
{
}

/*----------------------------------------------------------------------------
  Test metadata
  ----------------------------------------------------------------------------*/

/* Dummy get_metadata for our dummy source. */
static void dummy_source_get_metadata(MafwSource *self,
					  const gchar *object_id,
					  const gchar *const *mdkeys,
					  MafwSourceMetadataResultCb cb,
					  gpointer user_data)
{
	GHashTable *metadata;

	if(strcmp(object_id, "SOURCE::foo") == 0)
	{
		metadata = mafw_metadata_new();
		mafw_metadata_add_str(metadata, "uri", "boo");
		cb(self, object_id, metadata, user_data, NULL);
		g_hash_table_unref(metadata);
	}
	else
	{
		GError* error = NULL;
		g_set_error(&error, MAFW_RENDERER_ERROR, MAFW_RENDERER_ERROR_NO_MEDIA,
			    "Just nyt ei pysty");
		cb(self, object_id, NULL, user_data, error);
		g_error_free(error);
	}
}

static void get_metadata_cb(MafwSource* source, const gchar *objectid,
			    GHashTable *metadata, gpointer user_data,
			    const GError *error)
{
	/* Validate what we got. */
	fail_if(source != Source);
	fail_if(GPOINTER_TO_UINT(user_data) != 42);

	if (error != NULL)
	{
		fail_if(strcmp(objectid, "SOURCE::bar") != 0);
		fail_if(metadata != NULL);
		fail_if(error->domain != MAFW_RENDERER_ERROR);
		fail_if(error->code != MAFW_RENDERER_ERROR_NO_MEDIA);
		fail_if(strcmp(error->message, "Just nyt ei pysty") != 0);
	}
	else
	{
		GValue* val;
		val = mafw_metadata_first(metadata, "uri");

		fail_if(strcmp(objectid, "SOURCE::foo") != 0);
		fail_if(strcmp(g_value_get_string(val), "boo") != 0);
	}
}

START_TEST(test_get_metadata)
{
	Renderer = g_object_new(mocked_renderer_get_type(), "uuid", "RENDERER", NULL);
	Source = g_object_new(nop_source_get_type(),
			      "uuid", "SOURCE",
			      "name", "nop",
			      NULL);
	MAFW_SOURCE_GET_CLASS(Source)->get_metadata = dummy_source_get_metadata;
	Reg = mafw_registry_get_instance();
	mafw_registry_add_extension(MAFW_REGISTRY(Reg), MAFW_EXTENSION(Renderer));
	mafw_registry_add_extension(MAFW_REGISTRY(Reg), MAFW_EXTENSION(Source));

	/* Try to get metadata for an "existing" object ID */
	mafw_source_get_metadata(Source, "SOURCE::foo",
				 MAFW_SOURCE_LIST("uri"),
				 get_metadata_cb,
				 GUINT_TO_POINTER(42));

	/* Try to get metadata for an "invalid" object ID */
	mafw_source_get_metadata(Source, "SOURCE::bar",
				 MAFW_SOURCE_LIST("uri"),
				 get_metadata_cb,
				 GUINT_TO_POINTER(42));

	g_object_unref(G_OBJECT(Reg));
}
END_TEST

/*----------------------------------------------------------------------------
  Play
  ----------------------------------------------------------------------------*/

static void play_cb(MafwRenderer *self, gpointer user_data, const GError *error)
{
	fail_if(error == NULL, "Play should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0xACDCABBA), "Wrong user data");
}

START_TEST(test_play)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	
	mafw_renderer_play(MAFW_RENDERER(renderer), play_cb, GINT_TO_POINTER(0xACDCABBA));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Play Object
  ----------------------------------------------------------------------------*/

static void play_object_cb(MafwRenderer *self, gpointer user_data,
			   const GError *error)
{
	fail_if(error == NULL, "Play object should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x00BADA55), "Wrong user data");
}

START_TEST(test_play_object)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	
	mafw_renderer_play_object(MAFW_RENDERER(renderer),
			      "Hello::Gentlemen",
			      play_object_cb,
			      GINT_TO_POINTER(0x00BADA55));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Play URI
  ----------------------------------------------------------------------------*/

static void play_uri_cb(MafwRenderer *self, gpointer user_data, const GError *error)
{
	fail_if(error == NULL, "Play URI should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x00BADA55), "Wrong user data");
}

START_TEST(test_play_uri)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	
	mafw_renderer_play_uri(MAFW_RENDERER(renderer),
			   "http://all.your.base.are.belong.to.us/",
			   play_uri_cb,
			   GINT_TO_POINTER(0x00BADA55));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Stop
  ----------------------------------------------------------------------------*/

static void stop_cb(MafwRenderer *self, gpointer user_data, const GError *error)
{
	fail_if(error == NULL, "Stop should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0xFEEDBABE), "Wrong user data");
}

START_TEST(test_stop)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	
	mafw_renderer_stop(MAFW_RENDERER(renderer), stop_cb, GINT_TO_POINTER(0xFEEDBABE));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Pause
  ----------------------------------------------------------------------------*/

static void pause_cb(MafwRenderer *self, gpointer user_data, const GError *error)
{
	fail_if(error == NULL, "Pause should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x5AFEF00D), "Wrong user data");
}

START_TEST(test_pause)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	
	mafw_renderer_pause(MAFW_RENDERER(renderer), pause_cb, GINT_TO_POINTER(0x5AFEF00D));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Resume
  ----------------------------------------------------------------------------*/

static void resume_cb(MafwRenderer *self, gpointer user_data, const GError *error)
{
	fail_if(error == NULL, "Resume should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x00BADA55), "Wrong user data");
}

START_TEST(test_resume)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	
	mafw_renderer_resume(MAFW_RENDERER(renderer), resume_cb,
			 GINT_TO_POINTER(0x00BADA55));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Assign playlist
  ----------------------------------------------------------------------------*/

START_TEST(test_assign_playlist)
{
	MockedRenderer *renderer = NULL;
	GError *error = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	
	mafw_renderer_assign_playlist(MAFW_RENDERER(renderer),
				   GINT_TO_POINTER(0x00BADA55),
				   &error);

	fail_if(error == NULL, "Assign playlist should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");

	g_error_free(error);
	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Next
  ----------------------------------------------------------------------------*/

static void next_cb(MafwRenderer *self, gpointer user_data, const GError *error)
{
	fail_if(error == NULL, "Assign playlist should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x00BADA55), "Wrong user data");
}

START_TEST(test_next)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	
	mafw_renderer_next(MAFW_RENDERER(renderer), next_cb, GINT_TO_POINTER(0x00BADA55));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Previous
  ----------------------------------------------------------------------------*/

static void previous_cb(MafwRenderer *self, gpointer user_data, const GError *error)
{
	fail_if(error == NULL, "Assign playlist should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x00BADA55), "Wrong user data");
}

START_TEST(test_previous)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);

	mafw_renderer_previous(MAFW_RENDERER(renderer), previous_cb,
			   GINT_TO_POINTER(0x00BADA55));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Goto index
  ----------------------------------------------------------------------------*/

static void goto_index_cb(MafwRenderer *self, gpointer user_data,
			  const GError *error)
{
	fail_if(error == NULL, "Assign playlist should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x00BADA55), "Wrong user data");
}

START_TEST(test_goto_index)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);

	mafw_renderer_goto_index(MAFW_RENDERER(renderer), 42, goto_index_cb,
			     GINT_TO_POINTER(0x00BADA55));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Set position
  ----------------------------------------------------------------------------*/

static void set_position_cb(MafwRenderer *self, gint position, gpointer user_data,
			    const GError *error)
{
	fail_if(position != 42, "Wrong position");
	fail_if(error == NULL, "Assign playlist should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x00BADA55), "Wrong user data");
}

START_TEST(test_set_position)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);

	mafw_renderer_set_position(MAFW_RENDERER(renderer), SeekAbsolute, 42,
				set_position_cb, GINT_TO_POINTER(0x00BADA55));

	g_object_unref(renderer);
}
END_TEST

/*----------------------------------------------------------------------------
  Get position
  ----------------------------------------------------------------------------*/

static void get_position_cb(MafwRenderer *self, gint position, gpointer user_data,
			    const GError *error)
{
	fail_if(position != 42, "Wrong position");
	fail_if(error == NULL, "Assign playlist should have failed");
	fail_if(error->domain != MAFW_EXTENSION_ERROR, "Wrong domain");
	fail_if(error->code != MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		"Wrong code");
	fail_if(error->message == NULL, "Message missing");
	fail_if(user_data != GINT_TO_POINTER(0x00BADA55), "Wrong user data");
}

START_TEST(test_get_position)
{
	MockedRenderer *renderer = NULL;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);

	mafw_renderer_get_position(MAFW_RENDERER(renderer), get_position_cb,
			       GINT_TO_POINTER(0x00BADA55));

	g_object_unref(renderer);
}
END_TEST

static void gotmeta(MafwRenderer *renderer,
		    const gchar *name, GValueArray *varr,
		    gboolean *isok)
{
	static guint callcount = 0;

	switch (callcount++) {
	case 0:
		*isok = !strcmp(name, "bitrate")
			&& varr->n_values == 1
			&& g_value_get_int(g_value_array_get_nth(varr, 0)) == 123;
		break;
	case 1:
		*isok = !strcmp(name, "date")
			&& varr->n_values == 3
			&& g_value_get_int(g_value_array_get_nth(varr, 0)) == 2008
			&& g_value_get_int(g_value_array_get_nth(varr, 1)) == 05
			&& g_value_get_int(g_value_array_get_nth(varr, 2)) == 19;
		break;
	case 2:
		*isok = !strcmp(name, "tags")
			&& varr->n_values == 3
			&& !strcmp("epic", g_value_get_string(g_value_array_get_nth(varr, 0)))
			&& !strcmp("fail", g_value_get_string(g_value_array_get_nth(varr, 1)))
			&& !strcmp("aye", g_value_get_string(g_value_array_get_nth(varr, 2)));
		break;
	default:
		fail("invoked too many times");
		break;
	}
}

START_TEST(test_metadata_changed)
{
	MafwRenderer *renderer;
	gboolean isok;

	renderer = g_object_new(mocked_renderer_get_type(),
			    "uuid", "mockedsnk",
			    "name", "Mocked renderer", NULL);
	g_signal_connect(renderer, "metadata-changed", G_CALLBACK(gotmeta), &isok);

	isok = FALSE;
	mafw_renderer_emit_metadata_int(renderer, "bitrate", 123);
	fail_unless(isok);

	isok = FALSE;
	mafw_renderer_emit_metadata_int(renderer, "date", 2008, 05, 19);
	fail_unless(isok);

	isok = FALSE;
	mafw_renderer_emit_metadata_string(renderer, "tags", "epic", "fail", "aye");
	fail_unless(isok);
}
END_TEST

/*----------------------------------------------------------------------------
  Main
  ----------------------------------------------------------------------------*/

int main(void)
{
	TCase *tc;
	Suite *suite;

	suite = suite_create("MafwRenderer");
	tc = tcase_create("Metadata");
	tcase_add_test(tc, test_get_metadata);
	tcase_add_test(tc, test_metadata_changed);
	suite_add_tcase(suite, tc);

	tc = tcase_create("Playback");
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, test_play);
	tcase_add_test(tc, test_play_object);
	tcase_add_test(tc, test_play_uri);
	tcase_add_test(tc, test_stop);
	tcase_add_test(tc, test_pause);
	tcase_add_test(tc, test_resume);

	tc = tcase_create("Playlist");
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, test_assign_playlist);
	tcase_add_test(tc, test_next);
	tcase_add_test(tc, test_previous);
	tcase_add_test(tc, test_goto_index);

	tc = tcase_create("Position");
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, test_set_position);
	tcase_add_test(tc, test_get_position);

	return checkmore_run(srunner_create(suite), FALSE);
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
