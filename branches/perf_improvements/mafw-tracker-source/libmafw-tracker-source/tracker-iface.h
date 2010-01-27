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

#ifndef __MAFW_TRACKER_IFACE_H__
#define __MAFW_TRACKER_IFACE_H__

#include <libmafw/mafw.h>
#include "util.h"

typedef struct {
	GList *ids;
	GList *metadata_values;
} MafwResult;

enum TrackerObjectType {
        TRACKER_TYPE_MUSIC,
        TRACKER_TYPE_VIDEO,
        TRACKER_TYPE_PLAYLIST
};

typedef void (*MafwTrackerSongsResultCB)(MafwResult *result,
					 GError *error,
					 gpointer user_data);

typedef void (*MafwTrackerMetadataResultCB)(GHashTable *result,
					    GError *error,
					    gpointer user_data);

typedef void (*MafwTrackerMetadatasResultCB)(GList *results,
                                             GError *error,
                                             gpointer user_data);

gboolean ti_init(void);
void ti_init_watch(GObject *source);
void ti_deinit(void);

gchar *ti_create_filter(const MafwFilter *filter_str);

void ti_get_songs(gchar *genre,
                  gchar *artist,
                  gchar *album,
                  guint64 keys,
                  const gchar *user_filter,
                  gchar **sort_fields,
                  guint offset,
                  guint count,
                  MafwTrackerSongsResultCB callback,
                  gpointer user_data);

void ti_get_videos(guint64 keys,
		   const gchar *rdf_filter,
		   gchar **sort_fields,
		   guint offset,
		   guint count,
		   MafwTrackerSongsResultCB callback,
		   gpointer user_data);

void ti_get_albums(gchar *genre,
                   gchar *artist,
                   guint64 keys,
                   const gchar *rdf_filter,
                   gchar **sort_fields,
                   guint offset,
                   guint count,
                   MafwTrackerSongsResultCB callback,
                   gpointer user_data);

void ti_get_artists(gchar *genre,
                    guint64 keys,
		    const gchar *rdf_filter,
		    gchar **sort_fields,
		    guint offset,
		    guint count,
		    MafwTrackerSongsResultCB callback,
		    gpointer user_data);

void ti_get_genres(guint64 keys,
		   const gchar *rdf_filter,
		   gchar **sort_fields,
		   guint offset,
		   guint count,
		   MafwTrackerSongsResultCB callback,
		   gpointer user_data);

void ti_get_playlists(guint64 keys,
		      const gchar *user_filter,
		      gchar **sort_fields,
		      guint offset,
		      guint count,
		      MafwTrackerSongsResultCB callback,
		      gpointer user_data);

void ti_get_metadata_from_videoclip(gchar **uris,
                                    guint64 keys,
                                    MafwTrackerMetadatasResultCB callback,
                                    gpointer user_data);

void ti_get_metadata_from_audioclip(gchar **uris,
                                   guint64 keys,
                                    MafwTrackerMetadatasResultCB callback,
                                    gpointer user_data);

void ti_get_metadata_from_playlist(gchar **uris,
				   guint64 keys,
				   MafwTrackerMetadatasResultCB callback,
				   gpointer user_data);

void ti_get_metadata_from_category(gchar *genre,
                                   gchar *artist,
                                   gchar *album,
                                   gint default_count_key,
                                   gchar *title,
                                   guint64 keys,
                                   MafwTrackerMetadataResultCB callback,
                                   gpointer user_data);

void ti_get_metadata_from_videos(guint64 keys,
                                 gchar *title,
                                 MafwTrackerMetadataResultCB callback,
                                 gpointer user_data);

void ti_get_metadata_from_music(guint64 keys,
                                gchar *title,
                                MafwTrackerMetadataResultCB callback,
                                gpointer user_data);

void ti_get_metadata_from_playlists(guint64 keys,
                                    gchar *title,
                                    MafwTrackerMetadataResultCB callback,
                                    gpointer user_data);

void ti_set_playlist_duration(const gchar *uri, guint duration);

gchar **ti_set_metadata(const gchar *uri, 
			GHashTable *metadata,
			CategoryType category,
			gboolean *updated);

#endif
