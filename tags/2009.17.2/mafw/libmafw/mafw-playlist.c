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

#include "mafw-errors.h"
#include "mafw-playlist.h"
#include "mafw-source.h"
#include "mafw-marshal.h"
#include "mafw-registry.h"

/**
 * SECTION: mafwplaylist
 * @short_description: Playlists in Mafw
 *
 * Implementations of the #MafwPlaylist interface allow the UI
 * developer to store a list of Object IDs, and make a #MafwRenderer play
 * from it continuously using mafw_renderer_assign_playlist().
 *
 * Basic operations are: inserting an item
 * (mafw_playlist_insert_item()), removing an item
 * (mafw_playlist_remove_item()), moving an item
 * (mafw_playlist_move_item()).
 *
 * A #MafwPlaylist can also be shuffled (mafw_playlist_shuffle()),
 * which operation randomizes the `playing order' (that renderers use to
 * traverse the playlist).  mafw_playlist_get_items_md() provides a convenient
 * way for retrieving multiple items and their metadata asynchronously.
 *
 * Currently the only existing playlist implementation is #MafwProxyPlaylist (in
 * the libmafw-shared library).
 */

/*----------------------------------------------------------------------------
  Object type definition
  ----------------------------------------------------------------------------*/

#ifndef G_PARAM_STATIC_STRINGS
#define G_PARAM_STATIC_STRINGS \
	(G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB)
#endif

static void mafw_playlist_base_init(MafwPlaylistIface *iface)
{
	static gboolean initialized = FALSE;

	if (G_UNLIKELY(!initialized)) {
		GType itype;

		initialized = TRUE;
		itype = G_TYPE_FROM_INTERFACE(iface);
		/**
		 * MafwPlaylist:name
		 *
		 * The name of the playlist.
		 */
		g_object_interface_install_property(
			iface,
			g_param_spec_string("name", "Name",
					    "Name of the playlist", "",
					    G_PARAM_READWRITE |
					    G_PARAM_STATIC_STRINGS));
		/**
		 * MafwPlaylist:repeat
		 *
		 * If %TRUE, the playlist wraps around when playing
		 * continuously.
		 */
		g_object_interface_install_property(
			iface,
			g_param_spec_boolean("repeat", "Repeat",
					     "Repeat on/off", FALSE,
					     G_PARAM_READWRITE |
					     G_PARAM_STATIC_STRINGS));
		/**
		 * MafwPlaylist:is-shuffled
		 *
		 * If %TRUE then the playlist's playing order is
		 * different from it's visual order.
		 */
		g_object_interface_install_property(
			iface,
			g_param_spec_boolean("is-shuffled", "Shuffled?",
					     "Playlist is shuffled", FALSE,
					     G_PARAM_READABLE |
					     G_PARAM_STATIC_STRINGS));
		/**
		 * MafwPlaylist::contents-changed:
		 * @playlist: The signaling #MafwPlaylist object.
		 * @from:     The index of the first element in the range
		 *            of modified items.
		 * @nremove:  Number of items to remove, starting at @from.
		 * @nreplace: Number of items to to get (in place of removed
		 *            items), starting at @from.
		 *
		 * The ::contents_changed signal on #MafwPlaylist is emitted
		 * when the playlist contents are modified. This can happen
		 * during insert, remove, and clear. The
		 * parametes define the first item affected by the change,
		 * number of items to remove and number of items to get in
		 * place of the removed items.
		 *
		 * Examples:
		 *
		 * - One item was added to index 5 to a playlist of 10 items:
		 *   from = 5, nremove = 0, nreplace = 1.
		 *
		 * - Two items were removed from indices 3 & 4 from a playlist
		 *   of 10 items: from = 3, nremove = 2, nreplace = 5.
		 *
		 * - A playlist of 10 items was cleared:
		 *   from = 0, nremove = 10, nreplace = 0.
		 *
		 *
		 * Callback prototype example:
		 *
		 * static void mafw_playlist_changed(MafwPlaylist *playlist,
		 *                                   guint from,
		 *                                   guint nremove,
		 *                                   guint nreplace);
		 */
		g_signal_new("contents-changed",
			     itype,
			     G_SIGNAL_RUN_FIRST,
			     0, NULL, NULL,
			     mafw_marshal_VOID__UINT_UINT_UINT,
			     G_TYPE_NONE,
			     3,
			     G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);

		/**
		 * MafwPlaylist::item-moved:
		 * @playlist: The signaling #MafwPlaylist object.
		 * @from:     The old index of the item
		 * @to:       The new index of the item
		 *
		 * The ::item-moved signal on #MafwPlaylist is emitted
		 * when an item of the playlist has been moved to a new place.
		 *
		 * Callback prototype example:
		 *
		 * static void mafw_playlist_item_moved(MafwPlaylist *playlist,
		 *                                   guint from,
		 *                                   guint to);
		 */
		g_signal_new("item-moved",
			     itype,
			     G_SIGNAL_RUN_FIRST,
			     0, NULL, NULL,
			     mafw_marshal_VOID__UINT_UINT,
			     G_TYPE_NONE,
			     2,
			     G_TYPE_UINT, G_TYPE_UINT);
	}
}

GType mafw_playlist_get_type(void)
{
	static GType type = 0;
	if (G_UNLIKELY(!type)) {
		static const GTypeInfo info = {
			.class_size = sizeof(MafwPlaylistIface),
			.base_init = (gpointer)mafw_playlist_base_init,
		};
		type = g_type_register_static(G_TYPE_INTERFACE,
					      "MafwPlaylist",
					      &info, 0);
	}
	return type;
}

/*----------------------------------------------------------------------------
  Shuffle
  ----------------------------------------------------------------------------*/

/**
 * mafw_playlist_shuffle:
 * @playlist: a #MafwPlaylist instance.
 * @error: return location for a GError, or NULL
 *
 * Shuffles the playlist.  Shuffling means randomizing the playlist's
 * playing order which renderers use.
 *
 * Returns: %TRUE if the operation was successful. In case of error, details
 * are set in the error argument.
 **/
gboolean mafw_playlist_shuffle(MafwPlaylist *playlist, GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->shuffle(playlist, error);
}

/**
 * mafw_playlist_unshuffle:
 * @playlist: a #MafwPlaylist instance.
 * @error: return location for a GError, or NULL
 *
 * Restores the original playing order of a playlist.
 *
 * Returns: %TRUE if the operation was successful. In case of error, details
 * are set in the error argument.
 **/
gboolean mafw_playlist_unshuffle(MafwPlaylist *playlist, GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->unshuffle(playlist, error);
}


/*----------------------------------------------------------------------------
  Increment/decrement use count
  ----------------------------------------------------------------------------*/

/**
 * mafw_playlist_increment_use_count:
 * @playlist: a #MafwPlaylist instance.
 * @error: a #GError to store an error if somethings goes wrong.
 *
 * Increments the use count of the playlist. The counter must be incremented
 * when the playlist is assigned to some renderer. 
 * 
 * Returns: %TRUE if the operation was successful. In case of error, details
 * are set in the error argument.
 */
gboolean mafw_playlist_increment_use_count(MafwPlaylist *playlist,
					    GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->increment_use_count(playlist,
								       error);
}

/**
 * mafw_playlist_decrement_use_count:
 * @playlist: a #MafwPlaylist instance.
 * @error: a #GError to store an error if somethings goes wrong.
 *
 * Decrements the use count of the playlist. The counter must be decremented
 * when the playlist is unassigned to some renderer.
 *
 * Returns: %TRUE if the operation was successful. In case of error, details
 * are set in the error argument.
 */
gboolean mafw_playlist_decrement_use_count(MafwPlaylist *playlist,
					    GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->decrement_use_count(playlist,
								       error);
}

/*----------------------------------------------------------------------------
  Contents manipulation
  ----------------------------------------------------------------------------*/

/**
 * mafw_playlist_insert_item:
 * @playlist: a #MafwPlaylist instance.
 * @index:    the position to insert the item at.  Valid value range
 *            is between 0 (insert before all existing items) and
 *            playlist size (append).
 * @objectid: the ID of the item to insert into the playlist.
 * @error: return location for a GError, or NULL
 *
 * Inserts an item at the given position in the playlist.  The @index
 * parameter should be an absolute visual index (i.e. not in playing
 * order).  If @objectid is appended to the list it will be played last.
 * Otherwise it will inherit the playing position of the @index:th item,
 * and all subsequent items are moved downwards.
 *
 * Returns: %TRUE if the operation was successful. In case of error, details
 * are set in the error argument.
 */
gboolean mafw_playlist_insert_item(MafwPlaylist *playlist, guint index,
				   const gchar *objectid, GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	g_return_val_if_fail(objectid != NULL, FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->insert_item(playlist, index,
							      objectid,
							      error);
}

/**
 * mafw_playlist_insert_items:
 * @playlist: a #MafwPlaylist instance.
 * @index:    the position to insert the items at.  Valid value range
 *            is between 0 (insert before all existing items) and
 *            playlist size (append).
 * @objectid: NULL terminated array of object-ids to insert into the playlist.
 * @error: return location for a GError, or NULL
 *
 * Inserts items at the given position in the playlist.  The @index
 * parameter should be an absolute visual index (i.e. not in playing
 * order).
 *
 * Returns: %TRUE if the operation was successful. In case of error, details
 * are set in the error argument.
 */
gboolean mafw_playlist_insert_items(MafwPlaylist *playlist, guint index,
				   const gchar **objectid, GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	g_return_val_if_fail(objectid != NULL, FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->insert_items(playlist, index,
							      objectid,
							      error);
}


/**
 * mafw_playlist_insert_uri:
 * @playlist: playlist
 * @index: index
 * @uri: uri
 * @error: an error
 *
 * Insert an uri in the playlist.
 *
 * Returns: %TRUE if ok, %FALSE otherwise
 */
gboolean mafw_playlist_insert_uri(MafwPlaylist *playlist, guint index,
				  const gchar *uri, GError **error)
{
	gboolean isok;
	gchar *objectid;

	g_return_val_if_fail(uri != NULL, FALSE);
	objectid = mafw_source_create_objectid(uri);
	isok = mafw_playlist_insert_item(playlist, index, objectid, error);
	g_free(objectid);
	return isok;
}

/**
 * mafw_playlist_append_items:
 * @playlist: a #MafwPlaylist instance.
 * @oid:      NULL terminated array of object-ids to insert into the playlist.
 * @error:    return location for a #GError, or %NULL.
 *
 * Appends @objectids to the playlist.
 *
 * Returns: %TRUE if the operation was successful.  In case of error, details
 * are set in the error argument.
 */
gboolean mafw_playlist_append_items(MafwPlaylist *playlist, const gchar **oid,
				    GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	g_return_val_if_fail(oid != NULL, FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->append_items(playlist,
							       oid,
							       error);
}

/**
 * mafw_playlist_append_item:
 * @playlist: a #MafwPlaylist instance.
 * @oid:      the ID of the item to insert into the playlist.
 * @error:    return location for a #GError, or %NULL.
 *
 * Appends @objectid to the playlist.  The item will be played last.
 *
 * Returns: %TRUE if the operation was successful.  In case of error, details
 * are set in the error argument.
 */
gboolean mafw_playlist_append_item(MafwPlaylist *playlist, const gchar *oid,
				    GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	g_return_val_if_fail(oid != NULL, FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->append_item(playlist,
							       oid,
							       error);
}

/**
 * mafw_playlist_append_uri:
 * @playlist: a #MafwPlaylist instance.
 * @uri:      an uri.
 * @error:    location for a #GError or %NULL.
 *
 * Appends the given @uri to the playlist (using mafw_source_create_objectid()
 * to transform it to an object id).
 *
 * Returns: %TRUE if the operation was successful.  Otherwise, @error contains
 * details.
 */
gboolean mafw_playlist_append_uri(MafwPlaylist *playlist, const gchar *uri,
				   GError **error)
{
	gboolean isok;
	gchar *objectid;

	g_return_val_if_fail(uri != NULL, FALSE);
	objectid = mafw_source_create_objectid(uri);
	isok = mafw_playlist_append_item(playlist, objectid, error);
	g_free(objectid);
	return isok;
}

/**
 * mafw_playlist_remove_item:
 * @playlist: a #MafwPlaylist instance.
 * @index:    position of an element to remove in the playlist.
 *            Valid value is between 0 and (playlist size - 1).
 * @error: return location for a GError, or NULL
 *
 * Removes an item from a playlist.  The @index parameter is an
 * absolute visual index.
 *
 * Returns: %TRUE if the operation was successful. In case of error, details
 * are set in the error argument.
 */
gboolean mafw_playlist_remove_item(MafwPlaylist *playlist, guint index,
			       GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->remove_item(playlist, index,
								error);
}

/**
 * mafw_playlist_move_item:
 * @playlist: a #MafwPlaylist instance.
 * @from:     a position in the playlist to move an item from.  Valid
 *            value range is between 0 and (playlist size - 1).
 * @to:       a position in the playlist move the item to.  Valid
 *            value range is between 0 and playlist size, and must not
 *            be equal to @from.
 * @error: return location for a GError, or NULL
 *
 * Moves an item in the playlist from one position to another.
 * Notice that both locations must be valid for the given playlist
 * (i.e. within the boundaries of the playlist).
 *
 * The playing order of the playlist is not affected by the operation.
 * That is, if the $i:th item played at the $o:th position before moving
 * the $i:th item will be played at the corresponding $o:th position after
 * the move too.
 *
 * Examples:
 *
 * <itemizedlist>
 * <listitem><para>playlist = [A, B, C, D]</para></listitem>
 * <listitem><para>mafw_playlist_move_item(playlist, 0, 1) -> [B, A,
 * C, D]</para></listitem>
 * <listitem><para>mafw_playlist_move_item(playlist, 3, 1) -> [B, D,
 * A, C]</para></listitem>
 * </itemizedlist>
 *
 * Returns: %TRUE if the operation was successful. In case of error, details
 * are set in the error argument.
 **/
gboolean mafw_playlist_move_item(MafwPlaylist *playlist, guint from, guint to,
			     GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->move_item(playlist, from,
							    to,
							    error);
}

/**
 * mafw_playlist_clear:
 * @playlist: a #MafwPlaylist instance.
 * @error: return location for a GError, or NULL
 *
 * Removes all entries from the given playlist.
 *
 * Returns: %TRUE if the operation was successful. In case of error,
 * details are set in the error argument.
 **/
gboolean mafw_playlist_clear(MafwPlaylist *playlist, GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->clear(playlist, error);
}

/**
 * mafw_playlist_get_item:
 * @playlist: a #MafwPlaylist instance.
 * @index:    an index of an item to get from playlist.  Valid value
 *            range is between 0 and (playlist size - 1).
 * @error: return location for a GError, or NULL
 *
 * Gets the item at the given playlist index.
 *
 * Returns: the item at the given index or %NULL if not available.  Must be
 * freed with g_free(). In case of error, details are set in the error
 * argument.
 **/
gchar *mafw_playlist_get_item(MafwPlaylist *playlist, guint index,
			      GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), NULL);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->get_item(playlist, index,
							   error);
}

/**
 * mafw_playlist_get_items:
 * @playlist: a #MafwPlaylist instance.
 * @first_index:    the first index of an item to get from playlist.  Valid value
 *            range is between 0 and (playlist size - 1).
 * @last_index:    the last index of an item to get from playlist.  Valid value
 *            range is between 0 and (playlist size - 1).
 * @error: return location for a GError, or NULL
 *
 * Get the items between the given indicies from the given playlist.
 *
 * Returns: NULL terminated array of the items from the given index or 
 * %NULL if not available.  Must be
 * freed with g_strfreev(). In case of error, details are set in the error
 * argument.
 **/
gchar **mafw_playlist_get_items(MafwPlaylist *playlist,
			      guint first_index, guint last_index,
			      GError **error)
{
	GPtrArray *oids = NULL;
	gchar *cur_item, **retarray;
	guint i;
	GError *err = NULL;

	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), NULL);
	if (MAFW_PLAYLIST_GET_IFACE(playlist)->get_items)
		return MAFW_PLAYLIST_GET_IFACE(playlist)->get_items(playlist,
							   first_index,
							   last_index, error);
	oids = g_ptr_array_new();
	i = first_index;
	while (i <= last_index)
	{
		cur_item = mafw_playlist_get_item(playlist, i, &err);
		if (err)
		{
			g_error_free(err);
			break;
		}
		g_ptr_array_add(oids, cur_item);
		i++;
	}
	g_ptr_array_add(oids, NULL);
	retarray = (gchar**)oids->pdata;
	g_ptr_array_free(oids, FALSE);
	return retarray;
}

/**
 * mafw_playlist_get_starting_index:
 * @playlist: a #MafwPlaylist instance.
 * @index:    Pointer to an unsigned-integer, where the index will be updated to
	      visual index of the first item
 * @object_id: return location for the object-id of the first playable item
 * @error:    return location for a GError, or NULL
 *
 * Sets the objectid and the visual index of the first playable item. In case
 * of error, @error will be set, if any given.
 **/
void mafw_playlist_get_starting_index(MafwPlaylist *playlist, guint *index,
        				gchar **object_id, GError **error)
{
	g_return_if_fail(MAFW_IS_PLAYLIST(playlist));
	MAFW_PLAYLIST_GET_IFACE(playlist)->get_starting_index(playlist,
							index,
							object_id,
							error);
}

/**
 * mafw_playlist_get_last_index:
 * @playlist: a #MafwPlaylist instance.
 * @index:    Pointer to an unsigned-integer, where the index will be updated to
	      visual index of the last item
 * @object_id: return location for the object-id of the last playable item
 * @error:    return location for a GError, or NULL
 *
 * Sets the objectid and the visual index of the last playable item. In case
 * of error, @error will be set, if any given.
 **/
void mafw_playlist_get_last_index(MafwPlaylist *playlist, guint *index,
        				gchar **object_id, GError **error)
{
	g_return_if_fail(MAFW_IS_PLAYLIST(playlist));
	MAFW_PLAYLIST_GET_IFACE(playlist)->get_last_index(playlist,
							index,
							object_id,
							error);
}

/**
 * mafw_playlist_get_next:
 * @playlist: a #MafwPlaylist instance.
 * @index:    Pointer to an unsigned-integer what stores the current item. It
	      will be updated to the visual index of the next playable item.
 * @object_id: return location for the object-id if the next playable item
 * @error:    return location for a GError, or NULL
 *
 * Sets the objectid and the visual index of the next playable item. The current
 * item's visual index is stored in @index. In case of error, @error will be
 * set, if any given.
 *
 * Returns: %TRUE if the operation was successful, %FALSE otherwise. In case of
 * error, details are set in the error argument.
 **/
gboolean mafw_playlist_get_next(MafwPlaylist *playlist, guint *index,
        			gchar **object_id, GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->get_next(playlist, index,
							object_id,
							error);
}

/**
 * mafw_playlist_get_prev:
 * @playlist: a #MafwPlaylist instance.
 * @index:    Pointer to an unsigned-integer what stores the current item. It
	      will be updated to the visual index of the previous playable item.
 * @object_id: return location for the object-id if the previous playable item
 * @error:    return location for a GError, or NULL
 *
 * Sets the objectid and the visual index of the previous playable item. The current
 * item's visual index is stored in @index. In case of error, @error will be
 * set, if any given.
 *
 * Returns: %TRUE if the operation was successful, %FALSE otherwise. In case of
 * error, details are set in the error argument.
 **/
gboolean mafw_playlist_get_prev(MafwPlaylist *playlist, guint *index,
        			gchar **object_id, GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), FALSE);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->get_prev(playlist, index,
							object_id,
							error);
}
/**
 * mafw_playlist_get_size:
 * @playlist: a #MafwPlaylist instance.
 * @error: return location for a GError, or NULL
 *
 * Gets the number of items in the playlist.
 *
 * Returns: the number of elements in the playlist. In case of error, details
 * are set in the error argument.
 **/
guint mafw_playlist_get_size(MafwPlaylist *playlist, GError **error)
{
	g_return_val_if_fail(MAFW_IS_PLAYLIST(playlist), 0);
	return MAFW_PLAYLIST_GET_IFACE(playlist)->get_size(playlist, error);
}

/* get/set */

/**
 * mafw_playlist_set_name:
 * @pls: a #MafwPlaylist instance.
 * @name: the name to set.
 *
 * Sets the name of the playlist.
 */
void mafw_playlist_set_name(MafwPlaylist *pls, const gchar *name)
{
	g_object_set(pls, "name", name, NULL);
}

/**
 * mafw_playlist_get_name:
 * @pls: a #MafwPlaylist instance.
 *
 * Gets the playlist name.
 *
 * Returns: a newly allocated string with the name of the @pls.
 */
gchar *mafw_playlist_get_name(MafwPlaylist *pls)
{
	gchar *name;
	g_object_get(pls, "name", &name, NULL);
	return name;
}

/**
 * mafw_playlist_set_repeat:
 * @pls: a #MafwPlaylist instance.
 * @repeat: repeat property.
 *
 * Sets the repeat property of @pls.
 */
void mafw_playlist_set_repeat(MafwPlaylist *pls, gboolean repeat)
{
	g_object_set(pls, "repeat", repeat, NULL);
}

/**
 * mafw_playlist_get_repeat:
 * @pls: a #MafwPlaylist instance.
 *
 * Gets the repeat property of @pls.
 *
 * Returns: the repeat property of @pls.
 */
gboolean mafw_playlist_get_repeat(MafwPlaylist *pls)
{
	gboolean repeat;
	g_object_get(pls, "repeat", &repeat, NULL);
	return repeat;
}

/**
 * mafw_playlist_is_shuffled:
 * @pls: a #MafwPlaylist instance.
 *
 * Gets the information about shuffle.
 *
 * Returns: the is-shuffled property of @pls.
 */
gboolean mafw_playlist_is_shuffled(MafwPlaylist *pls)
{
	gboolean shuffled;
	g_object_get(pls, "is-shuffled", &shuffled, NULL);
	return shuffled;
}

/* Multiple Items With Metadata */

/* Active requests (struct GetPlItemData *). */
static GQueue *Active_miwmds = NULL;

struct GetPlItemData
{
	gchar **oids;
	guint from;
	gboolean cancelled;
	guint remaining_reqs;
	GHashTable *indexhash;
	gchar **keys;
	MafwPlaylistGetItemsCB cb;
	gpointer cbarg;
	MafwPlaylist *pls;
	GDestroyNotify free_cbarg;
};

static void miwmd_free(struct GetPlItemData *mi)
{
	g_assert(g_queue_find(Active_miwmds, mi));
	g_queue_remove(Active_miwmds, mi);
	if (mi->free_cbarg)
		mi->free_cbarg(mi->cbarg);
	g_strfreev(mi->keys);
	if (mi->indexhash)
		g_hash_table_unref(mi->indexhash);
	g_strfreev(mi->oids);
	g_free(mi);
}
static void miwd_got_mdatas(MafwSource *self, GHashTable *metadatas,
				struct GetPlItemData *data, const GError *error)
{
	GHashTableIter htiter;
	GSList *idxlist, *iter;;
	gchar *oid;
	GHashTable *cur_md;

	if (metadatas)
	{
		g_hash_table_iter_init(&htiter, metadatas);

		while (!data->cancelled && g_hash_table_iter_next(&htiter,
							(gpointer*)&oid,
							(gpointer*)&cur_md))
		{
			idxlist = g_hash_table_lookup(data->indexhash, oid);
			iter = idxlist;
		
			while (!data->cancelled && iter)
			{
				data->cb(data->pls, GPOINTER_TO_UINT(iter->data),
					oid, cur_md, data->cbarg);
				iter = g_slist_next(iter);
			}
			g_hash_table_remove(data->indexhash, oid);
		}
	}
	
	data->remaining_reqs--;
	
	if (!data->remaining_reqs)
	{
		/* Call the remaining objects with NULL mdata */
		g_hash_table_iter_init(&htiter, data->indexhash);
		while (!data->cancelled && g_hash_table_iter_next(&htiter,
						(gpointer*)&oid,
						(gpointer*)&idxlist))
		{
			iter = idxlist;
			while (iter && !data->cancelled)
			{
				data->cb(data->pls, GPOINTER_TO_UINT(iter->data),
					oid, NULL, data->cbarg);
				iter = g_slist_next(iter);
			}
		}
		miwmd_free(data);
	}
}

static void _free_ptr_array(gpointer key, GPtrArray *arr, gpointer user_data)
{
	g_ptr_array_free(arr, TRUE);
}

static gboolean miwd_send_requests(struct GetPlItemData *pldata)
{
	guint i = 0;
	GHashTable *helperhash;
	MafwRegistry *reg;
	GHashTableIter htiter;
	GPtrArray *oblist;
	MafwSource *source;
	gchar *uuid = NULL;

	if (pldata->cancelled)
	{
		miwmd_free(pldata);
		return FALSE;
	}
	
	helperhash = g_hash_table_new(g_direct_hash, g_direct_equal);
	pldata->indexhash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
						(GDestroyNotify)g_slist_free);
	reg = MAFW_REGISTRY(mafw_registry_get_instance());

	while(!pldata->cancelled && pldata->oids[i])
	{
		if (mafw_source_split_objectid(pldata->oids[i], &uuid, NULL)) {
			GSList *idxlist;

			source = MAFW_SOURCE(
				mafw_registry_get_extension_by_uuid(reg,
								    uuid));
			g_free(uuid);
			if (!source || !pldata->keys) {
				/* Call the callback to notify
				 * that we could not get
				 * metadata for this item,
				 * either because the source
				 * is not available (!source)
				 * or because we have not been
				 * asked any metadata at all
				 * (!mi->keys) */
				pldata->cb(pldata->pls, pldata->from + i,
					pldata->oids[i], NULL,
				       pldata->cbarg);
			} else {
				oblist = g_hash_table_lookup(helperhash, source);
				
				if (!oblist)
				{
					oblist = g_ptr_array_new();
				}
				
				g_ptr_array_add(oblist, pldata->oids[i]);
				
				g_hash_table_replace(helperhash, source, oblist);
				idxlist = g_hash_table_lookup(pldata->indexhash,
								pldata->oids[i]);
				idxlist = g_slist_prepend(idxlist, 
						GUINT_TO_POINTER(pldata->from + i));
				g_hash_table_replace(pldata->indexhash,
							pldata->oids[i], 
							idxlist);
			}
		}
		else
		{
			pldata->cb(pldata->pls, pldata->from + i,
					pldata->oids[i], NULL, pldata->cbarg);
		}
		i++;
	}

	if (pldata->cancelled || ((i = g_hash_table_size(helperhash)) == 0))
	{
		miwmd_free(pldata);
		g_hash_table_foreach(helperhash, (GHFunc)_free_ptr_array, NULL);
		g_hash_table_destroy(helperhash);
		return FALSE;
	}

	
	pldata->remaining_reqs = i;
	g_hash_table_iter_init(&htiter, helperhash);
	while (g_hash_table_iter_next(&htiter, (gpointer*)&source,
						(gpointer*)&oblist))
	{
		g_ptr_array_add(oblist, NULL);
		mafw_source_get_metadatas(source, (const gchar**)oblist->pdata,
					(const gchar**)pldata->keys, 
					(MafwSourceMetadataResultsCb)miwd_got_mdatas, 
					pldata);
	}
	g_hash_table_foreach(helperhash, (GHFunc)_free_ptr_array, NULL);
	g_hash_table_destroy(helperhash);

	return FALSE;
}

/**
 * mafw_playlist_get_items_md:
 * @pls:        a #MafwPlaylist instance.
 * @from:       first index, inclusive.
 * @to:         last index, inclusive, or a negative number for the all
 *              remaining elements.
 * @keys:       metadata keys to retrieve along with the items.
 * @cb:         callback, which gets called with the results.
 * @cbarg:      additional data passed to @cb.
 * @free_cbarg: destructor of @cbarg, called after the callback has been called
 *              for the very last time.
 *
 * Fetch items and metadata (@keys) from @pls asynchronously via an idle
 * callback.  For each item in @pls, the requested metadata @keys will be
 * queried, and eventually @cb will be called.  It is not guaranteed that the
 * callback is called in the order of the items.  At the very end, @free_cbarg
 * is called.  It is possible to cancel the operation with
 * mafw_playlist_cancel_get_items_md().
 *
 * Returns: an opaque identifier which can be used to
 * mafw_playlist_cancel_get_items_md() the operation.
 */
gpointer mafw_playlist_get_items_md(MafwPlaylist *pls,
				    guint from, gint to,
				    const gchar *const keys[],
				    MafwPlaylistGetItemsCB cb, gpointer cbarg,
				    GDestroyNotify free_cbarg)
{
	gchar **pl_items;
	GError *err = NULL;
	
	struct GetPlItemData *pldata;

	g_return_val_if_fail(pls, NULL);
	g_return_val_if_fail(cb, NULL);
	g_return_val_if_fail(to < 0 || from <= to, NULL);

	if (!Active_miwmds)
		Active_miwmds = g_queue_new();

	if (to < 0) {
		gint size;
		GError *error = NULL;

		size = mafw_playlist_get_size(pls, &error);

		if (size <= 0)
		{
			if (free_cbarg)
				free_cbarg(cbarg);
			return NULL;
		}

		if (error == NULL) {
			to = size - 1;
		} else {
			g_warning("Could not get playlist size to get items "
				  "metadata because: %s", error->message);
			g_error_free(error);
			return NULL;
		}
	}

	pl_items = mafw_playlist_get_items(pls, from, to, &err);
	
	if (err)
	{
		g_warning("Could not get playlist items to get items "
				  "metadata because: %s", err->message);
		g_error_free(err);
		return NULL;
	}

	pldata = g_new0(struct GetPlItemData, 1);
	pldata->remaining_reqs = 0;
	pldata->cb = cb;
	pldata->pls = pls;
	pldata->cbarg = cbarg;
	pldata->oids = pl_items;
	pldata->free_cbarg = free_cbarg;
	pldata->cancelled = FALSE;
	pldata->from = from;
	if (keys)
		pldata->keys = g_strdupv((gchar**)keys);
	pldata->indexhash = NULL;
	
	g_queue_push_tail(Active_miwmds, pldata);
	g_idle_add((GSourceFunc)miwd_send_requests, pldata);
	return pldata;
}

/**
 * mafw_playlist_cancel_get_items_md:
 * @op: the operation identifier.
 *
 * Cancels a previous mafw_playlist_get_items_md() operation.
 */
void mafw_playlist_cancel_get_items_md(gconstpointer op)
{
	GList *link;
	struct GetPlItemData *mi;

	if (!Active_miwmds)
		/* Canceling without ever initiating any operation, ey ey. */
		return;
	link = g_queue_find(Active_miwmds, op);
	if (!link)
		/* Unknown request, probably already freed. */
		return;
	mi = link->data;
	mi->cancelled = TRUE;
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
