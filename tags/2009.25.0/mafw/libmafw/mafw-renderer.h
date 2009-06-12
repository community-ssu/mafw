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

#ifndef __MAFW_RENDERER_H__
#define __MAFW_RENDERER_H__

#include <glib.h>
#include <glib-object.h>

#include <libmafw/mafw-extension.h>
#include <libmafw/mafw-playlist.h>
#include <libmafw/mafw-metadata.h>

G_BEGIN_DECLS

/* GLib conversion macros */

#define MAFW_TYPE_RENDERER \
        (mafw_renderer_get_type())
#define MAFW_RENDERER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), MAFW_TYPE_RENDERER, MafwRenderer))
#define MAFW_IS_RENDERER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj), MAFW_TYPE_RENDERER))
#define MAFW_RENDERER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), MAFW_TYPE_RENDERER, MafwRendererClass))
#define MAFW_IS_RENDERER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), MAFW_TYPE_RENDERER))
#define MAFW_RENDERER_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), MAFW_TYPE_RENDERER, MafwRendererClass))


/**
 * MafwRendererErrorPolicy:
 * @MAFW_RENDERER_ERROR_POLICY_CONTINUE: move to next item in playlist if available.
 * @MAFW_RENDERER_ERROR_POLICY_STOP:     stop playback.
 *
 * Error management policies for renderers. Define how the renderer behaves in
 * the presence of playback errors.
 */
typedef enum _MafwRendererErrorPolicy {
	MAFW_RENDERER_ERROR_POLICY_CONTINUE,
	MAFW_RENDERER_ERROR_POLICY_STOP
} MafwRendererErrorPolicy;

/**
 * MafwPlayState:
 * @Stopped:       playback is stopped.
 * @Playing:       playback is in progress.
 * @Paused:        playback is temporarily paused.
 * @Transitioning: the renderer is changing from a state to another.
 * @_LastMafwPlayState: marker of last element, not used.
 *
 * Possible renderer states.
 */
typedef enum _MafwPlayState {
	Stopped = 0,
	Playing,
	Paused,
	Transitioning,
	_LastMafwPlayState
} MafwPlayState;

/**
 * MafwRendererSeekMode:
 * @SeekAbsolute: seek to an absolute position (negative means to seek backwards
 *            from the end).
 * @SeekRelative: seek relatively to the current position.
 *
 * Possible seeking modes, see mafw_renderer_set_position().
 */
typedef enum {
	SeekAbsolute = 0,
	SeekRelative,
} MafwRendererSeekMode;

typedef struct _MafwRenderer MafwRenderer;
typedef struct _MafwRendererClass MafwRendererClass;

/**
 * MafwRendererPlaybackCB:
 * @self:      A renderer, who has been asked to play/stop/pause/resume
 * @user_data: Optional user data pointer passed when invoking the call
 * @error:     A GError that is set if an error has occurred.
 *
 * This callback is called after mafw_renderer_play(), mafw_renderer_play_uri(),
 * mafw_renderer_play_object(), mafw_renderer_stop(), mafw_renderer_pause(),
 * mafw_renderer_resume(), mafw_renderer_next(),* mafw_renderer_previous() or
 * mafw_renderer_goto_index() has been called. If the  operation failed for some
 * reason, the @error parameter is set. This callback might get called 
 * asynchronously, usually with renderers accessing the network.
 */
typedef void (*MafwRendererPlaybackCB) (MafwRenderer *self, gpointer user_data,
				    const GError *error);

/**
 * MafwRendererStatusCB:
 * @self:      A renderer, whose status has been queried
 * @playlist:  The renderer's assigned playlist, %NULL if none
 * @index:     The playlist index that is being used by the renderer as current item
 * @state:     The renderer's current playing state
 * @object_id: The object ID of the renderer's current media
 * @user_data: Optional user data pointer passed to mafw_renderer_get_status()
 * @error:     A GError that is set if an error has occurred. If not %NULL,
 *             none of the other parameters (except @self) are valid.
 *
 * When mafw_renderer_get_status() is called, the renderer will send its status reply
 * through the given callback function that is of this type. This means that the
 * reply might come asynchronously (i.e. after mafw_renderer_get_status() returns).
 */
typedef void (*MafwRendererStatusCB) (MafwRenderer *self, MafwPlaylist *playlist,
				  guint index, MafwPlayState state,
				  const gchar *object_id, gpointer user_data,
				  const GError *error);

/**
 * MafwRendererPositionCB:
 * @self:      A renderer, whose current position has been queried
 * @position:  The renderer's current playback position in seconds, counting
 *             from the beginning of the current media item or -1 if we could
 *             not retrieve it
 * @user_data: Optional user data pointer passed to mafw_renderer_get_position()
 * @error:     A GError that is set if an error has occurred. If not %NULL,
 *             none of the other parameters (except @self) are valid.
 *
 * When mafw_renderer_get_position() is called, the renderer will send its position
 * reply through the given callback function that is of this type. This means
 * that the reply might come asynchronously, usually with renderers accessing the
 * network.
 */
typedef void (*MafwRendererPositionCB) (MafwRenderer *self, gint position,
				    gpointer user_data, const GError *error);

/**
 * MafwRendererMetadataResultCB:
 * @self:      A renderer, whose current media's metadata has been queried.
 * @object_id: The object ID of the current media or %NULL if there wasn't any
 *             media.
 * @metadata:  Metadata of the current media.
 * @user_data: Optional user data pointer passed to
 *             mafw_renderer_get_current_metadata()
 * @error:     A GError that is set if an error has occurred. If not %NULL, none
 *             of the other parameters (except @self) are valid.
 *
 * When mafw_renderer_get_current_metadata() is called, the renderer will send
 * its metadata reply through the given callback function that is of this type.
 * This means that the reply might come asynchronously.
 */
typedef void (*MafwRendererMetadataResultCB) (MafwRenderer *self,
					      const gchar *object_id,
					      GHashTable *metadata,
					      gpointer user_data,
					      const GError *error);

/* Class definition */
/**
 * MafwRendererClass:
 * @parent: parent structure.
 * @play: virtual method to perform a play.
 * @play_object: virtual method to play an object.
 * @play_uri: virtual method to play an uri.
 * @stop: virtual method to perform a stop.
 * @pause: virtual method to perform a pause.
 * @resume: virtual method to perform a resume.
 * @get_status: virtual method to get the renderer status.
 * @assign_playlist: virtual method to assign a playlist to the renderer.
 * @next: virtual method to perform a next.
 * @previous: virtual method to a previous.
 * @goto_index: virtual method to perform a goto_index.
 * @set_position: virtual method to set the position in the playing
 * stream.
 * @get_position: virtual method to get the position of the playing
 * stream.
 * @get_current_metadata: virtual method to get the metadata of the current
 * media.
 * @emit_buffering_info: virtual method to emit the buffering-info
 * signal
 * @state_changed: virtual method to be run when the state_changed
 * signal is emitted.
 * @media_changed: virtual method to be run when the media_changed
 * signal is emitted.
 * @playlist_changed: virtual method to be run when the
 * playlist_changed signal is emitted.
 * @buffering_info: virtual method to be run when the buffering_info
 * signal is emitted.
 * @metadata_changed: virtual method to be run when the
 * metadata_changed signal is emitted.
 *
 * Abstract Renderer Class structure.
 */
struct _MafwRendererClass {
	MafwExtensionClass parent;

	/* Playback */

	void (*play)(MafwRenderer *self, MafwRendererPlaybackCB callback,
		     gpointer user_data);
	void (*play_object)(MafwRenderer *self, const gchar *object_id,
			    MafwRendererPlaybackCB callback, gpointer user_data);
	void (*play_uri)(MafwRenderer *self, const gchar *uri,
			 MafwRendererPlaybackCB callback, gpointer user_data);
	void (*stop)(MafwRenderer *self, MafwRendererPlaybackCB callback,
		     gpointer user_data);
	void (*pause)(MafwRenderer *self, MafwRendererPlaybackCB callback,
		      gpointer user_data);
	void (*resume)(MafwRenderer *self, MafwRendererPlaybackCB callback,
		       gpointer user_data);

	/* Status */

	void (*get_status)(MafwRenderer *self, MafwRendererStatusCB callback,
			   gpointer user_data);

	/* Playlist */

	gboolean (*assign_playlist)(MafwRenderer *self, MafwPlaylist *playlist,
				GError **error);
	void (*next)(MafwRenderer *self, MafwRendererPlaybackCB callback,
		     gpointer user_data);
	void (*previous)(MafwRenderer *self, MafwRendererPlaybackCB callback,
			 gpointer user_data);
	void (*goto_index)(MafwRenderer *self, guint index,
			   MafwRendererPlaybackCB callback, gpointer user_data);

	/* Position */

	void (*set_position)(MafwRenderer *self, MafwRendererSeekMode mode,
			     gint seconds, MafwRendererPositionCB callback,
			     gpointer user_data);
	void (*get_position)(MafwRenderer *self, MafwRendererPositionCB callback,
			     gpointer user_data);

	/* Metadata */

	void (*get_current_metadata)(MafwRenderer *self,
				     MafwRendererMetadataResultCB cb,
				     gpointer user_data);
	/* Signals emission */

        void (*emit_buffering_info)(MafwRenderer *self, gfloat fraction);

	/* Signals */

	void (*state_changed)(MafwRenderer *self,
			       MafwPlayState state);
	void (*media_changed)(MafwRenderer *self,
			       gint index, const gchar *object_id);
	void (*playlist_changed)(MafwRenderer *self,
			       MafwPlaylist *playlist);
	void (*buffering_info)(MafwRenderer *self, gfloat status);
	void (*metadata_changed)(MafwRenderer *self, const GHashTable *metadata);
};

/* Object definition */

/**
 * MafwRenderer:
 *
 * Object structure.
 */
struct _MafwRenderer {
	MafwExtension parent;
};

/* Object initialization */

GType mafw_renderer_get_type(void);

/* Signals emission */

extern void mafw_renderer_emit_metadata(MafwRenderer *self, const gchar *name,
				    GType type, guint nargs, ...);

/**
 * mafw_renderer_emit_metadata_boolean:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits boolean metadata on renderer.
 */
#define mafw_renderer_emit_metadata_boolean(self, name, ...)		\
	mafw_renderer_emit_metadata(self, name, G_TYPE_BOOLEAN,		\
				_MAFW_NARGS(gboolean, ##__VA_ARGS__),	\
				##__VA_ARGS__)

/**
 * mafw_renderer_emit_metadata_int:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits int metadata on renderer.
 */
#define mafw_renderer_emit_metadata_int(self, name, ...)			\
	mafw_renderer_emit_metadata(self, name, G_TYPE_INT,			\
				_MAFW_NARGS(gint , ##__VA_ARGS__),	\
				##__VA_ARGS__)

/**
 * mafw_renderer_emit_metadata_uint:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits uint metadata on renderer.
 */
#define mafw_renderer_emit_metadata_uint(self, name, ...)			\
	mafw_renderer_emit_metadata(self, name, G_TYPE_UINT,		\
				_MAFW_NARGS(guint , ##__VA_ARGS__),	\
				##__VA_ARGS__)

/**
 * mafw_renderer_emit_metadata_long:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits long metadata on renderer.
 */
#define mafw_renderer_emit_metadata_long(self, name, ...)			\
	mafw_renderer_emit_metadata(self, name, G_TYPE_LONG,		\
				_MAFW_NARGS(glong , ##__VA_ARGS__),	\
				##__VA_ARGS__)

/**
 * mafw_renderer_emit_metadata_ulong:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits ulong metadata on renderer.
 */
#define mafw_renderer_emit_metadata_ulong(self, name, ...)			\
	mafw_renderer_emit_metadata(self, name, G_TYPE_ULONG,		\
				_MAFW_NARGS(gulong , ##__VA_ARGS__),	\
				##__VA_ARGS__)

/**
 * mafw_renderer_emit_metadata_int64:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits int64 metadata on renderer.
 */
#define mafw_renderer_emit_metadata_int64(self, name, ...)			\
	mafw_renderer_emit_metadata(self, name, G_TYPE_INT64,		\
				_MAFW_NARGS(gint64 , ##__VA_ARGS__),	\
				##__VA_ARGS__)

/**
 * mafw_renderer_emit_metadata_uint64:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits uint64 metadata on renderer.
 */
#define mafw_renderer_emit_metadata_uint64(self, name, ...)			\
	mafw_renderer_emit_metadata(self, name, G_TYPE_UINT64,		\
				_MAFW_NARGS(guint64 , ##__VA_ARGS__),	\
				##__VA_ARGS__)

/**
 * mafw_renderer_emit_metadata_double:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits double metadata on renderer.
 */
#define mafw_renderer_emit_metadata_double(self, name, ...)			\
	mafw_renderer_emit_metadata(self, name, G_TYPE_DOUBLE,		\
				_MAFW_NARGS(gdouble , ##__VA_ARGS__),	\
				##__VA_ARGS__)

/**
 * mafw_renderer_emit_metadata_string:
 * @self: a #MafwRenderer instance
 * @name: the metadata key
 * @...: list of values
 * 
 * Emits string (gchar*) metadata on renderer.
 */
#define mafw_renderer_emit_metadata_string(self, name, ...)			\
	mafw_renderer_emit_metadata(self, name, G_TYPE_STRING,		\
				_MAFW_NARGS(gchar *, ##__VA_ARGS__),	\
				##__VA_ARGS__)

void mafw_renderer_emit_buffering_info(MafwRenderer *self, gfloat fraction);

/* Playback */

void mafw_renderer_play(MafwRenderer *self, MafwRendererPlaybackCB callback,
		    gpointer user_data);
void mafw_renderer_play_object(MafwRenderer *self, const gchar *object_id,
			   MafwRendererPlaybackCB callback, gpointer user_data);
void mafw_renderer_play_uri(MafwRenderer *self, const gchar *uri,
			MafwRendererPlaybackCB callback, gpointer user_data);
void mafw_renderer_stop(MafwRenderer *self, MafwRendererPlaybackCB callback,
		    gpointer user_data);
void mafw_renderer_pause(MafwRenderer *self, MafwRendererPlaybackCB callback,
		     gpointer user_data);
void mafw_renderer_resume(MafwRenderer *self, MafwRendererPlaybackCB callback,
		      gpointer user_data);

/* Status */

void mafw_renderer_get_status(MafwRenderer *self, MafwRendererStatusCB callback,
			  gpointer user_data);

/* Playlist operations */

gboolean mafw_renderer_assign_playlist(MafwRenderer *self, MafwPlaylist *playlist,
				    GError **error);
void mafw_renderer_next(MafwRenderer *self, MafwRendererPlaybackCB callback,
		    gpointer user_data);
void mafw_renderer_previous(MafwRenderer *self, MafwRendererPlaybackCB callback,
			gpointer user_data);
void mafw_renderer_goto_index(MafwRenderer *self, guint index,
			  MafwRendererPlaybackCB callback, gpointer user_data);

/* Playback position */

void mafw_renderer_set_position(MafwRenderer *self, MafwRendererSeekMode mode,
			     gint seconds,
			     MafwRendererPositionCB callback, gpointer user_data);
void mafw_renderer_get_position(MafwRenderer *self, MafwRendererPositionCB callback,
			     gpointer user_data);

/* Metadata */

void mafw_renderer_get_current_metadata(MafwRenderer *self,
					MafwRendererMetadataResultCB callback,
					gpointer user_data);

G_END_DECLS
#endif				/* __MAFW_RENDERER_H__ */
/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
