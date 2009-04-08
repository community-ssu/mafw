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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "mafw-renderer.h"
#include "mafw-extension.h"
#include "mafw-marshal.h"
#include "mafw-source.h"
#include "mafw-registry.h"
#include "mafw-errors.h"

/**
 * SECTION:mafwrenderer
 * @short_description: base class for renderers
 *
 * A renderer is a point of playback represented with another object of
 * #MafwRenderer class. Playback is controlled through renderer objects from
 * an application. Renderers are designed so that any of them can play
 * content from any source, and can be thought of as a GStreamer
 * chain, although this might usually not be the actual case.
 *
 * #MafwRenderer is an abstract base class for renderers. It provides various playback-
 * related operations to control playback: mafw_renderer_play(), mafw_renderer_stop(),
 * mafw_renderer_pause() and mafw_renderer_resume().
 *
 * If the renderer has an assigned playlist (mafw_renderer_assign_playlist()), it plays
 * continously according to the playing order specified for the playlist.
 * Skipping items is possible with mafw_renderer_next(), mafw_renderer_previous() and
 * mafw_renderer_goto_index(). Skipping within the currently playing item can be
 * achieved with mafw_renderer_set_position() and the current playback position can
 * be queried with mafw_renderer_get_position().
 *
 * When assigned playlist is modified, confirmation signal(s) from playlist
 * daemon must be waited before proceeding with operations that depend on
 * certain playlist state. E.g. if playlist with single item A is cleared,
 * item B is added to playlist and mafw_renderer_play() is called. It is possible
 * that the play command is handled by renderer before signal about cleared
 * playlist arrives, this again causes renderer to start playing item A and go
 * into stopped state when clear playlist signal arrives. By waiting until
 * clear signal is handled by commanding process, it is guaranteed that renderer
 * starts playing the correct item B.
 *
 * The status of the renderer can be queried with mafw_renderer_get_status(), which
 * should be used only for initial status update. The rest of the renderer's status
 * information is sent with signals.
 */

/*----------------------------------------------------------------------------
  Default playback implementations
  ----------------------------------------------------------------------------*/

static void mafw_renderer_default_play(MafwRenderer *self, MafwRendererPlaybackCB callback,
				   gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, user_data, error);
		g_error_free(error);
	}
}

static void mafw_renderer_default_play_object(MafwRenderer *self, const gchar *objectid,
					  MafwRendererPlaybackCB callback,
					  gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, user_data, error);
		g_error_free(error);
	}
}

static void mafw_renderer_default_play_uri(MafwRenderer *self, const gchar *uri,
				       MafwRendererPlaybackCB callback,
				       gpointer user_data)
{
	gchar *objectid;

	objectid = mafw_source_create_objectid(uri);
	mafw_renderer_play_object(self, objectid, callback, user_data);
	g_free(objectid);
}

static void mafw_renderer_default_stop(MafwRenderer *self, MafwRendererPlaybackCB callback,
				   gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, user_data, error);
		g_error_free(error);
	}
}

static void mafw_renderer_default_pause(MafwRenderer *self, MafwRendererPlaybackCB callback,
				   gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, user_data, error);
		g_error_free(error);
	}
}

static void mafw_renderer_default_resume(MafwRenderer *self,
				     MafwRendererPlaybackCB callback,
				     gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, user_data, error);
		g_error_free(error);
	}
}

/*----------------------------------------------------------------------------
  Default status implementations
  ----------------------------------------------------------------------------*/

static void mafw_renderer_default_get_status(MafwRenderer *self,
					 MafwRendererStatusCB callback,
					 gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, NULL, 0, 0, NULL, user_data, error);
		g_error_free(error);
	}
}

/*----------------------------------------------------------------------------
  Default playlist implementations
  ----------------------------------------------------------------------------*/

static gboolean mafw_renderer_default_assign_playlist(MafwRenderer *self,
						   MafwPlaylist *playlist,
						   GError **error)
{      
	if (error != NULL) 
	{ 
		g_set_error(error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
	}
	
	return FALSE;
}

static void mafw_renderer_default_next(MafwRenderer *self, MafwRendererPlaybackCB callback,
				   gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, user_data, error);
		g_error_free(error);
	}
}

static void mafw_renderer_default_previous(MafwRenderer *self,
				       MafwRendererPlaybackCB callback,
				       gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, user_data, error);
		g_error_free(error);
	}
}

static void mafw_renderer_default_goto_index(MafwRenderer *self, guint index,
					 MafwRendererPlaybackCB callback,
					 gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, user_data, error);
		g_error_free(error);
	}
}

/*----------------------------------------------------------------------------
  Default playback position implementations
  ----------------------------------------------------------------------------*/

static void mafw_renderer_default_set_position(MafwRenderer *self,
					    MafwRendererSeekMode mode,
					    gint seconds,
					    MafwRendererPositionCB callback,
					    gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, seconds, user_data, error);
		g_error_free(error);
	}
}

static void mafw_renderer_default_get_position(MafwRenderer *self,
					   MafwRendererPositionCB callback,
					   gpointer user_data)
{
	if (callback != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		callback(self, 0, user_data, error);
		g_error_free(error);
	}
}

/*----------------------------------------------------------------------------
  Signals
  ----------------------------------------------------------------------------*/

enum {
	PLAYLIST_CHANGED = 0,
	MEDIA_CHANGED,
	PLAYSTATE_CHANGED,
	BUFFERING_INFO,
	METADATA_CHANGED,
	PROPERTY_CHANGED,
	LAST_SIGNAL,
};

static guint mafw_renderer_signals[LAST_SIGNAL];

/*----------------------------------------------------------------------------
  Signals emission
  ----------------------------------------------------------------------------*/

static void
mafw_renderer_default_emit_buffering_info(MafwRenderer *self, gfloat fraction)
{
	g_signal_emit_by_name(self, "buffering-info", fraction);
}


/*----------------------------------------------------------------------------
  GObject init
  ----------------------------------------------------------------------------*/

G_DEFINE_ABSTRACT_TYPE(MafwRenderer, mafw_renderer, MAFW_TYPE_EXTENSION);

static void mafw_renderer_init(MafwRenderer *self)
{
}

static void mafw_renderer_class_init(MafwRendererClass *klass)
{
	/* Playback */

	klass->play        = mafw_renderer_default_play;
	klass->play_object = mafw_renderer_default_play_object;
	klass->play_uri    = mafw_renderer_default_play_uri;
	klass->stop        = mafw_renderer_default_stop;
	klass->pause       = mafw_renderer_default_pause;
	klass->resume      = mafw_renderer_default_resume;

	/* Status */

	klass->get_status = mafw_renderer_default_get_status;

	/* Playlist */

	klass->assign_playlist   = mafw_renderer_default_assign_playlist;
	klass->next              = mafw_renderer_default_next;
	klass->previous          = mafw_renderer_default_previous;
	klass->goto_index        = mafw_renderer_default_goto_index;

	/* Position */

	klass->set_position = mafw_renderer_default_set_position;
	klass->get_position = mafw_renderer_default_get_position;

	/* Signals emission */
	klass->emit_buffering_info = mafw_renderer_default_emit_buffering_info;

	/* Signals */

	/**
	 * MafwRenderer::playlist-changed:
	 * @self: a #MafwRenderer instance.
	 * @playlist: the renderer's current playlist.
	 *
	 * Emitted when the playlist of #MafwRenderer has changed.
	 */
	mafw_renderer_signals[PLAYLIST_CHANGED] =
	    g_signal_new("playlist-changed",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 G_STRUCT_OFFSET(MafwRendererClass, playlist_changed),
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__POINTER,
			 G_TYPE_NONE,
			 1,
			 G_TYPE_OBJECT);
	/**
	 * MafwRenderer::media-changed:
	 * @self: a #MafwRenderer instance.
	 * @index: the renderer's current playlist index or -1 if
	 * there is no playlist assigned or playlist is empty
	 * @object: object ID of the current item. if index == -1,
	 * object = NULL
	 *
	 * Emitted when the currently played media of #MafwRenderer has changed.
	 */
	mafw_renderer_signals[MEDIA_CHANGED] =
	    g_signal_new("media-changed",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 G_STRUCT_OFFSET(MafwRendererClass, media_changed),
			 NULL,
			 NULL,
			 mafw_marshal_VOID__INT_STRING,
			 G_TYPE_NONE,
			 2,
			 G_TYPE_INT, G_TYPE_STRING);
	/**
	 * MafwRenderer::state-changed:
	 * @self: a #MafwRenderer instance.
	 * @state: the renderer's current playback state (#MafwPlayState).
	 *
	 * Emitted when the playback state of #MafwRenderer has changed.
	 */
	mafw_renderer_signals[PLAYSTATE_CHANGED] =
	    g_signal_new("state-changed",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 G_STRUCT_OFFSET(MafwRendererClass, state_changed),
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__INT,
			 G_TYPE_NONE,
			 1,
			 G_TYPE_INT);
	/**
	 * MafwRenderer::buffering-info:
	 * @self: a #MafwRenderer instance.
	 * @status: buffering status as a fraction (0.0 - 1.0).
	 *
	 * Emitted to indicate buffering status as a fraction.
	 */
	mafw_renderer_signals[BUFFERING_INFO] =
	    g_signal_new("buffering-info",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 G_STRUCT_OFFSET(MafwRendererClass, buffering_info),
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__FLOAT,
			 G_TYPE_NONE, 1, G_TYPE_FLOAT);

	/**
         * MafwRenderer::metadata-changed:
	 * @self:  a #MafwRenderer instance.
	 * @name:  metadata that changed.
	 * @value: a #GValueArray representing the new value
         *
	 * Emitted when some metadata of the currently playing item has changed.
	 * If, for example, the duration of a track can
	 * be calculated more accurately when the track
	 * is played, a renderer can inform of this.
	 */
	mafw_renderer_signals[METADATA_CHANGED] =
	    g_signal_new("metadata-changed",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 G_STRUCT_OFFSET(MafwRendererClass, metadata_changed),
			 NULL,
			 NULL,
			 mafw_marshal_VOID__STRING_BOXED,
			 G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_VALUE_ARRAY);
}

/**
 * mafw_renderer_emit_metadata:
 * @self:    a #MafwRenderer instance.
 * @name:    name of the changed metadata key.
 * @type:    #GType of the following value(s).
 * @nargs:   number of values following.
 * @Varargs: metadata values.
 *
 * Emits #MafwRenderer::metadata-changed for @name.  Pass the metadata
 * value(s) (#GValue:s) in the varargs, following their type and
 * count.  This function exists for the convenience of subclasses.
 */
void mafw_renderer_emit_metadata(MafwRenderer *self, const gchar *name,
			     GType type, guint nargs, ...)
{
	guint i;
	va_list args;
	GValueArray *arr;

	if (nargs == 0)
		return;
	arr = g_value_array_new(nargs);
	va_start(args, nargs);
	for (i = 0; i < nargs; ++i) {
		GValue val = {0};

		g_value_init(&val, type);
		switch (type) {
		case G_TYPE_BOOLEAN:
			g_value_set_boolean(&val, va_arg(args, gboolean));
			break;
		case G_TYPE_INT:
			g_value_set_int(&val, va_arg(args, gint));
			break;
		case G_TYPE_UINT:
			g_value_set_uint(&val, va_arg(args, guint));
			break;
		case G_TYPE_INT64:
			g_value_set_int64(&val, va_arg(args, gint64));
			break;
		case G_TYPE_UINT64:
			g_value_set_uint64(&val, va_arg(args, guint64));
			break;
		case G_TYPE_LONG:
			g_value_set_long(&val, va_arg(args, glong));
			break;
		case G_TYPE_ULONG:
			g_value_set_ulong(&val, va_arg(args, gulong));
			break;
		case G_TYPE_DOUBLE:
			g_value_set_double(&val, va_arg(args, gdouble));
			break;
		case G_TYPE_STRING:
			g_value_set_string(&val, va_arg(args, const gchar *));
			break;
		}
		g_value_array_append(arr, &val);
		g_value_unset(&val);
	}
	va_end(args);
	g_signal_emit(self, mafw_renderer_signals[METADATA_CHANGED], 0, name, arr);
	g_value_array_free(arr);
}

/**
 * mafw_renderer_emit_buffering_info:
 * @self: a #MafwRenderer instance
 * @fraction: the percentage of buffering completion
 *
 * Emits the signal of buffering on the given renderer.
 */
void
mafw_renderer_emit_buffering_info(MafwRenderer *self, gfloat fraction)
{
	MAFW_RENDERER_GET_CLASS(self)->emit_buffering_info(self, fraction);
}

/*----------------------------------------------------------------------------
  Public API functions
  ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Playback
  ----------------------------------------------------------------------------*/

/**
 * mafw_renderer_play:
 * @self:      A #MafwRenderer instance.
 * @callback:  Function that is called after the command has been executed.
 * @user_data: Optional user data pointer.
 *
 * Starts playback. If there is no assigned playlist, or the renderer was
 * already playing, this does nothing. In other cases a
 * #MafwRenderer::state-changed signal is sent.
 */
void mafw_renderer_play(MafwRenderer *self, MafwRendererPlaybackCB callback,
		    gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->play(self, callback, user_data);
}

/**
 * mafw_renderer_play_object:
 * @self:      A #MafwRenderer instance.
 * @object_id: An object id to play.
 * @callback:  Function that is called after the command has been executed.
 * @user_data: Optional user data pointer.
 *
 * Starts playing the given object. If there is an assigned playlist,
 * the renderer resumes playing the item it left off after it is finished with
 * @object_id.
 */
void mafw_renderer_play_object(MafwRenderer *self, const gchar *object_id,
			   MafwRendererPlaybackCB callback, gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->play_object(self, object_id, callback,
					       user_data);
}

/**
 * mafw_renderer_play_uri:
 * @self:      A #MafwRenderer instance.
 * @uri:       The URI to play.
 * @callback:  Function that is called after the command has been executed
 * @user_data: Optional user data pointer
 *
 * Starts playing the given URI. If there is an assigned playlist, the renderer
 * resumes playing the item it left off after it is finished with @uri.
 */
void mafw_renderer_play_uri(MafwRenderer *self, const gchar *uri,
			MafwRendererPlaybackCB callback, gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->play_uri(self, uri, callback, user_data);
}

/**
 * mafw_renderer_stop:
 * @self:      A #MafwRenderer instance
 * @callback:  Function that is called after the command has been executed
 * @user_data: Optional user data pointer
 *
 * Stops playback. If the renderer was playing an object_id, or a URI, a later
 * mafw_renderer_play() will resume the assigned playlist (if there is one).
 */
void mafw_renderer_stop(MafwRenderer *self, MafwRendererPlaybackCB callback,
		    gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->stop(self, callback, user_data);
}
	
/**
 * mafw_renderer_pause:
 * @self:      A #MafwRenderer instance
 * @callback:  Function that is called after the command has been executed
 * @user_data: Optional user data pointer
 *
 * Pauses playback. If the renderer was not playing, this does nothing.
 */
void mafw_renderer_pause(MafwRenderer *self, MafwRendererPlaybackCB callback,
		     gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->pause(self, callback, user_data);
}

/**
 * mafw_renderer_resume:
 * @self:      A #MafwRenderer instance
 * @callback:  Function that is called after the command has been executed
 * @user_data: Optional user data pointer
 *
 * Resumes playback. If the renderer was not paused, this does nothing.
 */
void mafw_renderer_resume(MafwRenderer *self, MafwRendererPlaybackCB callback,
		      gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->resume(self, callback, user_data);
}

/*----------------------------------------------------------------------------
  Status
  ----------------------------------------------------------------------------*/

/**
 * mafw_renderer_get_status:
 * @self:      A #MafwRenderer instance.
 * @callback:  The callback function that receives the renderer's status reply
 * @user_data: Optional user data pointer that is passed to the callback
 *
 * Gets the latest known status of the renderer.
 */
void mafw_renderer_get_status(MafwRenderer *self, MafwRendererStatusCB callback,
			  gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->get_status(self, callback, user_data);
}

/*----------------------------------------------------------------------------
  Playlist
  ----------------------------------------------------------------------------*/

/**
 * mafw_renderer_assign_playlist:
 * @self:      A #MafwRenderer instance.
 * @playlist:  The playlist to assign, %NULL to clear the assignment.
 * @error: a #GError to store an error if needed
 *
 * Assigns the given playlist to the renderer for playback. If there was already
 * a playlist assigned and the renderer was playing from it, the renderer immediately
 * stops playing from the old playlist. Then, if the new one is not %NULL, it
 * starts playing the first item from the new playlist.
 *
 * The renderer should ref the @playlist.
 *
 * Returns: %TRUE if the call succeeds, %FALSE in other case
 */
gboolean mafw_renderer_assign_playlist(MafwRenderer *self, MafwPlaylist *playlist, 
				    GError **error)
{
	return MAFW_RENDERER_GET_CLASS(self)->assign_playlist(self, playlist, error);
}

/**
 * mafw_renderer_next:
 * @self:      A #MafwRenderer instance.
 * @callback:  Function that is called after the command has been executed
 * @user_data: Optional user data pointer
 *
 * Skips to the next item in the renderer's playlist. If the current index points
 * to the last item in the playlist, this wraps around to the first item of
 * the playlist, regardless of playlist repeat setting.
 *
 * After playlist modification(s), wait until changes are confirmed by playlist
 * daemon before attempting to command renderer.
 *
 * The given callback function is executed with the error parameter set if this
 * call failed. If the call succeeds, the callback is called with %NULL error.
 */
void mafw_renderer_next(MafwRenderer *self, MafwRendererPlaybackCB callback,
		    gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->next(self, callback, user_data);
}

/**
 * mafw_renderer_previous:
 * @self:      A #MafwRenderer instance.
 * @callback:  Function that is called after the command has been executed
 * @user_data: Optional user data pointer
 *
 * Skips to the previous item in the renderer's playlist. If the current index
 * points to the first item in the playlist, this wraps around to the last
 * item of the playlist, regardless of playlist repeat setting.
 *
 * After playlist modification(s), wait until changes are confirmed by playlist
 * daemon before attempting to command renderer.
 *
 * The given callback function is executed with the error parameter set if this
 * call failed. If the call succeeds, the callback is called with %NULL error.
 */
void mafw_renderer_previous(MafwRenderer *self, MafwRendererPlaybackCB callback,
			gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->previous(self, callback, user_data);
}

/**
 * mafw_renderer_goto_index:
 * @self:      A #MafwRenderer instance.
 * @index:     The absolute playlist index to skip to
 * @callback:  Function that is called after the command has been executed
 * @user_data: Optional user data pointer
 *
 * Skips to the given index in the renderer's playlist. Nothing happens if the
 * given index is invalid (out of bounds).
 *
 * After playlist modification(s), wait until changes are confirmed by playlist
 * daemon before attempting to command renderer.
 *
 * The given callback function is executed with the error parameter set if this
 * call failed. If the call succeeds, the callback is called with %NULL error.
 */
void mafw_renderer_goto_index(MafwRenderer *self, guint index,
			  MafwRendererPlaybackCB callback, gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->goto_index(self, index, callback, user_data);
}

/*----------------------------------------------------------------------------
  Playback position
  ----------------------------------------------------------------------------*/

/**
 * mafw_renderer_set_position:
 * @self:      A #MafwRenderer instance.
 * @mode:      #MafwRendererSeekMode specifying the seek mode.
 * @seconds:   Time in seconds to seek to.
 * @callback:  A #MafwRendererPositionCB callback that receives the new position
 * @user_data: Optional user data pointer passed to the callback.
 *
 * Seeks in the current media; time is given in seconds.  If @mode is
 * #SeekAbsolute, it seeks to the given from the beginning, or if @seconds is
 * negative seeks from the end.  If @mode is #SeekRelative, the operation is
 * performed relatively to the actual position.  If @seconds is beyond the
 * length of the current media, the renderer should just proceed to the next item in
 * the playlist (if any).  Success/fail status is received through the callback
 * that is of type #MafwRendererPositionCB.
 */
void mafw_renderer_set_position(MafwRenderer *self, MafwRendererSeekMode mode,
			     gint seconds, MafwRendererPositionCB callback,
			     gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->set_position(self, mode, seconds, callback,
						 user_data);
}

/**
 * mafw_renderer_get_position:
 * @self:      A #MafwRenderer instance.
 * @callback:  A #MafwRendererPositionCB callback that receives the current position
 * @user_data: Optional user data pointer passed to the callback.
 *
 * Gets the renderer's current playback position (in seconds) from the beginning of
 * the current media item. If the renderer is not in #Playing or #Paused state, the
 * callback returns zero. The position is returned asynchronously through a
 * callback that is of type #MafwRendererPositionCB.
 */
void mafw_renderer_get_position(MafwRenderer *self, MafwRendererPositionCB callback,
			    gpointer user_data)
{
	MAFW_RENDERER_GET_CLASS(self)->get_position(self, callback, user_data);
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
