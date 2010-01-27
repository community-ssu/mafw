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

#include <glib.h>
#include <string.h>
#include <tracker.h>
#include <libmafw/mafw.h>
#include <totem-pl-parser.h>
#include <stdlib.h>
#include <dbus/dbus-glib.h>

#include "tracker-iface.h"
#include "tracker-cache.h"
#include "mafw-tracker-source.h"
#include "mafw-tracker-source-marshal.h"
#include "util.h"
#include "album-art.h"
#include "key-mapping.h"

#define AGGREGATED_TYPE_CONCAT "CONCAT"
#define AGGREGATED_TYPE_COUNT  "COUNT"
#define AGGREGATED_TYPE_SUM    "SUM"

extern gint maxid;
/* ------------------------ Internal types ----------------------- */

/* Stores information needed to invoke MAFW's callback after getting
   results from tracker */
struct _mafw_query_closure {
	/* Mafw callback */
	MafwTrackerSongsResultCB callback;
	/* Calback's user_data */
	gpointer user_data;
        /* Cache to store keys and values */
        TrackerCache *cache;
	guint64 asked_keys;
};

struct _mafw_metadata_closure {
        /* Mafw callback */
	gpointer callback;
        /* Callback's user data */
        gpointer user_data;
        /* If the childcount key must be counted instead of aggregated
         * when aggregating data in get_metadata */
        gboolean count_childcount;
        /* Type of service to be used in tracker */
        enum TrackerObjectType tracker_type;
        /* Cache to store keys and values */
        TrackerCache *cache;
	/* List of paths to the asked items */
	gchar **path_list;
	guint64 asked_keys;
};

/* ---------------------------- Globals -------------------------- */

static TrackerClient *tc = NULL;

/* ------------------------- Private API ------------------------- */

static void _stats_changed_handler(DBusGProxy *proxy,
				   GPtrArray *change_set,
				   gpointer user_data)
{	gint i;
	MafwSource *source;
	const gchar **p;
	const gchar *service_type = NULL;

	if (change_set == NULL) {
		return;
	}

	source = (MafwSource *) user_data;

	for (i = 0; i < change_set->len; i++) {
		p = g_ptr_array_index(change_set, i);
		service_type = p[0];

                if (!service_type) {
                        continue;
                }

		if (strcmp(service_type, "Music") == 0) {
			g_signal_emit_by_name(source,
					      "container-changed",
					      MUSIC_OBJECT_ID);
		} else if (strcmp(service_type, "Videos") == 0) {
			g_signal_emit_by_name(source,
					      "container-changed",
					      VIDEOS_OBJECT_ID);
		} else if (strcmp(service_type, "Playlists") == 0) {
			g_signal_emit_by_name(source,
					      "container-changed",
					      PLAYLISTS_OBJECT_ID);
		}
	}
}

static void _index_state_changed_handler(DBusGProxy *proxy,
                                         const gchar *state,
                                         gboolean initial_index,
                                         gboolean in_merge,
                                         gboolean is_paused_manually,
                                         gboolean is_paused_for_bat,
                                         gboolean is_paused_for_io,
                                         gboolean is_indexing_enabled,
                                         gpointer user_data)
{
        MafwTrackerSource *source;

        source = MAFW_TRACKER_SOURCE(user_data);

        if (source->priv->last_progress != 100 &&
            strcmp(state, "Idle") == 0) {
                /* Indexing has finished */
                source->priv->last_progress = 100;
                source->priv->remaining_items = 0;
                source->priv->remaining_time = 0;
                g_signal_emit_by_name(source, "updating", 100,
                                      source->priv->processed_items,
                                      source->priv->remaining_items,
                                      source->priv->remaining_time);
        } else if (source->priv->last_progress == 100 &&
                   strcmp(state, "Indexing") == 0) {
                /* Tracker has began to index */
                source->priv->last_progress = 0;
                source->priv->processed_items = 0;
                source->priv->remaining_items = -1;
                source->priv->remaining_items = -1;
                g_signal_emit_by_name(source, "updating", 0,
                                      source->priv->processed_items,
                                      source->priv->remaining_items,
                                      source->priv->remaining_time);
        }
}

static void _progress_changed_handler (DBusGProxy  *proxy,
                                       const gchar *service,
                                       const gchar *uri,
                                       gint items_done,
                                       gint items_remaining,
                                       gint items_total,
                                       gdouble seconds_elapsed,
                                       gpointer user_data)
{
        MafwTrackerSource *source;
        gint progress;
        gint time_remaining;

        source = MAFW_TRACKER_SOURCE(user_data);

        /* Reserve 100% when index finishes */
	if (items_total > 0) {
		progress = CLAMP(100*items_done/items_total, 0, 99);
	} else {
		progress = 0;
	}

        if (items_done > 0) {
                time_remaining =
                        items_remaining * (seconds_elapsed / items_done);
        } else {
                time_remaining = -1;
        }


        if (source->priv->last_progress != progress ||
            source->priv->processed_items != items_done ||
            source->priv->remaining_items != items_remaining ||
            source->priv->remaining_time != time_remaining) {
                source->priv->last_progress = progress;
                source->priv->processed_items = items_done;
                source->priv->remaining_items = items_remaining;
                source->priv->remaining_time = time_remaining;
                g_signal_emit_by_name(source, "updating", progress, items_done,
                                      items_remaining, time_remaining);
        }
}


static GList *_build_objectids_from_pathname(TrackerCache *cache)
{
        GList *objectid_list = NULL;
        const GPtrArray *results;
        gchar *uri;
        gchar *pathname;
        gint i;

        results = tracker_cache_values_get_results(cache);
        for (i = 0; i < results->len; i++) {
		gboolean be_free;
                uri = tracker_cache_value_get_str(cache, MTrackerSrc_ID_URI, i, 0, &be_free);
                pathname = g_filename_from_uri(uri, NULL, NULL);
		if (be_free)
			g_free(uri);
                objectid_list = g_list_prepend(objectid_list, pathname);
        }
        objectid_list = g_list_reverse(objectid_list);

        return objectid_list;
}

static GList *_build_objectids_from_unique_key(TrackerCache *cache)
{
	MetadataKey *metadata_key;
	GList *objectid_list = NULL;
	gchar *unique_value;
	gint i;

	metadata_key = keymap_get_metadata_by_id(cache->asked_uniq_id);

	if (!metadata_key)
		return NULL;

	for (i = 0; i < cache->tracker_results->len; i++) {
		if (metadata_key->value_type == G_TYPE_STRING)
		{
			gboolean be_free;
			unique_value = tracker_cache_value_get_str(cache,
						cache->asked_uniq_id, i, 0,
						&be_free);
			if (!be_free)
				unique_value = g_strdup(unique_value);
		}
		else if (metadata_key->value_type == G_TYPE_INT)
		{
			unique_value =
                                g_strdup_printf("%d", 
					tracker_cache_value_get_int(cache,
						cache->asked_uniq_id, i, 0));
		}
		else
		{
			unique_value = g_strdup("");
		}
		objectid_list = g_list_prepend(objectid_list, unique_value);
	}
	objectid_list = g_list_reverse(objectid_list);
	return objectid_list;
}

static void _tracker_query_cb(GPtrArray *tracker_result,
                              GError *error,
                              gpointer user_data)
{
	MafwResult *mafw_result = NULL;
	struct _mafw_query_closure *mc;

	mc = (struct _mafw_query_closure *) user_data;

	if (error == NULL) {
                mafw_result = g_slice_new(MafwResult);
                tracker_cache_values_add_results(mc->cache, tracker_result);
                mafw_result->metadata_values =
                        tracker_cache_build_metadata(mc->cache, mc->asked_keys, NULL);
                mafw_result->ids =
                        _build_objectids_from_pathname(mc->cache);

                /* Invoke callback */
                mc->callback(mafw_result, NULL, mc->user_data);
	} else {
                g_warning("Error while querying: %s\n", error->message);
                mc->callback(NULL, error, mc->user_data);
        }

        tracker_cache_free(mc->cache);
        g_slice_free(struct _mafw_query_closure, mc);
}

static void _tracker_unique_values_cb(GPtrArray *tracker_result,
				      GError *error,
				      gpointer user_data)
{
	MafwResult *mafw_result = NULL;
	struct _mafw_query_closure *mc;

	mc = (struct _mafw_query_closure *) user_data;

	if (error == NULL) {
                mafw_result = g_slice_new(MafwResult);
                tracker_cache_values_add_results(mc->cache, tracker_result);
                mafw_result->metadata_values =
                        tracker_cache_build_metadata(mc->cache, mc->asked_keys,
							NULL);
                mafw_result->ids = _build_objectids_from_unique_key(mc->cache);

                /* Invoke callback */
                mc->callback(mafw_result, NULL, mc->user_data);
        } else {
                g_warning("Error while querying: %s\n", error->message);
                mc->callback(NULL, error, mc->user_data);
        }

        tracker_cache_free(mc->cache);
        g_slice_free(struct _mafw_query_closure, mc);
}

static void _do_tracker_get_unique_values(gchar **keys,
                                          gchar **aggregated_keys,
                                          gchar **aggregated_types,
                                          char **filters,
                                          guint offset,
                                          guint count,
                                          struct _mafw_query_closure *mc)
{
        gchar *filter = NULL;

        filter = util_build_complex_rdf_filter(filters, NULL);

#ifndef G_DEBUG_DISABLE
	perf_elapsed_time_checkpoint("Ready to query Tracker");
#endif

        tracker_metadata_get_unique_values_with_aggregates_async(
                tc,
                SERVICE_MUSIC,
                keys,
                filter,
                aggregated_types,
                aggregated_keys,
                FALSE,
                offset,
                count,
                _tracker_unique_values_cb,
                mc);

	g_free(filter);
}

static void _tracker_metadata_cb(GPtrArray *results,
                                 GError *error,
                                 gpointer user_data)
{
        GList *metadata_list = NULL;
        struct _mafw_metadata_closure *mc =
                (struct _mafw_metadata_closure *) user_data;

        if (!error) {
                tracker_cache_values_add_results(mc->cache, results);
                metadata_list = tracker_cache_build_metadata(mc->cache,
					mc->asked_keys,
					(const gchar**)mc->path_list);
                ((MafwTrackerMetadatasResultCB)mc->callback)(metadata_list, NULL, mc->user_data);
                g_list_foreach(metadata_list, (GFunc) g_hash_table_unref, NULL);
                g_list_free(metadata_list);
        } else {
                g_warning("Error while getting metadata: %s\n",
                          error->message);
                ((MafwTrackerMetadatasResultCB)mc->callback)(NULL, error, mc->user_data);
        }

        tracker_cache_free(mc->cache);
	g_strfreev(mc->path_list);
	g_slice_free(struct _mafw_metadata_closure, mc);
}

static gchar **_uris_to_filenames(gchar **uris)
{
        gchar **filenames;
        gint i;

        filenames = g_new0(gchar *, g_strv_length(uris) + 1);
        for (i = 0; uris[i]; i++) {
                filenames[i] = g_filename_from_uri(uris[i], NULL, NULL);
        }

        return filenames;
}

static gboolean _run_tracker_metadata_cb(gpointer data)
{
        _tracker_metadata_cb(NULL, NULL, data);

        return FALSE;
}

static gchar **_get_tracker_keys(TrackerCache *cache,
					 ServiceType service)
{
	GPtrArray *str_array;
	guint64 asked_keys = cache->asked_tracker_keys;
	const TrackerKey *trk;
	gint id = 0;

	if (cache->result_type == TRACKER_CACHE_RESULT_TYPE_QUERY)
	{/* Do not ask URI */
		asked_keys >>= 1;
		id++;
	}
	str_array = g_ptr_array_new();
	
	while (asked_keys)
	{
		if ((asked_keys & 1) == 1)
		{
			trk = keymap_get_tracker_info_by_id(id, service);
			
			if (trk)
			{
				g_ptr_array_add(str_array, trk->tracker_key);
			}
			else
			{
				guint64 flag_to_remove = keymap_get_flag_from_keyid(id);
				
				cache->asked_tracker_keys &= ~flag_to_remove;
			}
		}
		asked_keys >>= 1;
		id++;
	}
	g_ptr_array_add(str_array, NULL);

	return (gchar**)g_ptr_array_free(str_array, FALSE);
}

static void _do_tracker_get_metadata(gchar **uris,
				     guint64 keys,
				     enum TrackerObjectType tracker_obj_type,
				     MafwTrackerMetadatasResultCB callback,
				     gpointer user_data)
{
	gchar **tracker_keys;
	gint service_type;
        struct _mafw_metadata_closure *mc = NULL;
        gchar **pathnames;

	/* Figure out tracker service type */
	if (tracker_obj_type == TRACKER_TYPE_VIDEO) {
		service_type = SERVICE_VIDEOS;
	} else if (tracker_obj_type == TRACKER_TYPE_PLAYLIST){
		service_type = SERVICE_PLAYLISTS;
	} else {
		service_type = SERVICE_MUSIC;
	}

        /* Save required information */
        mc = g_slice_new(struct _mafw_metadata_closure);
        mc->callback = callback;
        mc->user_data = user_data;
        mc->cache = tracker_cache_new(service_type,
                                      TRACKER_CACHE_RESULT_TYPE_GET_METADATA);
	mc->count_childcount = FALSE;
	mc->tracker_type = 0;
	mc->path_list = NULL;
	mc->asked_keys = keys;

        /* If we have only a URI, add it as a predefined value */
        if (!uris[1]) {
                tracker_cache_key_add_precomputed_string(mc->cache,
                                                         MTrackerSrc_ID_URI,
                                                         g_strdup(uris[0]),
							 TRUE);
        }

        tracker_cache_key_add_several(mc->cache, keys, 1);

	tracker_keys = _get_tracker_keys(mc->cache,
							service_type);

        if (g_strv_length(tracker_keys) > 0) {
                pathnames = _uris_to_filenames(uris);
		mc->path_list = pathnames;
                tracker_metadata_get_multiple_async(
                        tc,
                        service_type,
                        (const gchar **) pathnames,
                        (const gchar **)tracker_keys,
                        _tracker_metadata_cb,
                        mc);
        } else {
                g_idle_add(_run_tracker_metadata_cb, mc);
        }
	g_free(tracker_keys);
}

/* ------------------------- Public API ------------------------- */

gchar *ti_create_filter(const MafwFilter *filter)
{
	GString *clause = NULL;
	gchar *ret_str = NULL;

	if (filter == NULL)
		return NULL;

	/* Convert the filter to RDF */
	clause = g_string_new("");
	if (util_mafw_filter_to_rdf(filter, clause)) {
		ret_str = g_string_free(clause, FALSE);
	} else {
		g_warning("Invalid or unsupported filter syntax");
		g_string_free(clause, TRUE);
	}

	return ret_str;
}

gboolean ti_init(void)
{
	tc = tracker_connect(TRUE);

	if (tc == NULL) {
		g_critical("Could not get a connection to Tracker. "
			   "Plugin disabled.");
	}

	return tc != NULL;
}

void  ti_init_watch (GObject *source)
{
	DBusGConnection *connection;
	DBusGProxy *proxy;
	GError *error = NULL;
	GType collection_type;

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (connection == NULL) {
		g_warning ("Failed to open connection to bus: %s\n",
			   error->message);
		g_error_free (error);
		return;
	}

	proxy = dbus_g_proxy_new_for_name (connection,
					   "org.freedesktop.Tracker",
					   "/org/freedesktop/Tracker",
					   "org.freedesktop.Tracker");

	collection_type = dbus_g_type_get_collection("GPtrArray",
						      G_TYPE_STRV);
	dbus_g_object_register_marshaller (
                mafw_tracker_source_marshal_VOID__STRING_BOOLEAN_BOOLEAN_BOOLEAN_BOOLEAN_BOOLEAN_BOOLEAN,
                G_TYPE_NONE,
                G_TYPE_STRING,
                G_TYPE_BOOLEAN,
                G_TYPE_BOOLEAN,
                G_TYPE_BOOLEAN,
                G_TYPE_BOOLEAN,
                G_TYPE_BOOLEAN,
                G_TYPE_BOOLEAN,
                G_TYPE_INVALID);
        dbus_g_object_register_marshaller (
                mafw_tracker_source_marshal_VOID__STRING_STRING_INT_INT_INT_DOUBLE,
                G_TYPE_NONE,
                G_TYPE_STRING,
                G_TYPE_STRING,
                G_TYPE_INT,
                G_TYPE_INT,
                G_TYPE_INT,
                G_TYPE_DOUBLE,
                G_TYPE_INVALID);

	dbus_g_proxy_add_signal (proxy,
				 "ServiceStatisticsUpdated",
				 collection_type,
				 G_TYPE_INVALID);
        dbus_g_proxy_add_signal (proxy,
                                 "IndexStateChange",
                                 G_TYPE_STRING,
                                 G_TYPE_BOOLEAN,
                                 G_TYPE_BOOLEAN,
                                 G_TYPE_BOOLEAN,
                                 G_TYPE_BOOLEAN,
                                 G_TYPE_BOOLEAN,
                                 G_TYPE_BOOLEAN,
                                 G_TYPE_INVALID);
        dbus_g_proxy_add_signal (proxy,
                                 "IndexProgress",
                                 G_TYPE_STRING,
                                 G_TYPE_STRING,
                                 G_TYPE_INT,
                                 G_TYPE_INT,
                                 G_TYPE_INT,
                                 G_TYPE_DOUBLE,
                                 G_TYPE_INVALID);

	dbus_g_proxy_connect_signal (proxy, "ServiceStatisticsUpdated",
				     G_CALLBACK(_stats_changed_handler),
				     source, NULL);
        dbus_g_proxy_connect_signal (proxy, "IndexStateChange",
                                     G_CALLBACK (_index_state_changed_handler),
                                     source, NULL);
        dbus_g_proxy_connect_signal (proxy, "IndexProgress",
                                     G_CALLBACK(_progress_changed_handler),
                                     source, NULL);
}

void ti_deinit()
{
	tracker_disconnect(tc);
	tc = NULL;
}

void ti_get_videos(guint64 keys,
		   const gchar *rdf_filter,
		   gchar **sort_fields,
		   guint offset,
		   guint count,
		   MafwTrackerSongsResultCB callback,
		   gpointer user_data)
{
	gchar **tracker_keys;
        gchar **tracker_sort_keys;
	struct _mafw_query_closure *mc;
        gchar *filter;

        if (rdf_filter) {
                filter = g_strdup_printf(RDF_QUERY_BEGIN
                                         " %s "
                                         RDF_QUERY_END,
                                         rdf_filter);
        } else {
                filter = NULL;
	}

	/* Prepare mafw closure struct */
	mc = g_slice_new(struct _mafw_query_closure);
	mc->callback = callback;
	mc->user_data = user_data;
	mc->asked_keys = keys;
        mc->cache = tracker_cache_new(SERVICE_VIDEOS,
                                      TRACKER_CACHE_RESULT_TYPE_QUERY);

        /* Add requested keys; add also uri, as it will be needed to
         * build object_id list */
        tracker_cache_key_add(mc->cache, MTrackerSrc_ID_URI, 1);
        tracker_cache_key_add_several(mc->cache, keys, 1);

	/* Map MAFW keys to Tracker keys */
	tracker_keys = _get_tracker_keys(mc->cache,
							SERVICE_VIDEOS);

	if (sort_fields != NULL) {
		tracker_sort_keys =
			keymap_mafw_sort_keys_to_tracker_keys(sort_fields,
                                                              SERVICE_VIDEOS);
	} else {
		tracker_sort_keys =
			util_create_sort_keys_array(2,
						    TRACKER_VKEY_TITLE,
						    TRACKER_FKEY_FILENAME);
	}

	/* Query tracker */
        tracker_search_query_async(tc, -1,
                                   SERVICE_VIDEOS,
                                   tracker_keys,
                                   NULL,   /* Search text */
                                   NULL,   /* Keywords */
                                   filter,
                                   offset, count,
                                   FALSE,   /* Sort by service */
                                   (gchar**)tracker_sort_keys, /* Sort fields */
                                   FALSE, /* sort descending? */
                                   _tracker_query_cb,
                                   mc);

        if (filter) {
                g_free(filter);
        }

        g_free(tracker_keys);
        g_free(tracker_sort_keys);
}

void ti_get_songs(gchar *genre,
                  gchar *artist,
                  gchar *album,
                  guint64 keys,
                  const gchar *user_filter,
                  gchar **sort_fields,
                  guint offset,
                  guint count,
                  MafwTrackerSongsResultCB callback,
                  gpointer user_data)
{
	gchar **tracker_keys;
        gchar **tracker_sort_keys;
	gchar *rdf_filter = NULL;
        gchar **use_sort_fields;
	struct _mafw_query_closure *mc;

        /* Select default sort fields */
        if (!sort_fields) {
                if (album) {
	                use_sort_fields = g_new0(gchar *, 3);
                        use_sort_fields[0] = MAFW_METADATA_KEY_TRACK;
                        use_sort_fields[1] = MAFW_METADATA_KEY_TITLE;
                } else {
	                use_sort_fields = g_new0(gchar *, 2);
                        use_sort_fields[0] = MAFW_METADATA_KEY_TITLE;
                }
        } else {
                use_sort_fields = sort_fields;
        }

	/* Prepare mafw closure struct */
	mc = g_slice_new(struct _mafw_query_closure);
	mc->callback = callback;
	mc->user_data = user_data;
	mc->asked_keys = keys;
        mc->cache = tracker_cache_new(SERVICE_MUSIC,
                                      TRACKER_CACHE_RESULT_TYPE_QUERY);

        /* Save known values */
        if (genre) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_GENRE,
                        genre,
			TRUE);
        }

        if (artist) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_ARTIST,
                        artist,
			TRUE);
        }

        if (album) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_ALBUM,
                        album,
			TRUE);
        }

        /* Add URI, as it will likely be needed to build ids */
        tracker_cache_key_add(mc->cache, MTrackerSrc_ID_URI, 1);

        /* Add remaining keys */
        tracker_cache_key_add_several(mc->cache, keys, 1);

        /* Get the keys to ask tracker */
	tracker_keys = _get_tracker_keys(mc->cache,
							SERVICE_MUSIC);
        tracker_sort_keys =
                keymap_mafw_sort_keys_to_tracker_keys(use_sort_fields,
                                                      SERVICE_MUSIC);
        rdf_filter = util_create_filter_from_category(genre, artist,
						      album, user_filter);

	/* Map to tracker keys */
        tracker_search_query_async(tc, -1,
                                   SERVICE_MUSIC,
                                   tracker_keys,
                                   NULL,   /* Search text */
                                   NULL,   /* Keywords */
                                   rdf_filter,
                                   offset, count,
                                   FALSE,   /* Sort by service */
                                   tracker_sort_keys, /* Sort fields */
                                   FALSE, /* sort descending? */
                                   _tracker_query_cb,
                                   mc);

	if (rdf_filter) {
                g_free(rdf_filter);
        }

        if (use_sort_fields != sort_fields) {
                g_free(use_sort_fields);
        }

        g_free(tracker_keys);
        g_strfreev(tracker_sort_keys);
}

static void _fill_aggregates(gint keyid, const gchar **aggregate_keys,
			gchar **aggregate_types, gint start_to_look, gint idx, gint childcount_mode)
{
	const gchar *count_keys[] = { TRACKER_AKEY_GENRE, TRACKER_AKEY_ARTIST,
                                      TRACKER_AKEY_ALBUM, "*" };
	MetadataKey *metadata_key;
	gint level;
	

	metadata_key = keymap_get_metadata_by_id(keyid);

	if (!metadata_key)
		return;

	switch (metadata_key->special) {
	case SPECIAL_KEY_DURATION:
		aggregate_keys[idx-1] =
			keymap_get_tracker_key_by_id(keyid, SERVICE_MUSIC);
		aggregate_types[idx-1] = AGGREGATED_TYPE_SUM;
		break;

	case SPECIAL_KEY_CHILDCOUNT:
		if (childcount_mode == 1)
		{
			level = keymap_get_childcount_level(keyid);
			aggregate_keys[idx-1] =
				count_keys[start_to_look + level - 1];
		}
		else if (childcount_mode == 0)
		{
			aggregate_keys[idx-1] = "*";
		} else if (childcount_mode == 2)
		{
			level = keymap_get_childcount_level(keyid);
			/* What is the level requested? */
                        if (level == 1) {
                                aggregate_keys[idx-1] = TRACKER_AKEY_ALBUM;
                        } else {
                                aggregate_keys[idx-1] = "*";
                        }
		} else if (childcount_mode == 3)
		{
			level = keymap_get_childcount_level(keyid);
			/* What is the level requested? */
                        if (level == 1) {
                                aggregate_keys[idx-1] = TRACKER_AKEY_ARTIST;
                        } else if (level == 2) {
                                aggregate_keys[idx-1] = TRACKER_AKEY_ALBUM;
                        } else {
                                aggregate_keys[idx-1] = "*";
                        }
		}
		aggregate_types[idx-1] = AGGREGATED_TYPE_COUNT;
		break;

	default:
		aggregate_keys[idx-1] =
			keymap_get_tracker_key_by_id(keyid, SERVICE_MUSIC);
		aggregate_types[idx-1] = AGGREGATED_TYPE_CONCAT;
	}
}

void ti_get_artists(gchar *genre,
                    guint64 keys,
		    const gchar *rdf_filter,
		    gchar **sort_fields,
		    guint offset,
		    guint count,
		    MafwTrackerSongsResultCB callback,
		    gpointer user_data)
{
        const int MAXLEVEL = 2;
	struct _mafw_query_closure *mc;
        gchar *escaped_genre = NULL;
        gchar **filters;
        gchar *tracker_unique_keys[] = {TRACKER_AKEY_ARTIST, NULL};
        const gchar *aggregate_keys[5] = {0, };
        gchar *aggregate_types[5] = { 0 };

	/* Prepare mafw closure struct */
	mc = g_slice_new(struct _mafw_query_closure);
	mc->callback = callback;
	mc->user_data = user_data;
	mc->asked_keys = keys;
        mc->cache =
                tracker_cache_new(SERVICE_MUSIC,
                                  TRACKER_CACHE_RESULT_TYPE_UNIQUE);

        filters = g_new0(gchar *, 3);

        /* If genre, retrieve all artists for that genre */
        if (genre) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_GENRE,
                        genre,
			TRUE);
                escaped_genre =
                        util_get_tracker_value_for_filter(
                                MTrackerSrc_ID_GENRE,
                                SERVICE_MUSIC,
                                genre);
                filters[0] = g_strdup_printf(RDF_QUERY_BY_GENRE,
                                             escaped_genre);
                g_free(escaped_genre);
                filters[1] = g_strdup(rdf_filter);
        } else {
                filters[0] = g_strdup(rdf_filter);
        }

        /* Artist will be used as title */
        tracker_cache_key_add_derived(mc->cache,
                                      MTrackerSrc_ID_TITLE,
                                      MTrackerSrc_ID_ARTIST);

        /* Insert unique key */
        tracker_cache_key_add_unique(mc->cache, MTrackerSrc_ID_ARTIST);

        tracker_cache_key_add_several(mc->cache, keys, MAXLEVEL);

        /* Concat albums if requested */
        if (tracker_cache_key_exists(mc->cache, MTrackerSrc_ID_ALBUM)) {
                tracker_cache_key_add_concat(mc->cache,
                                             MTrackerSrc_ID_ALBUM);
        }

	/* Create the array for aggregate keys and their types; skip unique
         * key */
	gint id = 0;
	gint tr_idx = 0;
	guint64 keys_tocheck = mc->cache->asked_tracker_keys;
	
	while(keys_tocheck && id <= maxid)
	{
		if ((keys_tocheck & 1) == 1)
		{
			tr_idx++;
			_fill_aggregates(id, 
					aggregate_keys, aggregate_types,
					0, tr_idx, 2);
		}
		id++;
		keys_tocheck >>= 1;
	}

	_do_tracker_get_unique_values(tracker_unique_keys,
                                      (gchar**)aggregate_keys,
                                      aggregate_types,
                                      filters,
                                      offset,
                                      count,
                                      mc);

        g_strfreev(filters);
}

void ti_get_genres(guint64 keys,
		   const gchar *rdf_filter,
		   gchar **sort_fields,
		   guint offset,
		   guint count,
		   MafwTrackerSongsResultCB callback,
		   gpointer user_data)
{
        const int MAXLEVEL = 3;
	struct _mafw_query_closure *mc;
        const gchar *filters[2] = { 0, };
        gchar *tracker_unique_keys[] = {TRACKER_AKEY_GENRE, NULL};
        const gchar *aggregate_keys[6] = { 0 };
        gchar *aggregate_types[6] = { 0 };

	/* Prepare mafw closure struct */
	mc = g_slice_new(struct _mafw_query_closure);
	mc->callback = callback;
	mc->user_data = user_data;
	mc->asked_keys = keys;
        mc->cache =
                tracker_cache_new(SERVICE_MUSIC,
                                  TRACKER_CACHE_RESULT_TYPE_UNIQUE);

        /* Genre will be used as title */
        tracker_cache_key_add_derived(mc->cache,
                                      MTrackerSrc_ID_TITLE,
                                      MTrackerSrc_ID_GENRE);

        /* Insert unique key */
        tracker_cache_key_add_unique(mc->cache, MTrackerSrc_ID_GENRE);

        tracker_cache_key_add_several(mc->cache, keys, MAXLEVEL);

        /* Concat artists if requested */
        if (tracker_cache_key_exists(mc->cache, MTrackerSrc_ID_ARTIST)) {
                tracker_cache_key_add_concat(mc->cache,
                                             MTrackerSrc_ID_ARTIST);
        }

        filters[0] = rdf_filter;

	/* Create the array for aggregate keys and their types; skip unique
         * key */
	gint id = 0;
	gint tr_idx = 0;
	guint64 keys_tocheck = mc->cache->asked_tracker_keys;
	
	while(keys_tocheck && id <= maxid)
	{
		if ((keys_tocheck & 1) == 1)
		{
			tr_idx++;
			_fill_aggregates(id, 
					aggregate_keys, aggregate_types,
					0, tr_idx, 3);
		}
		id++;
		keys_tocheck >>= 1;
	}
	
	/* Query tracker */
	_do_tracker_get_unique_values(tracker_unique_keys,
                                      (gchar**)aggregate_keys,
                                      aggregate_types,
                                      (gchar**)filters,
                                      offset,
                                      count,
                                      mc);

}

void ti_get_playlists(guint64 keys,
		      const gchar *user_filter,
		      gchar **sort_fields,
		      guint offset,
		      guint count,
		      MafwTrackerSongsResultCB callback,
		      gpointer user_data)
{
	struct _mafw_query_closure *mc;
	gchar *rdf_filter = NULL;
        gchar **use_sort_fields;
        gchar **tracker_sort_keys;
	gchar **tracker_keys;

        /* Select default sort fields */
        if (!sort_fields) {
		use_sort_fields = g_new0(gchar *, 2);
		use_sort_fields[0] = MAFW_METADATA_KEY_FILENAME;
        } else {
                use_sort_fields = sort_fields;
        }

	/* Prepare mafw closure struct */
	mc = g_slice_new(struct _mafw_query_closure);
	mc->callback = callback;
	mc->user_data = user_data;
	mc->asked_keys = keys;
        mc->cache = tracker_cache_new(SERVICE_PLAYLISTS,
                                      TRACKER_CACHE_RESULT_TYPE_QUERY);

        /* Add URI, as it will likely be needed to build ids */
        tracker_cache_key_add(mc->cache, MTrackerSrc_ID_URI, 1);

        tracker_cache_key_add_several(mc->cache, keys, 1);

	/* Map to tracker keys */
	tracker_keys = _get_tracker_keys(mc->cache,
							SERVICE_PLAYLISTS);
        tracker_sort_keys =
                keymap_mafw_sort_keys_to_tracker_keys(use_sort_fields,
                                                      SERVICE_MUSIC);
        rdf_filter = util_build_complex_rdf_filter(NULL, user_filter);

	/* Execute query */
        tracker_search_query_async(tc, -1,
                                   SERVICE_PLAYLISTS,
                                   tracker_keys,
                                   NULL,   /* Search text */
                                   NULL,   /* Keywords */
                                   rdf_filter,
                                   offset, count,
                                   FALSE,   /* Sort by service */
                                   tracker_sort_keys, /* Sort fields */
                                   FALSE, /* sort descending? */
                                   _tracker_query_cb,
                                   mc);

        if (use_sort_fields != sort_fields) {
                g_free(use_sort_fields);
        }

	g_free(rdf_filter);
        g_free(tracker_keys);
        g_strfreev(tracker_sort_keys);
}

void ti_get_albums(gchar *genre,
                   gchar *artist,
                   guint64 keys,
                   const gchar *rdf_filter,
                   gchar **sort_fields,
                   guint offset,
                   guint count,
                   MafwTrackerSongsResultCB callback,
                   gpointer user_data)
{
        const int MAXLEVEL = 1;
        gchar *escaped_genre;
        gchar *escaped_artist;
        gchar **filters;
        gchar *tracker_unique_keys[] = {TRACKER_AKEY_ALBUM, NULL};
        const gchar *aggregate_keys[4] = { 0 };
        gchar *aggregate_types[4] = { 0 };
        gint i;
        struct _mafw_query_closure *mc;

	/* Prepare mafw closure struct */
	mc = g_slice_new(struct _mafw_query_closure);
	mc->callback = callback;
 	mc->user_data = user_data;
	mc->asked_keys = keys;
        mc->cache = tracker_cache_new(SERVICE_MUSIC,
                                      TRACKER_CACHE_RESULT_TYPE_UNIQUE);

        filters = g_new0(gchar *, 4);

	/* If genre and/or artist, retrieve all albums for that genre
         * and/or artist */
        i = 0;
        if (genre) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_GENRE,
                        genre,
			TRUE);
                escaped_genre =
                        util_get_tracker_value_for_filter(
                                MTrackerSrc_ID_GENRE,
                                SERVICE_MUSIC,
                                genre);
                filters[i] = g_strdup_printf(RDF_QUERY_BY_GENRE,
                                             escaped_genre);
                g_free(escaped_genre);
                i++;
        }

        if (artist) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_ARTIST,
                        artist,
			TRUE);
                escaped_artist =
                        util_get_tracker_value_for_filter(
                                MTrackerSrc_ID_ARTIST,
                                SERVICE_MUSIC,
                                artist);
                filters[i] = g_strdup_printf(RDF_QUERY_BY_ARTIST,
                                             escaped_artist);
                g_free(escaped_artist);
                i++;
        }

        filters[i] = g_strdup(rdf_filter);

        /* Album will be used as title */
        tracker_cache_key_add_derived(mc->cache,
                                      MTrackerSrc_ID_TITLE,
                                      MTrackerSrc_ID_ALBUM);

        /* Insert unique key */
        tracker_cache_key_add_unique(mc->cache, MTrackerSrc_ID_ALBUM);

        /* Add user keys */
        tracker_cache_key_add_several(mc->cache, keys, MAXLEVEL);

        /* Concat artists, if requested */
        if (!artist &&
            tracker_cache_key_exists(mc->cache, MTrackerSrc_ID_ARTIST)) {
                tracker_cache_key_add_concat(mc->cache,
                                             MTrackerSrc_ID_ARTIST);
        }

	/* Create the array for aggregate keys and their types; skip unique
         * key */
	gint id = 0;
	gint tr_idx = 0;

	guint64 keys_tocheck = mc->cache->asked_tracker_keys;
	
	while(keys_tocheck && id <= maxid)
	{
		if ((keys_tocheck & 1) == 1)
		{
			tr_idx++;
			_fill_aggregates(id, 
					aggregate_keys, aggregate_types,
					0, tr_idx, 0);
		}
		id++;
		keys_tocheck >>= 1;
	}

        _do_tracker_get_unique_values(tracker_unique_keys,
                                      (gchar**)aggregate_keys,
                                      aggregate_types,
                                      filters,
                                      offset,
                                      count,
                                      mc);

        g_strfreev(filters);
}

static void _tracker_metadata_from_container_cb(GPtrArray *tracker_result,
                                                GError *error,
                                                gpointer user_data)
{
        GHashTable *metadata;
        struct _mafw_metadata_closure *mc =
                (struct _mafw_metadata_closure *) user_data;

        if (!error) {
                tracker_cache_values_add_results(mc->cache, tracker_result);
                metadata =
                        tracker_cache_build_metadata_aggregated(
                                mc->cache,
                                mc->count_childcount,
				mc->asked_keys);
                ((MafwTrackerMetadataResultCB)mc->callback)(metadata, NULL, mc->user_data);
        } else {
                ((MafwTrackerMetadataResultCB)mc->callback)(NULL, error, mc->user_data);
        }

        tracker_cache_free(mc->cache);
        g_slice_free(struct _mafw_metadata_closure, mc);
}

static void _get_stats_cb(GPtrArray *result, GError *error, gpointer user_data)
{
        GHashTable *metadata = NULL;
        gchar **item;
        gchar *lookstring;
        gint i;
        struct _mafw_metadata_closure *mc =
                (struct _mafw_metadata_closure *) user_data;

        if (!error) {
                switch (mc->tracker_type) {
                case TRACKER_TYPE_MUSIC:
                        lookstring = "Music";
                        break;

                case TRACKER_TYPE_VIDEO:
                        lookstring = "Videos";
                        break;

                default:
                        lookstring = "Playlists";
                        break;
                }

                i = 0;
                while (i < result->len) {
                        item = g_ptr_array_index(result, i);
                        if (strcmp(item[0], lookstring) == 0) {
                                metadata = mafw_metadata_new();
                                mafw_metadata_add_int(
                                        metadata,
                                        MAFW_METADATA_KEY_CHILDCOUNT_1,
                                        atoi(item[1]));
                                break;
                        } else {
                                i++;
                        }
                }
                ((MafwTrackerMetadataResultCB)mc->callback)(metadata, NULL, mc->user_data);
        } else {
                ((MafwTrackerMetadataResultCB)mc->callback)(NULL, error, mc->user_data);
        }

        g_slice_free(struct _mafw_metadata_closure, mc);
}

static gboolean _run_tracker_metadata_from_container_cb(gpointer data)
{
        GPtrArray *results = g_ptr_array_sized_new(0);

        _tracker_metadata_from_container_cb(results, NULL, data);

        return FALSE;
}

static void _do_tracker_get_metadata_from_service(
        guint64 keys,
        gchar *title,
        enum TrackerObjectType tracker_type,
        MafwTrackerMetadataResultCB callback,
        gpointer user_data)
{
        const gchar *aggregate_types[3] = { 0 };
        const gchar *aggregate_keys[3] = { 0 };
        gchar *sum_key;
        ServiceType service;
        gchar *unique_keys[2] = { 0 };;
        MetadataKey *metadata_key;
	gint id = 0;
	guint64 trkeys;
        gint i = 1;
        struct _mafw_metadata_closure *mc = NULL;

        mc = g_slice_new(struct _mafw_metadata_closure);
        mc->callback = callback;
        mc->user_data = user_data;
        mc->tracker_type = tracker_type;
	mc->asked_keys = keys;
	mc->path_list = NULL;

        /* If user has only requested CHILDCOUNT, then use a special tracker API
         * to speed up the request */
        if (keys == MTrackerSrc_KEY_CHILDCOUNT_1) {
                tracker_get_stats_async(tc, _get_stats_cb, mc);
                return;
        }

        mc->count_childcount = FALSE;
	unique_keys[0] = "File:Mime";

        if (tracker_type == TRACKER_TYPE_MUSIC) {
                sum_key = TRACKER_AKEY_DURATION;
                service = SERVICE_MUSIC;
        } else if (tracker_type == TRACKER_TYPE_VIDEO) {
                sum_key = TRACKER_VKEY_DURATION;
                service = SERVICE_VIDEOS;
        } else {
                sum_key = TRACKER_PKEY_DURATION;
                service = SERVICE_PLAYLISTS;
        }

        mc->cache =
                tracker_cache_new(service, TRACKER_CACHE_RESULT_TYPE_UNIQUE);

        tracker_cache_key_add_unique(mc->cache, MTrackerSrc_ID_MIME);

        if (title) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_TITLE,
                        title,
			FALSE);
        }

        tracker_cache_key_add_several(mc->cache, keys, 1);

	trkeys = mc->cache->asked_tracker_keys;

	while (trkeys && (id <= maxid))
	{
		if ((trkeys & 1) == 1)
		{
			metadata_key = keymap_get_metadata_by_id(id);
			
			if (metadata_key)
				switch (metadata_key->special) {
				case SPECIAL_KEY_DURATION:
					aggregate_keys[i-1] =
						keymap_get_tracker_key_by_id(id,
									       service);
					aggregate_types[i-1] = AGGREGATED_TYPE_SUM;
					break;
		
				case SPECIAL_KEY_CHILDCOUNT:
					aggregate_keys[i-1] = "*";
					aggregate_types[i-1] = AGGREGATED_TYPE_COUNT;
					break;
		
				default:
					aggregate_keys[i-1] =
						keymap_get_tracker_key_by_id(id,
									       service);
					aggregate_types[i-1] = AGGREGATED_TYPE_CONCAT;
					break;
				}
			i++;
		}
		trkeys >>= 1;
		id++;
	}

        if (aggregate_keys[0]) {
                tracker_metadata_get_unique_values_with_aggregates_async(
                        tc,
                        service,
                        unique_keys,
                        NULL,
                        (gchar**)aggregate_types,
                        (gchar**)aggregate_keys,
                        FALSE,
                        0,
                        -1,
                        _tracker_metadata_from_container_cb,
                        mc);
        } else {
                g_idle_add(_run_tracker_metadata_from_container_cb, mc);
        }

}

void ti_get_metadata_from_videos(guint64 keys,
                                 gchar *title,
                                 MafwTrackerMetadataResultCB callback,
                                 gpointer user_data)
{
        _do_tracker_get_metadata_from_service(keys, title, TRACKER_TYPE_VIDEO,
                                              callback, user_data);
}

void ti_get_metadata_from_music(guint64 keys,
                                gchar *title,
                                MafwTrackerMetadataResultCB callback,
                                gpointer user_data)
{
        _do_tracker_get_metadata_from_service(keys, title, TRACKER_TYPE_MUSIC,
                                              callback, user_data);
}

void ti_get_metadata_from_playlists(guint64 keys,
                                    gchar *title,
                                    MafwTrackerMetadataResultCB callback,
                                    gpointer user_data)
{
        _do_tracker_get_metadata_from_service(keys, title,
                                              TRACKER_TYPE_PLAYLIST,
                                              callback, user_data);
}

void ti_get_metadata_from_category(gchar *genre,
                                   gchar *artist,
                                   gchar *album,
                                   gint default_count_key_id,
                                   gchar *title,
                                   guint64 keys,
                                   MafwTrackerMetadataResultCB callback,
                                   gpointer user_data)
{
        gchar *filter;
        gint MAXLEVEL;
        struct _mafw_metadata_closure *mc;
        gint ukey;
        const gchar *tracker_ukeys[2] = { 0 };
        gchar *aggregate_types[7] = { 0 };
        const gchar *aggregate_keys[7] = { 0 };
        gint start_to_look;

        mc = g_slice_new(struct _mafw_metadata_closure);
        mc->callback = callback;
        mc->user_data = user_data;
	mc->asked_keys = keys;
	/* Create cache */
        mc->cache =
                tracker_cache_new(SERVICE_MUSIC,
                                  TRACKER_CACHE_RESULT_TYPE_UNIQUE);
	mc->tracker_type = 0;
	mc->path_list = NULL;

        /* Preset metadata that we know already */
        if (genre) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_GENRE,
                        genre,
			TRUE);
        }

        if (artist) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_ARTIST,
                        artist,
			TRUE);
        }

        if (album) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_ALBUM,
                        album,
			TRUE);
        }

        /* Select the key that will be used as title */
        if (album) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_TITLE,
                        album,
			FALSE);
        } else if (artist) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_TITLE,
                        artist,
			FALSE);
        } else if (genre) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_TITLE,
                        genre,
			FALSE);
        } else if (title) {
                tracker_cache_key_add_precomputed_string(
                        mc->cache,
                        MTrackerSrc_ID_TITLE,
                        title,
			FALSE);
        }

	/* Select unique key to use, and other data */
        if (album) {
                ukey = MTrackerSrc_ID_ALBUM;
                MAXLEVEL = 1;
                start_to_look = 3;
                mc->count_childcount = FALSE;
        } else if (artist) {
                ukey = MTrackerSrc_ID_ARTIST;
                MAXLEVEL = 2;
                start_to_look = 2;
                mc->count_childcount = FALSE;
        } else if (genre) {
                ukey = MTrackerSrc_ID_GENRE;
                MAXLEVEL = 3;
                start_to_look = 1;
                mc->count_childcount = FALSE;
        } else {
                ukey = default_count_key_id;
                mc->count_childcount = TRUE;
                if (default_count_key_id == MTrackerSrc_ID_GENRE) {
                        MAXLEVEL = 4;
                        start_to_look = 0;
                } else if (default_count_key_id == MTrackerSrc_ID_ARTIST) {
                        MAXLEVEL = 3;
                        start_to_look = 1;
                } else if (default_count_key_id == MTrackerSrc_ID_ALBUM) {
                        MAXLEVEL = 2;
                        start_to_look = 2;
                } else {
                        MAXLEVEL = 1;
                        start_to_look = 3;
                }
        }

	/* Add required keys to the cache (beware: order is important) */
        tracker_cache_key_add_unique(mc->cache, ukey);

        tracker_cache_key_add_several(mc->cache, keys, MAXLEVEL);

	/* Check if we have to concat keys */
        if (artist && !album &&
            tracker_cache_key_exists(mc->cache, MTrackerSrc_ID_ALBUM)) {
		/* Concatenate albums if requesting metadata from an artist */
                tracker_cache_key_add_concat(mc->cache,
                                             MTrackerSrc_ID_ALBUM);
        } else if (!artist && album &&
                   tracker_cache_key_exists(mc->cache,
                                            MTrackerSrc_ID_ARTIST)) {
		/* Concatenate artist if requesting metadata from an album */
                tracker_cache_key_add_concat(mc->cache,
                                             MTrackerSrc_ID_ARTIST);
        }

	/* Compute tracker filter and tracker keys */
        filter = util_create_filter_from_category(genre, artist, album, NULL);

        tracker_ukeys[0] = keymap_get_tracker_key_by_id(ukey, SERVICE_MUSIC);

        /* Get the list of keys to use with tracker */

        /* Create the array for aggregate keys and their types; skip unique
         * key */
	gint id = 0;
	gint tr_idx = 0;
	guint64 keys_tocheck = mc->cache->asked_tracker_keys;
	
	while(keys_tocheck && id <= maxid)
	{
		if ((keys_tocheck & 1) == 1)
		{
			tr_idx++;
			_fill_aggregates(id, 
					aggregate_keys, aggregate_types,
					start_to_look, tr_idx, 1);
		}
		id++;
		keys_tocheck >>= 1;
	}

        if (aggregate_keys[0]) {
                tracker_metadata_get_unique_values_with_aggregates_async(
                        tc,
                        SERVICE_MUSIC,
                        (gchar **)tracker_ukeys,
                        filter,
                        aggregate_types,
                        (gchar**)aggregate_keys,
                        FALSE,
                        0,
                        -1,
                        _tracker_metadata_from_container_cb,
                        mc);
        } else {
                g_idle_add(_run_tracker_metadata_from_container_cb, mc);
        }

        g_free(filter);
}


void ti_get_metadata_from_videoclip(gchar **uris,
                                    guint64 keys,
                                    MafwTrackerMetadatasResultCB callback,
                                    gpointer user_data)
{
        _do_tracker_get_metadata(uris, keys, TRACKER_TYPE_VIDEO,
				 callback, user_data);
}

void ti_get_metadata_from_audioclip(gchar **uris,
                                    guint64 keys,
                                    MafwTrackerMetadatasResultCB callback,
                                    gpointer user_data)
{
        _do_tracker_get_metadata(uris, keys, TRACKER_TYPE_MUSIC,
				 callback, user_data);
}

void ti_get_metadata_from_playlist(gchar **uris,
				   guint64 keys,
				   MafwTrackerMetadatasResultCB callback,
				   gpointer user_data)
{
        _do_tracker_get_metadata(uris, keys, TRACKER_TYPE_PLAYLIST,
				 callback, user_data);
}

gchar **
ti_set_metadata(const gchar *uri, GHashTable *metadata, CategoryType category, gboolean *updated)
{
        GList *keys = NULL;
        GList *running_key;
        gint count_keys;
        gint i_key, u_key;
        const gchar **keys_array = NULL;
        gchar **values_array = NULL;;
        gchar **unsupported_array = NULL;
        gchar *mafw_key;
        gchar *pathname;
        GError *error = NULL;
        GValue *value;
        gboolean updatable;
        ServiceType service;
        const TrackerKey *tracker_key;

	/* We have not updated anything yet */
	if (updated) {
		*updated = FALSE;
	}

        /* Get list of keys */
        keys = g_hash_table_get_keys(metadata);

        count_keys = g_list_length(keys);
        keys_array = g_new0(const gchar *, count_keys+1);
        values_array = g_new0(gchar *, count_keys+1);

	if (category == CATEGORY_VIDEO)
	{
		service = SERVICE_VIDEOS;	
	}
	else
	{
		service = SERVICE_MUSIC;
	}

        /* Convert lists to arrays */
        running_key = keys;
        i_key = 0;
        u_key = 0;
        while (running_key) {
                mafw_key = (gchar *) running_key->data;
                updatable = TRUE;
                /* Get only supported keys, and convert values to
                 * strings */
                if (keymap_mafw_key_is_writable(mafw_key)) {
			gint mid = keymap_get_id_from_mafwkey(mafw_key);
			guint64 mflag = keymap_get_flag_from_keyid(mid);
                        /* Special case: some keys should follow ISO-8601
                         * spec */
                        tracker_key = keymap_get_tracker_info_by_id(mid,
                                                              service);
                        if (mflag & MTrackerSrc_Date_Types) {
                                value = mafw_metadata_first(metadata,
                                                             mafw_key);
                                if (value && G_VALUE_HOLDS_LONG(value)) {
                                        values_array[i_key] =
					  util_epoch_to_iso8601(
					    g_value_get_long(value));
                                } else {
                                        updatable = FALSE;
                                }
                        } else {
                                value = mafw_metadata_first(metadata,
                                                             mafw_key);
				if (value)
                                	switch (G_VALUE_TYPE(value)) {
                                		case G_TYPE_STRING:
                                        		values_array[i_key] =
                                                	g_value_dup_string(value);
                                        	break;
                                	default:
                                        	values_array[i_key] =
                                                	g_strdup_value_contents(value);
                                        	break;
                                	}

                        }
                } else {
                        updatable = FALSE;
                }

                if (updatable) {
                        keys_array[i_key] =
				keymap_mafw_key_to_tracker_key(mafw_key,
							       service);
                        i_key++;
                } else {
                        if (!unsupported_array) {
                                unsupported_array = g_new0(gchar *,
                                                           count_keys+1);
                        }
                        unsupported_array[u_key] = g_strdup(mafw_key);
                        u_key++;
                }

                running_key = g_list_next(running_key);
        }

        g_list_free(keys);

	/* If there are some updatable keys, call tracker to update them */
	if (keys_array[0] != NULL) {
                pathname = g_filename_from_uri(uri, NULL, NULL);
		tracker_metadata_set(tc, service, pathname,
                                     (const gchar **)keys_array,
                                     values_array, &error);
                g_free(pathname);

		if (error) {
			/* Tracker_metadata_set is an atomic operation; so
			 * none of the keys were updated */
			i_key = 0;
			while (keys_array[i_key]) {
				if (!unsupported_array) {
					unsupported_array =
						g_new0(gchar *, count_keys+1);
				}
				unsupported_array[u_key] =
					g_strdup(keys_array[i_key]);
				i_key++;
				u_key++;
			}

			g_error_free(error);
		} else if (updated) {
			/* We successfully updated some keys at least */
			*updated = TRUE;
		}
	}

        g_free(keys_array);
        g_strfreev(values_array);

        return unsupported_array;
}


static void _set_playlist_duration_cb(GError *error, gpointer user_data)
{
	if (error != NULL) {
		g_warning("Error while setting the playlist duration: "
			  "%s", error->message);
	}
}


void ti_set_playlist_duration(const gchar *uri, guint duration)
{
	static const gchar *keys_array[3] = { TRACKER_PKEY_DURATION,
					TRACKER_PKEY_VALID_DURATION, NULL};
	gchar *values_array[3] = { 0, };;
	gchar *pathname;

	/* Store in Tracker the new value for the playlist duration and set the
	   valid_duration value to TRUE. */
	pathname = g_filename_from_uri(uri, NULL, NULL);


	values_array[0] = g_strdup_printf("%d", duration);
	values_array[1] = g_strdup_printf("%d", 1);

	tracker_metadata_set_async(tc,
				   SERVICE_PLAYLISTS,
				   pathname,
				   keys_array,
				   values_array,
				   _set_playlist_duration_cb,
				   NULL);

	g_free(pathname);
        g_free(values_array[0]);
	g_free(values_array[1]);
}
