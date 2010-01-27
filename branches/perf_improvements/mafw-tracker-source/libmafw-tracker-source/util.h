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

#ifndef _MAFW_TRACKER_SOURCE_UTIL_H_
#define _MAFW_TRACKER_SOURCE_UTIL_H_

#include <glib.h>
#include <libmafw/mafw.h>
#include <tracker.h>
#include "definitions.h"

#define IS_STRING_EMPTY(str) ((str) == NULL || (str)[0] == '\0')

typedef enum {
	CATEGORY_ROOT,
	CATEGORY_VIDEO,
	CATEGORY_MUSIC,
	CATEGORY_MUSIC_PLAYLISTS,
	CATEGORY_MUSIC_SONGS,
	CATEGORY_MUSIC_ALBUMS,
	CATEGORY_MUSIC_GENRES,
	CATEGORY_MUSIC_ARTISTS,
	CATEGORY_ERROR
} CategoryType;

GList *util_itemid_to_path(const gchar *item_id);
gchar *util_unescape_string(const gchar* original);
inline gchar *get_data(const GList * list);

#ifndef G_DEBUG_DISABLE
void perf_elapsed_time_checkpoint(gchar *event);
#endif  /* G_DEBUG_DISABLE */

gchar *util_epoch_to_iso8601(glong epoch);
gchar *util_escape_rdf_text(const gchar *text);
gboolean util_mafw_filter_to_rdf(const MafwFilter *filter,
				 GString *p);
gchar *util_get_tracker_value_for_filter(gint mafw_key_id,
                                         ServiceType service,
					 const gchar *value);
gboolean util_tracker_value_is_unknown(const gchar *value);
gchar **util_create_sort_keys_array(gint n, gchar *key1, ...);
gchar *util_create_filter_from_category(const gchar *genre,
					const gchar *artist,
					const gchar *album,
					const gchar *user_filter);
gchar *util_build_complex_rdf_filter(gchar **filters,
				     const gchar *append_filter);
CategoryType util_extract_category_info(const gchar *object_id,
                                        gchar **genre,
                                        gchar **artist,
                                        gchar **album,
                                        gchar **clip);
gboolean util_is_duration_requested(guint64 key_list);
gboolean util_calculate_playlist_duration_is_needed(GHashTable *pls_metadata);
guint64 util_add_tracker_data_to_check_pls_duration(guint64 keys);
guint64 util_remove_tracker_data_to_check_pls_duration(GHashTable *metadata,
						    guint64 metadata_keys);

#endif
