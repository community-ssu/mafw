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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libmafw/mafw.h>
#include "key-mapping.h"
#include "tracker-cache.h"
#include "album-art.h"
#include "util.h"

#define SEVERAL_VALUES_DELIMITER '|'
#define SEVERAL_VALUES_DELIMITER_STR "|"

extern gint maxid;

/* ------------------------- Private API ------------------------- */

static void _replace_various_str_values(gchar **str_value, gboolean free_str_if_needed)
{
	/* Find for separator */
	if (str_value && *str_value &&
		strchr(*str_value, SEVERAL_VALUES_DELIMITER)) {
			if (free_str_if_needed)
			{
				g_free(*str_value);
				*str_value = g_strdup(MAFW_METADATA_VALUE_VARIOUS_VALUES);
			}
			else
			{
				*str_value = MAFW_METADATA_VALUE_VARIOUS_VALUES;
			}
	}
}

static gboolean _value_str_is_allowed(const gchar *str_val, gint key_id)
{
        MetadataKey *metadata_key;

        if (!str_val) {
                return FALSE;
        }

        metadata_key = keymap_get_metadata_by_id(key_id);

        if (!metadata_key) {
                return FALSE;
        }

        if (metadata_key->allowed_empty) {
                return TRUE;
        } else {
                /* Check if value is empty */
		return !IS_STRING_EMPTY(str_val);
        }
}
static gint _get_idx_from_id(TrackerCache *cache, gint key_id)
{
	gint idx = -1;
	gint curid = 0;
	guint64 keys = cache->asked_tracker_keys;
	
	if (cache->result_type == TRACKER_CACHE_RESULT_TYPE_UNIQUE)
	{
		idx++;
	}
	if (cache->result_type == TRACKER_CACHE_RESULT_TYPE_QUERY)
	{
		idx += 2;
	}
	while (keys)
	{
		if ((keys & 1) == 1)
		{
			idx++;
			if (curid == key_id)
				return idx;
		}
		curid++;
		if (key_id < curid)
			return -1;
		keys >>=1;
	}
	return -1;
}
static gchar *_get_title_str(TrackerCache *cache, gint index, gint tracker_index,
				const gchar *path, gboolean *should_be_freed)
{
        gchar *str_title;
        gchar *uri_title;
        gchar *filename;
        gchar *pathname;
        gchar *dot;

        str_title = tracker_cache_value_get_str(cache,
                                              MTrackerSrc_ID_TITLE,
                                              index,
		tracker_index,
		should_be_freed);

        /* If it is empty, then use the URI */
        if (IS_STRING_EMPTY(str_title) &&
            cache->result_type != TRACKER_CACHE_RESULT_TYPE_UNIQUE) {
		gboolean uri_to_free;
                uri_title = tracker_cache_value_get_str(cache,
                                                MTrackerSrc_ID_URI,
                                                index, 0, &uri_to_free);
                if (!uri_title) {
                        return str_title;
                }

                if (IS_STRING_EMPTY(uri_title)) {
			if (IS_STRING_EMPTY(path))
			{
				if (uri_to_free)
					g_free(uri_title);
                        	return str_title;
			}
			else
			{
				pathname = g_strdup(path);
			}
                }
		else
	                pathname = g_filename_from_uri(uri_title, NULL, NULL);

                /* Get filename */
                filename = g_path_get_basename(pathname);

                /* Remove extension */
                dot = g_strrstr(filename, ".");
                if (dot) {
                        *dot = '\0';
                }

                /* Use filename as the value */
                g_free(pathname);

		if (*should_be_freed)
			g_free(str_title);
		if (uri_to_free)
			g_free(uri_title);
		*should_be_freed = TRUE;
                return filename;
        }
	return str_title;
}

/* Inserts a key in the cache. 'pos' only makes sense when type is
 * TRACKER_CACHE_KEY_TYPE_TRACKER */
static void _insert_key(TrackerCache *cache,
                        gint key_id)
{
        cache->asked_tracker_keys |= keymap_get_flag_from_keyid(key_id);
}

static void _insert_unique_key(TrackerCache *cache,
                        gint key_id)
{
        cache->asked_uniq_id = key_id;
}


static gchar *_get_value_album_art_str(TrackerCache *cache, gint index)
{
	gchar *album_str;
	gchar **singles;
	gchar *album_art_uri;
	gboolean free_str;

	album_str = tracker_cache_value_get_str(cache,
                                              MTrackerSrc_ID_ALBUM,
                                              index,
		_get_idx_from_id(cache, MTrackerSrc_ID_ALBUM),
		&free_str);
	
	if (IS_STRING_EMPTY(album_str)) {
		if (free_str)
			g_free(album_str);
                return NULL;
        }

	/* As album can be actually several albums, split them and
         * show the first available cover */
        singles = g_strsplit(album_str, SEVERAL_VALUES_DELIMITER_STR, 0);

        gint i=0;
        album_art_uri = NULL;
        while (singles[i] && album_art_uri == NULL) {
                album_art_uri = albumart_get_album_art_uri(singles[i]);
                i++;
        }
        g_strfreev(singles);

	if (free_str)
		g_free(album_str);
	return album_art_uri;
}

static gchar *_get_value_thumbnail_str(TrackerCache *cache,
                                    gint key_id,
                                    gint index)
{
	gchar *uri;
	enum thumbnail_size size;
	gchar *th_uri;
	gboolean free_uri;

	/* For thumbnails, uri is needed */
        if (albumart_key_is_thumbnail_by_id(key_id)) {
                uri = tracker_cache_value_get_str(cache,
                                                    MTrackerSrc_ID_URI,
                                                    index, 0, &free_uri);
        } else {
                /* In case of album-art-large-uri, album-art is used */
                if (key_id == MTrackerSrc_ID_ALBUM_ART_LARGE_URI) {
                        return _get_value_album_art_str(cache, index);
                } else {
                        uri =
                                tracker_cache_value_get_str(
                                        cache,
                                        MTrackerSrc_ID_ALBUM_ART_URI,
                                        index,
					-1, /* Not used */
					&free_uri);
                }
        }

        if (uri) {
                /* Compute size requested */
                if (key_id == MTrackerSrc_ID_ALBUM_ART_SMALL_URI ||
                    key_id == MTrackerSrc_ID_ALBUM_ART_MEDIUM_URI ||
                    albumart_key_is_thumbnail_by_id(key_id)) {
                        size = THUMBNAIL_CROPPED;
                } else {
                        size = THUMBNAIL_NORMAL;
                }
                th_uri = albumart_get_thumbnail_uri(uri, size);

		if (free_uri)		
                	g_free(uri);
                return th_uri;
        }
	return NULL;
}

static gint _aggregate_key(TrackerCache *cache,
                               gint key_id,
                               gboolean count_childcount,
			       gint tracker_index)
{
        gint total = 0;
        gint value;
        gint i;
        gint results_length;

        results_length = cache->tracker_results? cache->tracker_results->len: 0;

        if (count_childcount &&
            key_id ==  MTrackerSrc_ID_CHILDCOUNT_1) {
                total = results_length;
        } else {
                for (i=0; i < results_length; i++) {
                        value = tracker_cache_value_get_int(cache, key_id, i, tracker_index);
                        if (value != 0 && value != G_MAXINT) {
                                total += value;
                        }
                }
        }

        return total;
}

/* ------------------------- Public API ------------------------- */

/*
 * tracker_cache_new:
 * @service: service that will be used in tracker
 * @result_type: type of query will be used in tracker.
 *
 * Creates a new cache.
 *
 * Returns: a new cache
 **/
TrackerCache *
tracker_cache_new(ServiceType service,
                  enum TrackerCacheResultType result_type)
{

        TrackerCache *cache;

        cache = g_slice_new(TrackerCache);
        cache->service = service;
        cache->result_type = result_type;
	memset(cache->cache, 0, sizeof(cache->cache));
	cache->tracker_results = NULL;
	cache->asked_tracker_keys = 0;
	cache->asked_uniq_id = G_MAXINT;
        return cache;
}

static void tracker_cache_value_free(TrackerCacheValue *value)
{
	if (value->key_derived_from_id == G_MAXINT &&
		value->value_type == G_TYPE_STRING && value->free_str)
		g_free(value->str);
	g_slice_free(TrackerCacheValue, value);
}
/*
 * tracker_cache_free:
 * @cache: cache to be freed
 *
 * Frees the cache and its contents
 */
void
tracker_cache_free(TrackerCache *cache)
{
	gint i = 0;

        /* Free tracker results */
        if (cache->tracker_results) {
                g_ptr_array_foreach(cache->tracker_results,
                                    (GFunc) g_strfreev,
                                    NULL);
                g_ptr_array_free(cache->tracker_results, TRUE);
        }

	for (i=0; i < 64; i++)
	{
		if (cache->cache[i])
			tracker_cache_value_free(cache->cache[i]);
	}
	

        /* Free the cache itself */
        g_slice_free(TrackerCache, cache);
}

/*
 * tracker_cache_key_add_precomputed_string:
 * @cache: the cache
 * @key: the key to be inserted
 * @value: the value to be inserted
 *
 * Inserts a key with its value in the cache. If the key already
 * exists, it does nothing.
 */
void
tracker_cache_key_add_precomputed_string(TrackerCache *cache,
                                         gint key_id,
                                         gchar *value,
					 gboolean free_str)
{/* TODO: value shouldn't be const, caller should not free the value, but when trackercache frees */
        TrackerCacheValue *cached_value;

        /* Look if the key already exists */
        if (!cache->cache[key_id]) {
                /* Create the value to be cached */
                cached_value = g_slice_new(TrackerCacheValue);
		cached_value->str = value;
		cached_value->free_str = free_str;
		cached_value->value_type = G_TYPE_STRING;
		cached_value->key_derived_from_id = G_MAXINT;
                cache->cache[key_id] = cached_value;
        }
}

/*
 * tracker_cache_key_add_precomputed_int:
 * @cache: the cache
 * @key: the key to be inserted
 * @user_key: @TRUE if this key has been requested by the user
 * @value: the value to be inserted
 *
 * Inserts a key with its value in the cache. If the key already
 * exists, it does nothing.
 */
void
tracker_cache_key_add_precomputed_int(TrackerCache *cache,
                                      gint key_id,
                                      gint value)
{
	TrackerCacheValue *cached_value;

        /* Look if the key already exists */
        if (!cache->cache[key_id]) {
                /* Create the value to be cached */
                cached_value = g_slice_new(TrackerCacheValue);
		cached_value->i = value;
		cached_value->value_type = G_TYPE_INT;
		cached_value->key_derived_from_id = G_MAXINT;
                cache->cache[key_id] = cached_value;
        }
}

/*
 * tracker_cache_key_add_derived:
 * @cache: the cache
 * @key: the key to be inserted
 * @user_key: @TRUE if this key has been requested by user
 * @source_key: the key which will be used to get the real value
 *
 * Inserts a key which value will be obtained from other key. If the
 * key already exists, it does nothing.
 */
void
tracker_cache_key_add_derived(TrackerCache *cache,
                              gint key_id,
                              gint source_key)
{
	TrackerCacheValue *cached_value;

        /* Look if the key already exists */
        if (!cache->cache[key_id]) {
                /* Create the value to be cached */
                cached_value = g_slice_new(TrackerCacheValue);
		cached_value->key_derived_from_id = source_key;
		cached_value->value_type = 0;
                cache->cache[key_id] = cached_value;
        }
}

/*
 * tracker_cache_key_add:
 * @cache: the cache
 * @key: the key to be inserted
 * @maximum_level: maxium level allowed for childcount keys
 * @user_key: @TRUE if the user has requested this key
 *
 * Inserts in the cache a new key. If the key exists, and the new one
 * is a user's key, then mark the old one as requested by user.
 * Notice that depending on the service and query type used to create
 * the cache, some keys could be discarded. Keys not supported in
 * tracker will be discarded.
 */
void
tracker_cache_key_add(TrackerCache *cache,
                      gint key_id,
                      gint maximum_level)
{
        gint level;
        MetadataKey *metadata_key;
	guint64 key_flag = keymap_get_flag_from_keyid(key_id);
	

        /* Look if the key already exists */
        if (key_flag == 0 ||
		((cache->asked_tracker_keys & key_flag) == key_flag)) {
                return;
        }

        metadata_key = keymap_get_metadata_by_id(key_id);

        /* Discard unsupported keys */
        if (!metadata_key) {
                return;
        }

        /* Insert dependencies */
        if (metadata_key->depends_on_id != G_MAXINT) {
                tracker_cache_key_add(cache, metadata_key->depends_on_id,
                                      maximum_level);
        }

        /* With childcount, check that fits in the allowed range */
        if (metadata_key->special == SPECIAL_KEY_CHILDCOUNT) {
                level = keymap_get_childcount_level(key_id);
                if (level < 1 || level > maximum_level) {
                        return;
                }
        }

        /* Within the current service, check if the key makes sense (CHILDCOUNT
         * always makes sense */
        if (metadata_key->special != SPECIAL_KEY_CHILDCOUNT &&
            keymap_get_tracker_info_by_id(key_id, cache->service) == NULL) {
                return;
        }

        /* With unique, only duration, childcount and mime keys
         * makes sense */
        if ((cache->result_type == TRACKER_CACHE_RESULT_TYPE_UNIQUE) &&
            metadata_key->special != SPECIAL_KEY_CHILDCOUNT &&
            metadata_key->special != SPECIAL_KEY_DURATION &&
            metadata_key->special != SPECIAL_KEY_MIME  &&
            metadata_key->key_type != ALBUM_ART_KEY) {
                return;
        }


        /* Childcount is 0 for all clips, unless playlists */
        if (cache->result_type != TRACKER_CACHE_RESULT_TYPE_UNIQUE &&
            metadata_key->special == SPECIAL_KEY_CHILDCOUNT &&
            cache->service != SERVICE_PLAYLISTS) {
                tracker_cache_key_add_precomputed_int(cache, key_id, 0);
                return;
        }

        /* MIME for playlists and non-leaf nodes are always 'container' */
        if (metadata_key->special == SPECIAL_KEY_MIME &&
            (cache->service == SERVICE_PLAYLISTS ||
             cache->result_type == TRACKER_CACHE_RESULT_TYPE_UNIQUE)) {
                tracker_cache_key_add_precomputed_string(
                        cache,
                        key_id,
                        MAFW_METADATA_VALUE_MIME_CONTAINER,
		     	FALSE);
                return;
        }

        /* In case of title and non-unique, ask also for URI, as it
         * could be used as title just if there it doesn't have one */
        if (cache->result_type != TRACKER_CACHE_RESULT_TYPE_UNIQUE &&
            metadata_key->special == SPECIAL_KEY_TITLE) {
                tracker_cache_key_add(cache, MTrackerSrc_ID_URI,
                                      maximum_level);
        }

        _insert_key(cache, key_id);

        return;
}

/*
 * tracker_cache_key_add_several:
 * @cache: the cache
 * @keys: NULL-ending array of keys
 * @max_level: maximum level allowed for childcount
 * @user_keys: @TRUE if the user has requested these keys
 *
 * Inserts in the cache the keys specified. See @tracker_cache_key_add
 * for more information.
 */
void
tracker_cache_key_add_several(TrackerCache *cache,
                              guint64 keys,
                              gint max_level)
{
        gint id = 0;

	while (keys && id <= maxid)
	{
		if ((keys & 1) == 1)
		{
			tracker_cache_key_add(cache, id, max_level);
		}
		keys >>= 1;
		id++;
	}
}

/*
 * tracker_cache_key_add_unique:
 * @cache: tracker cache
 * @unique_keys: the unique key
 *
 * Add a key that will be used to query tracker with 'unique' functions. If some
 * the key already exists, it is skipped. Warning! This function only works if
 * the case was created with TRACKER_CACHE_RESULT_TYPE_UNIQUE.
 */
void
tracker_cache_key_add_unique(TrackerCache *cache,
                             gint unique_key)
{
        MetadataKey *metadata_key;

        /* Check if cache was created to store data from
         * unique_count_sum functions */
        g_return_if_fail(
                cache->result_type == TRACKER_CACHE_RESULT_TYPE_UNIQUE);

        metadata_key = keymap_get_metadata_by_id(unique_key);

        /* Skip unsupported keys */
        if (metadata_key) {
		/* Special case: 'unique' functions are used
		 * when processing containers. In this case,
		 * mime type of a container must be always
		 * @MAFW_METADATA_VALUE_MIME_CONTAINER, no
		 * matter the result tracker returns. */
		if (metadata_key->special == SPECIAL_KEY_MIME) {
			tracker_cache_key_add_precomputed_string(
				cache,
				unique_key,
				MAFW_METADATA_VALUE_MIME_CONTAINER,
				FALSE);
		} else {
			_insert_unique_key(
				cache,
				unique_key);
		}
        }
}

/*
 * tracker_cache_key_add_concat:
 * @cache: tracker cache
 * @concat_key: key that will be concatenated
 *
 * Add the key to be concatenated with 'unique_concat'
 * function. Warning! This function only works if the cache was
 * created with TRACKER_CACHE_RESULT_TYPE_UNIQUE.
 */
void
tracker_cache_key_add_concat(TrackerCache *cache,
                             gint concat_key_id)
{
        g_return_if_fail(cache->result_type ==
                         TRACKER_CACHE_RESULT_TYPE_UNIQUE);

        _insert_key(cache, concat_key_id);

}

/*
 * tracker_cache_values_add_results:
 * @cache: tracker cache
 * @tracker_results: results returned by tracker.
 *
 * Adds to the cache the results returned by a tracker query.
 */
void
tracker_cache_values_add_results(TrackerCache *cache,
                                 GPtrArray *tracker_results)
{
        cache->tracker_results = tracker_results;
}

/*
 * tracker_cache_values_get_results:
 * @cache: tracker cache
 *
 * Returns the results obtained by tracker.
 *
 * Returns: results from tracker.
 */
const GPtrArray *
tracker_cache_values_get_results(TrackerCache *cache)
{
        return cache->tracker_results;
}

static gchar *_get_tr_val(TrackerCache *cache, gint index,
			gint tracker_index)
{
	gchar **queried_result = NULL;

	/* Check if the value can be obtained from tracker */
	if (index < 0 || tracker_index < 0) {
		return NULL;
	}

	/* Verify there is data, and index is within the range */
	if (!cache->tracker_results || cache->tracker_results->len <= index) {
		return NULL;
	}

	queried_result = (gchar **) g_ptr_array_index(cache->tracker_results,
							index);

	/* Verify that tracked found the metadata for the corresponding
	* entry */
	if (!queried_result[0]) {
		return NULL;
	}
	return queried_result[tracker_index];
}

gchar*
tracker_cache_value_get_str(TrackerCache *cache,
                        gint key_id,
                        gint index,
			gint tracker_index,
			gboolean *should_be_freed)
{
	MetadataKey *metadata_key;
	gchar *trval;

	if (cache->cache[key_id])
	{
		if (cache->cache[key_id]->key_derived_from_id == G_MAXINT &&
			cache->cache[key_id]->str)
		{
			*should_be_freed = FALSE;
			return (gchar*)cache->cache[key_id]->str;
		}
		if (cache->cache[key_id]->key_derived_from_id != G_MAXINT)
			return tracker_cache_value_get_str(cache,
					cache->cache[key_id]->key_derived_from_id,
					index,
					_get_idx_from_id(cache,
						cache->cache[key_id]->key_derived_from_id),
					should_be_freed);
	}
	
	metadata_key = keymap_get_metadata_by_id(key_id);

	if (!metadata_key)
		return NULL;

	if (metadata_key->key_type == ALBUM_ART_KEY ||
		metadata_key->key_type == THUMBNAIL_KEY)
	{
		if (key_id == MTrackerSrc_ID_ALBUM_ART_URI) {
			*should_be_freed = TRUE;
                        return _get_value_album_art_str(cache, index);
                } else {
			*should_be_freed = TRUE;
                        return _get_value_thumbnail_str(cache, key_id, index);
                }
	}

	if (cache->result_type == TRACKER_CACHE_RESULT_TYPE_UNIQUE &&
		key_id == cache->asked_uniq_id)
	{
		tracker_index = 0;
	}
	trval = _get_tr_val(cache, index, tracker_index);
	/* If the value must be obtained from tracker */

	if (metadata_key->special == SPECIAL_KEY_URI)
	{
		if (trval)
		{
			gchar *uri;
			*should_be_freed = TRUE;
			uri = g_filename_to_uri(trval, NULL, NULL);
			return uri;
		}
		return NULL;
	}
	*should_be_freed = FALSE;
	return trval;
}

gint
tracker_cache_value_get_int(TrackerCache *cache,
                        gint key_id,
                        gint index,
			gint tracker_index)
{
	gchar *trval = NULL;

	if (cache->cache[key_id])
	{
		if (cache->cache[key_id]->value_type == G_TYPE_INT)
			return cache->cache[key_id]->i;
		if (cache->cache[key_id]->key_derived_from_id != G_MAXINT)
			return tracker_cache_value_get_int(cache,
					cache->cache[key_id]->key_derived_from_id,
					index,
					_get_idx_from_id(cache,
						cache->cache[key_id]->key_derived_from_id));
	}

	/* If the value must be obtained from tracker */
	trval = _get_tr_val(cache, index, tracker_index);

	if (trval)
		return atoi(trval);
	return G_MAXINT;
}

static glong
tracker_cache_value_get_long(TrackerCache *cache,
                        gint key_id,
                        gint index,
			gint tracker_index)
{
	gchar *trval = NULL;

	if (cache->cache[key_id])
	{
		if (cache->cache[key_id]->key_derived_from_id != G_MAXINT)
			return tracker_cache_value_get_long(cache,
					cache->cache[key_id]->key_derived_from_id,
					index,
					_get_idx_from_id(cache,
						cache->cache[key_id]->key_derived_from_id));
	}

	/* If the value must be obtained from tracker */

	trval = _get_tr_val(cache, index, tracker_index);

	if (trval)
		return atol(trval);
	return 0;
}

static gfloat
tracker_cache_value_get_float(TrackerCache *cache,
                        gint key_id,
                        gint index,
			gint tracker_index)
{
	float float_val = 0;
	gchar *trval = NULL;


	if (cache->cache[key_id])
	{
		if (cache->cache[key_id]->key_derived_from_id != G_MAXINT)
			return tracker_cache_value_get_float(cache,
					cache->cache[key_id]->key_derived_from_id,
					index,
					_get_idx_from_id(cache,
						cache->cache[key_id]->key_derived_from_id));
	}

	/* If the value must be obtained from tracker */
	trval = _get_tr_val(cache, index, tracker_index);

	if (trval)
	{
		sscanf(trval,
                               "%f",
                               &float_val);
		return float_val;
	}
	return 0;
}

static gboolean
tracker_cache_value_get_boolean(TrackerCache *cache,
                        gint key_id,
                        gint index,
			gint tracker_index)
{
	gchar *trval = NULL;

	if (cache->cache[key_id])
	{
		if (cache->cache[key_id]->key_derived_from_id != G_MAXINT)
			return tracker_cache_value_get_boolean(cache,
					cache->cache[key_id]->key_derived_from_id,
					index,
					_get_idx_from_id(cache,
						cache->cache[key_id]->key_derived_from_id));
	}

	/* If the value must be obtained from tracker */
	trval = _get_tr_val(cache, index, tracker_index);

	if (trval)
		return trval[0] == '0';
	return FALSE;
}


static void _parse_tracker_value_add_to_mdata(GHashTable *metadata,
				TrackerCache *cache,
				const MetadataKey *metadata_key, gint id,
				gint idx,
				gint tracker_idx)
{
	if (!metadata_key)
		return;
	switch (metadata_key->value_type)
	{
		case G_TYPE_INT:
		{
			gint intval = tracker_cache_value_get_int(cache, id, idx, tracker_idx);
			if (intval != G_MAXINT &&
				(metadata_key->allowed_empty || intval > 0)) {
				mafw_metadata_add_int(metadata,
					keymap_get_mafwkey_from_keyid(id),
					intval);
			}
			break;
		}
		case G_TYPE_LONG:
		{
			glong longval = tracker_cache_value_get_long(cache, id, idx, tracker_idx);
			if (metadata_key->allowed_empty || longval > 0) {
				mafw_metadata_add_long(metadata,
					keymap_get_mafwkey_from_keyid(id),
					longval);
			}
			break;
		}
		case G_TYPE_FLOAT:
		{
			gfloat floatval = tracker_cache_value_get_float(cache, id, idx, tracker_idx);
			if (metadata_key->allowed_empty || floatval > 0.0) {
				mafw_metadata_add_long(metadata,
					keymap_get_mafwkey_from_keyid(id),
					floatval);
			}
			break;
		}
		case G_TYPE_STRING:
		{
			gboolean be_free = FALSE;
			gchar *value_str = tracker_cache_value_get_str(cache,
						id, idx, tracker_idx, &be_free);
			if (_value_str_is_allowed(value_str, id)) {
				_replace_various_str_values(&value_str, be_free);
				mafw_metadata_add_str(metadata,
					keymap_get_mafwkey_from_keyid(id),
					value_str);
			}
			if (be_free)
				g_free(value_str);
			break;
		}
		case G_TYPE_BOOLEAN:
		{
			gboolean boolval = tracker_cache_value_get_boolean(cache, id, idx, tracker_idx);
			mafw_metadata_add_boolean(metadata,
					keymap_get_mafwkey_from_keyid(id),
					boolval);
			break;
		}
	}
	
}

/*
 * tracker_cache_build_metadata:
 * @cache: tracker cache
 *
 * Builds a list of MAFW-metadata from cached results.
 *
 * Returns: list of MAFW-metadata
 */
GList *
tracker_cache_build_metadata(TrackerCache *cache, guint64 asked_keys, const gchar **path_list)
{
        GList *mafw_list = NULL;
        gint result_index;
        gint requested_metadatas;
        GHashTable *metadata = NULL;

        /* If there aren't results from tracker, there is even a chance of being
         * able to build metadata with precomputed values */
        if (!cache->tracker_results || cache->tracker_results->len == 0) {
                requested_metadatas = 1;
        } else {
                requested_metadatas = cache->tracker_results->len;
        }

        /* Create metadata */
        for (result_index = 0; result_index < requested_metadatas; result_index++)
	{
		metadata = mafw_metadata_new();
		gint id = 0;
		gint tracker_idx = -1, cur_tr_idx;
		guint64 keys = asked_keys;
		guint64 tr_keys = cache->asked_tracker_keys;
		gchar *value_str;
		MetadataKey *metadata_key;

		if (cache->result_type == TRACKER_CACHE_RESULT_TYPE_QUERY)
		{
			tracker_idx += 2;
		}
		else if (cache->result_type == TRACKER_CACHE_RESULT_TYPE_UNIQUE)
		{
			tracker_idx++;
			metadata_key = keymap_get_metadata_by_id(cache->asked_uniq_id);
			_parse_tracker_value_add_to_mdata(metadata,
						cache, metadata_key,
						cache->asked_uniq_id,
						result_index,
						0);
		}
		while (keys && id <= maxid)
		{
			cur_tr_idx = -1;
			if ((tr_keys & 1) == 1)
			{
				tracker_idx++;
				cur_tr_idx = tracker_idx;
			}
			if ((keys & 1) == 1)
			{
				metadata_key = keymap_get_metadata_by_id(id);
				if (id == MTrackerSrc_ID_TITLE)
				{
					const gchar *cur_path;
					gboolean be_free;
					if (path_list)
					{
						cur_path = path_list[result_index];
					}
					else
					{
						cur_path = NULL;
					}
					value_str = _get_title_str(cache,
								result_index,
								cur_tr_idx,
								cur_path,
								&be_free);
					if (_value_str_is_allowed(value_str,
							MTrackerSrc_ID_TITLE)) {
                                		_replace_various_str_values(&value_str, be_free);
                                		mafw_metadata_add_str(metadata,
                                                      MAFW_METADATA_KEY_TITLE,
                                                      value_str);
						
                        		}
					if (be_free)
						g_free(value_str);
				}
				else
				{
					_parse_tracker_value_add_to_mdata(metadata,
						cache, metadata_key, id, result_index,
						cur_tr_idx);
				}
				
			}
			keys >>= 1;
			tr_keys >>= 1;
			id++;
		}
                /* If we didn't get any metadata, add a NULL */
                if (g_hash_table_size(metadata) == 0) {
                        mafw_metadata_release(metadata);
                        mafw_list = g_list_prepend(mafw_list, NULL);
                } else {
                        mafw_list = g_list_prepend(mafw_list, metadata);
                }
        }

        /* Place elements in right order */
        mafw_list = g_list_reverse(mafw_list);

        return mafw_list;
}

/*
 * tracker_cache_build_metadata_aggregated:
 * @cache: tracker cache
 * @count_childcount: @TRUE if childcount must be aggregated counting number of
 * times it appears
 *
 * Build a MAFW-metadata from cached aggregating durations and childcounts.
 *
 * Returns: a MAFW-metadata
 */
GHashTable *
tracker_cache_build_metadata_aggregated(TrackerCache *cache,
                                        gboolean count_childcount,
					guint64 asked_keys)
{
	gint id = 0;
	GHashTable *metadata;
        MetadataKey *metadata_key;
	guint64 tr_keys = cache->asked_tracker_keys;
	gint tracker_idx = -1, cur_tr_idx;

	metadata = mafw_metadata_new();
	
	if (cache->result_type == TRACKER_CACHE_RESULT_TYPE_QUERY)
	{
		tracker_idx += 2;
	}
	if (cache->result_type == TRACKER_CACHE_RESULT_TYPE_UNIQUE)
	{
		tracker_idx++;
	}
	

	while (asked_keys && id <= maxid)
	{
		cur_tr_idx = -1;
		if ((tr_keys & 1) == 1)
		{
			tracker_idx++;
			cur_tr_idx = tracker_idx;
		}
		if ((asked_keys & 1) == 1)
		{
			metadata_key = keymap_get_metadata_by_id(id);

			/* Special cases */
			if (metadata_key && (metadata_key->special == SPECIAL_KEY_CHILDCOUNT ||
			    metadata_key->special == SPECIAL_KEY_DURATION)) {
				gint intval;

				if (cur_tr_idx > -1)
				{
					intval = _aggregate_key(cache, id,
							       count_childcount,
							       cur_tr_idx);
					if (metadata_key->allowed_empty || intval > 0) {
						mafw_metadata_add_int(metadata,
							keymap_get_mafwkey_from_keyid(id),
							intval);
					}
				}
			} else {
				_parse_tracker_value_add_to_mdata(metadata,
						cache, metadata_key, id, 0,
						cur_tr_idx);
			}
		}
		asked_keys >>= 1;
		tr_keys >>= 1;
		id++;
	}

        return metadata;
}

/*
 * tracker_cache_key_exists:
 * @cache: tracker cache
 * @key: key too look
 *
 * Check if the cache contains the key
 *
 * Returns: @TRUE if the key is in the cache
 */
gboolean
tracker_cache_key_exists(TrackerCache *cache,
                         gint key_id){
	guint64 keyflag;
	
	if (cache->asked_uniq_id == key_id)
		return TRUE;
	keyflag =  keymap_get_flag_from_keyid(key_id);
        return (cache->asked_tracker_keys && keyflag) == keyflag;
}
