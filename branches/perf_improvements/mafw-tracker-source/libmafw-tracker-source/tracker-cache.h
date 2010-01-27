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

#ifndef __MAFW_TRACKER_CACHE_H__
#define __MAFW_TRACKER_CACHE_H__

#include <glib.h>
#include <glib-object.h>
#include <tracker.h>

/* How results from tracker where obtained */
enum TrackerCacheResultType {
        /* Results were obtained with tracker_search_query_async */
        TRACKER_CACHE_RESULT_TYPE_QUERY,
        /* Results were obtained with
         * tracker_metadata_get_unique_values_with_aggregated */
        TRACKER_CACHE_RESULT_TYPE_UNIQUE,
        /* Results were obtained with tracker_get_metadata */
        TRACKER_CACHE_RESULT_TYPE_GET_METADATA,
};

/* The value of the cached key */
typedef struct TrackerCacheValue {
	/* If computed, the type of the stored value */
	GType value_type;
	union {
		gchar *str;
		gint i;
	};
	gboolean free_str;
	/* Derived from other key */
	gint key_derived_from_id;
} TrackerCacheValue;

/* The cache where to store the values */
typedef struct TrackerCache {
        /* How results from tracker have been obtained */
        enum TrackerCacheResultType result_type;
        /* The service used with tracker */
        ServiceType service;
        /* Values returned by tracker */
        GPtrArray *tracker_results;
        /* The list of keys */
        TrackerCacheValue *cache[64];
	
	guint64 asked_tracker_keys;
	gint asked_uniq_id;
} TrackerCache;


TrackerCache *tracker_cache_new(ServiceType service,
                                enum TrackerCacheResultType result_type);

void tracker_cache_free(TrackerCache *cache);

void tracker_cache_key_add_precomputed_string(TrackerCache *cache,
                                              gint key,
                                              gchar *value,
					      gboolean free_str);

void tracker_cache_key_add_precomputed_int(TrackerCache *cache,
                                           gint key,
                                           gint value);

void tracker_cache_key_add_derived(TrackerCache *cache,
                                   gint key,
                                   gint source_key);

void tracker_cache_key_add(TrackerCache *cache,
                           gint key_id,
                           gint maximum_level);

void tracker_cache_key_add_several(TrackerCache *cache,
                                   guint64 keys,
                                   gint max_level);

void tracker_cache_key_add_unique(TrackerCache *cache,
                                  gint unique_key);

void tracker_cache_key_add_concat(TrackerCache *cache,
                                  gint concat_key);

void tracker_cache_values_add_results(TrackerCache *cache,
                                      GPtrArray *tracker_results);

const GPtrArray *tracker_cache_values_get_results(TrackerCache *cache);


GList *tracker_cache_build_metadata(TrackerCache *cache, guint64 asked_keys,
					const gchar **path_list);

GHashTable *tracker_cache_build_metadata_aggregated(TrackerCache *cache,
                                                    gboolean count_childcount,
						    guint64 asked_keys);

gboolean tracker_cache_key_exists(TrackerCache *cache,
                                  gint key);
gchar*
tracker_cache_value_get_str(TrackerCache *cache,
                        gint key_id,
                        gint index,
			gint tracker_index,
			gboolean *should_be_freed);
gint
tracker_cache_value_get_int(TrackerCache *cache,
                        gint key_id,
                        gint index,
			gint tracker_index);
#endif /* __MAFW_TRACKER_CACHE_H__ */
