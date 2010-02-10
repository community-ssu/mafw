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

#ifndef _MAFW_PLAYLIST_H
#define _MAFW_PLAYLIST_H

#include <glib-object.h>

/*----------------------------------------------------------------------------
  GObject type conversion macros
  ----------------------------------------------------------------------------*/

#define MAFW_TYPE_PLAYLIST \
	(mafw_playlist_get_type())
#define MAFW_PLAYLIST(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), MAFW_TYPE_PLAYLIST, MafwPlaylist))
#define MAFW_IS_PLAYLIST(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), MAFW_TYPE_PLAYLIST))
#define MAFW_PLAYLIST_GET_IFACE(obj) \
	(G_TYPE_INSTANCE_GET_INTERFACE((obj), MAFW_TYPE_PLAYLIST, \
				       MafwPlaylistIface))

/*----------------------------------------------------------------------------
  Object type definitions
  ----------------------------------------------------------------------------*/

/**
 * MafwPlaylist:
 *
 * Instance of object implementing #MafwPlaylistIface.
 */
typedef struct _MafwPlaylist MafwPlaylist; /* dummy structure for gtk-doc hack */



/**
 * MafwPlaylistIface:
 * @parent: parent structure.
 * @shuffle: virtual function to shuffle playing order.
 * @unshuffle: virtual function to restore playing order.
 * @increment_use_count: virtual function to increment the use
 * count
 * @decrement_use_count: virtual function to decrement the use
 * count
 * @insert_item: virtual function to insert an intem in the playlist
 * @append_item: virtual function to append an item to the playlist
 * @remove_item: virtual function to remove an item from the playlist
 * @move_item: virtual function to move an item from one position to
 * other in the playlist
 * @clear: virtual function to clear the playlist
 * @get_item: virtual function to get the item in a playlist's
 * position
 * @get_size: virtual function to get the playlist's size
 *
 * Playlist interface.
 */
typedef struct {
        GTypeInterface parent;

	gboolean (*shuffle)(MafwPlaylist *playlist, GError **error);
	gboolean (*unshuffle)(MafwPlaylist *playlist, GError **error);
	gboolean (*increment_use_count)(MafwPlaylist *playlist,
					GError **error);
	gboolean (*decrement_use_count)(MafwPlaylist *playlist,
					GError **error);
	gboolean (*insert_item)(MafwPlaylist *playlist, guint index,
				const gchar *objectid, GError **error);
	gboolean (*append_item)(MafwPlaylist *playlist, const gchar *oid,
				GError **error);
	gboolean (*insert_items)(MafwPlaylist *playlist, guint index,
				const gchar **objectid, GError **error);
	gboolean (*append_items)(MafwPlaylist *playlist, const gchar **oid,
				GError **error);
	gboolean (*remove_item)(MafwPlaylist *playlist, guint index,
				GError **error);
	gboolean (*move_item)(MafwPlaylist *playlist, guint from, guint to,
			      GError **error);
	gboolean (*clear)(MafwPlaylist *playlist, GError **error);
	gchar *(*get_item)(MafwPlaylist *playlist, guint index, GError **error);
	gchar **(*get_items)(MafwPlaylist *playlist, guint first_index,
				guint last_index, GError **error);
	guint (*get_size)(MafwPlaylist *playlist, GError **error);
	void (*get_starting_index)(MafwPlaylist *playlist, guint *index,
				gchar **object_id, GError **error);
	void (*get_last_index)(MafwPlaylist *playlist, guint *index,
				gchar **object_id, GError **error);
	gboolean (*get_next)(MafwPlaylist *playlist, guint *index,
				gchar **object_id, GError **error);
	gboolean (*get_prev)(MafwPlaylist *playlist, guint *index,
        			gchar **object_id, GError **error);
} MafwPlaylistIface;

G_BEGIN_DECLS

/*----------------------------------------------------------------------------
  Object type definitions
  ----------------------------------------------------------------------------*/

GType mafw_playlist_get_type(void);

/* get/set */

void mafw_playlist_set_name(MafwPlaylist *pls, const gchar *name);
gchar *mafw_playlist_get_name(MafwPlaylist *pls);
void mafw_playlist_set_repeat(MafwPlaylist *pls, gboolean repeat);
gboolean mafw_playlist_get_repeat(MafwPlaylist *pls);
gboolean mafw_playlist_is_shuffled(MafwPlaylist *pls);

/*----------------------------------------------------------------------------
  Shuffle
  ----------------------------------------------------------------------------*/

/* Shuffle (randomize) a playlist's playing order */
gboolean mafw_playlist_shuffle(MafwPlaylist *playlist, GError **error);

/* Restore a shuffled playlist's original playing order */
gboolean mafw_playlist_unshuffle(MafwPlaylist *playlist, GError **error);

/*----------------------------------------------------------------------------
  Increments/decrements use count
  ----------------------------------------------------------------------------*/

/* Increments the use count of the playlist */
gboolean mafw_playlist_increment_use_count(MafwPlaylist *playlist,
					    GError **error);

/* Decrements the use count of the playlist */
gboolean mafw_playlist_decrement_use_count(MafwPlaylist *playlist,
					    GError **error);
/*----------------------------------------------------------------------------
  Contents manipulation
  ----------------------------------------------------------------------------*/

/* Insert an item into a playlist */
gboolean mafw_playlist_insert_item(MafwPlaylist *playlist, guint index,
			       const gchar *objectid, GError **error);
gboolean mafw_playlist_insert_items(MafwPlaylist *playlist, guint index,
			       const gchar **objectid, GError **error);

/* Insert an item pointing to @uri to the @playlist. */
gboolean mafw_playlist_insert_uri(MafwPlaylist *playlist, guint index,
				  const gchar *uri, GError **error);

gboolean mafw_playlist_append_item(MafwPlaylist *playlist, const gchar *oid,
				    GError **error);
gboolean mafw_playlist_append_items(MafwPlaylist *playlist, const gchar **oid,
				    GError **error);
gboolean mafw_playlist_append_uri(MafwPlaylist *playlist, const gchar *uri,
				   GError **error);

/* Remove an item from a playlist */
gboolean mafw_playlist_remove_item(MafwPlaylist *playlist, guint index,
				   GError **error);

/* Clear the contents of a playlist */
gboolean mafw_playlist_clear(MafwPlaylist *playlist, GError **error);

/* Move an item in a playlist to another position */
gboolean mafw_playlist_move_item(MafwPlaylist *playlist, guint from, guint to,
				 GError **error);

/* Get an item from a playlist */
gchar *mafw_playlist_get_item(MafwPlaylist *playlist, guint index,
			      GError **error);
/* Get an item from a playlist */
gchar **mafw_playlist_get_items(MafwPlaylist *playlist,
			      guint first_index, guint last_index,
			      GError **error);

/* Returns the objectid and the visual index of the first playable item */
void mafw_playlist_get_starting_index(MafwPlaylist *playlist, guint *index,
        				gchar **object_id, GError **error);

/* Returns the objectid and the visual index of the last playable item */
void mafw_playlist_get_last_index(MafwPlaylist *playlist, guint *index,
        				gchar **object_id, GError **error);

/* Returns object id for next item and updates visual index */
gboolean mafw_playlist_get_next(MafwPlaylist *playlist, guint *index,
        			gchar **object_id, GError **error);

/* Returns object id for previous item and updates visual index */
gboolean mafw_playlist_get_prev(MafwPlaylist *playlist, guint *index,
        			gchar **object_id, GError **error);

/* Get the size of a playlist */
guint mafw_playlist_get_size(MafwPlaylist *playlist, GError **error);

/**
 * MafwPlaylistGetItemsCB:
 * @pls:   the #MafwPlaylist instance.
 * @index: item index.
 * @object_id: object_id of the item.
 * @metadata: the requested metadata.
 * @cbarg: user-specified additional argument.
 *
 * Callback prototype for mafw_playlist_get_items_md().
 */
typedef void (*MafwPlaylistGetItemsCB)(MafwPlaylist *pls,
				       guint index,
				       const gchar *object_id,
				       GHashTable *metadata,
				       gpointer cbarg);

gpointer mafw_playlist_get_items_md(MafwPlaylist *pls,
				    guint from, gint to,
				    const gchar *const keys[],
				    MafwPlaylistGetItemsCB cb, gpointer cbarg,
				    GDestroyNotify free_cbarg);
void mafw_playlist_cancel_get_items_md(gconstpointer op);

G_END_DECLS

#endif

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
