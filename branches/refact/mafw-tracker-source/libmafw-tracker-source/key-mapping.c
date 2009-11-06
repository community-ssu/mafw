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

#include "key-mapping.h"
#include "album-art.h"
#include "definitions.h"
#include <string.h>
#include <libmafw/mafw.h>

/* ------------------------- Public API ------------------------- */

gchar *keymap_mafw_key_to_tracker_key(const gchar *mafw_key,
				      ServiceType service)
{
        TrackerKey *tracker_key;

        tracker_key = keymap_get_tracker_info(mafw_key, service);

        if (tracker_key) {
                return g_strdup(tracker_key->tracker_key);
        } else {
                return NULL;
        }
}

gboolean keymap_is_key_supported_in_tracker(const gchar *mafw_key)
{
        static InfoKeyTable *t = NULL;

        if (!t) {
                t = keymap_get_info_key_table();
        }

        return g_hash_table_lookup(t->music_keys, mafw_key) != NULL ||
                g_hash_table_lookup(t->videos_keys, mafw_key) != NULL ||
                g_hash_table_lookup(t->playlist_keys, mafw_key) != NULL ||
                g_hash_table_lookup(t->common_keys, mafw_key) != NULL;
}

gboolean keymap_mafw_key_is_writable(gchar *mafw_key)
{
        MetadataKey *metadata_key;

        metadata_key = keymap_get_metadata(mafw_key);

        /* If key is not found, return FALSE */
        return metadata_key && metadata_key->writable;
}

struct TrackerKeyData {
	gchar *metadata_key;
	TrackerKey keydata;
};

static struct TrackerKeyData music_keys[] = {
	{MAFW_METADATA_KEY_TITLE, {TRACKER_AKEY_TITLE, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_DURATION, {TRACKER_AKEY_DURATION, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_ARTIST, {TRACKER_AKEY_ARTIST, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_ALBUM, {TRACKER_AKEY_ALBUM, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_GENRE, {TRACKER_AKEY_GENRE, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_TRACK, {TRACKER_AKEY_TRACK, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_YEAR, {TRACKER_AKEY_YEAR, VALUE_TYPE_DATE}},
	{MAFW_METADATA_KEY_BITRATE, {TRACKER_AKEY_BITRATE, VALUE_TYPE_DOUBLE}},
	{MAFW_METADATA_KEY_LAST_PLAYED, {TRACKER_AKEY_LAST_PLAYED, VALUE_TYPE_DATE}},
	{MAFW_METADATA_KEY_PLAY_COUNT, {TRACKER_AKEY_PLAY_COUNT, VALUE_TYPE_INT}},
	{NULL, {0, 0}}
};

static struct TrackerKeyData video_keys[] = {
	{MAFW_METADATA_KEY_TITLE, {TRACKER_VKEY_TITLE, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_DURATION, {TRACKER_VKEY_DURATION, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_VIDEO_FRAMERATE, {TRACKER_VKEY_FRAMERATE, VALUE_TYPE_DOUBLE}},
	{NULL, {0, 0}}
};

static struct TrackerKeyData playlist_keys[] = {
	{MAFW_METADATA_KEY_DURATION, {TRACKER_PKEY_DURATION, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_CHILDCOUNT_1, {TRACKER_PKEY_COUNT, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_CHILDCOUNT_2, {TRACKER_PKEY_COUNT, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_CHILDCOUNT_3, {TRACKER_PKEY_COUNT, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_CHILDCOUNT_4, {TRACKER_PKEY_COUNT, VALUE_TYPE_INT}},
	{TRACKER_PKEY_VALID_DURATION, {TRACKER_PKEY_VALID_DURATION, VALUE_TYPE_INT}},
	{NULL, {0, 0}}
};

static struct TrackerKeyData common_keys[] = {
	{MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, {TRACKER_VKEY_PAUSED_THUMBNAIL, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_PAUSED_POSITION, {TRACKER_VKEY_PAUSED_POSITION, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_VIDEO_SOURCE, {TRACKER_VKEY_SOURCE, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_RES_X, {TRACKER_VKEY_RES_X, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_RES_Y, {TRACKER_VKEY_RES_Y, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_COPYRIGHT, {TRACKER_FKEY_COPYRIGHT, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_FILESIZE, {TRACKER_FKEY_FILESIZE, VALUE_TYPE_INT}},
	{MAFW_METADATA_KEY_FILENAME, {TRACKER_FKEY_FILENAME, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_MIME, {TRACKER_FKEY_MIME, VALUE_TYPE_STRING}},
	{MAFW_METADATA_KEY_ADDED, {TRACKER_FKEY_ADDED, VALUE_TYPE_DATE}},
	{MAFW_METADATA_KEY_MODIFIED, {TRACKER_FKEY_MODIFIED, VALUE_TYPE_DATE}},
	{MAFW_METADATA_KEY_URI, {TRACKER_FKEY_FULLNAME, VALUE_TYPE_STRING}},
	{TRACKER_FKEY_PATH, {TRACKER_FKEY_PATH, VALUE_TYPE_STRING}},
	{NULL, {0, 0}}
};

struct MetadataKeyList {
	gchar *metadata_key;
	MetadataKey keydata;
};

static struct MetadataKeyList mdata_keys[] = {
                {MAFW_METADATA_KEY_CHILDCOUNT_1, {G_TYPE_INT, FALSE, TRUE, 0, NULL, SPECIAL_KEY_CHILDCOUNT}},
                {MAFW_METADATA_KEY_CHILDCOUNT_2, {G_TYPE_INT, FALSE, TRUE, 0, NULL, SPECIAL_KEY_CHILDCOUNT}},
                {MAFW_METADATA_KEY_CHILDCOUNT_3, {G_TYPE_INT, FALSE, TRUE, 0, NULL, SPECIAL_KEY_CHILDCOUNT}},
                {MAFW_METADATA_KEY_CHILDCOUNT_4, {G_TYPE_INT, FALSE, TRUE, 0, NULL, SPECIAL_KEY_CHILDCOUNT}},
                {MAFW_METADATA_KEY_VIDEO_FRAMERATE, {G_TYPE_FLOAT, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_COPYRIGHT, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_FILESIZE, {G_TYPE_INT, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_FILENAME, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_TITLE, {G_TYPE_STRING, FALSE, TRUE, 0, NULL, SPECIAL_KEY_TITLE}},
                {MAFW_METADATA_KEY_DURATION, {G_TYPE_INT, TRUE, FALSE, 0, NULL, SPECIAL_KEY_DURATION}},
                {MAFW_METADATA_KEY_MIME, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, SPECIAL_KEY_MIME}},
                {MAFW_METADATA_KEY_ARTIST, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_ALBUM, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_GENRE, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_TRACK, {G_TYPE_INT, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_YEAR, {G_TYPE_INT, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_BITRATE, {G_TYPE_INT, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_URI, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, SPECIAL_KEY_URI}},
                {MAFW_METADATA_KEY_LAST_PLAYED, {G_TYPE_LONG, TRUE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_PLAY_COUNT, {G_TYPE_STRING, TRUE, TRUE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_ADDED, {G_TYPE_LONG, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_MODIFIED, {G_TYPE_LONG, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, {G_TYPE_STRING, TRUE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_PAUSED_POSITION, {G_TYPE_INT, TRUE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_VIDEO_SOURCE, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_RES_X, {G_TYPE_INT, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_RES_Y, {G_TYPE_INT, FALSE, FALSE, 0, NULL, 0}},
                {TRACKER_PKEY_VALID_DURATION, {G_TYPE_BOOLEAN, FALSE, FALSE, 0, NULL, 0}},
                {TRACKER_FKEY_PATH, {G_TYPE_STRING, FALSE, FALSE, 0, NULL, 0}},
                {MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI, {G_TYPE_STRING, FALSE, FALSE, ALBUM_ART_KEY, MAFW_METADATA_KEY_ALBUM_ART_URI, 0}},
                {MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI, {G_TYPE_STRING, FALSE, FALSE, ALBUM_ART_KEY, MAFW_METADATA_KEY_ALBUM_ART_URI, 0}},
                {MAFW_METADATA_KEY_ALBUM_ART_LARGE_URI, {G_TYPE_STRING, FALSE, FALSE, ALBUM_ART_KEY, MAFW_METADATA_KEY_ALBUM_ART_URI, 0}},
                {MAFW_METADATA_KEY_ALBUM_ART_URI, {G_TYPE_STRING, FALSE, FALSE, ALBUM_ART_KEY, MAFW_METADATA_KEY_ALBUM_ART, 0}},
                {MAFW_METADATA_KEY_THUMBNAIL_SMALL_URI, {G_TYPE_STRING, FALSE, FALSE, THUMBNAIL_KEY, MAFW_METADATA_KEY_URI, 0}},
                {MAFW_METADATA_KEY_THUMBNAIL_MEDIUM_URI, {G_TYPE_STRING, FALSE, FALSE, THUMBNAIL_KEY, MAFW_METADATA_KEY_URI, 0}},
                {MAFW_METADATA_KEY_THUMBNAIL_LARGE_URI, {G_TYPE_STRING, FALSE, FALSE, THUMBNAIL_KEY, MAFW_METADATA_KEY_URI, 0}},
                {MAFW_METADATA_KEY_THUMBNAIL_URI, {G_TYPE_STRING, FALSE, FALSE, THUMBNAIL_KEY, MAFW_METADATA_KEY_URI, 0}},
                {NULL, {0, FALSE, FALSE, 0, NULL, 0}}
};

InfoKeyTable *keymap_get_info_key_table(void)
{
	static InfoKeyTable *table = NULL;

        if (!table) {
		gint i;

                table = g_new(InfoKeyTable, 1);
                table->music_keys = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
                table->videos_keys = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
                table->playlist_keys = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
                table->common_keys = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
                table->metadata_keys = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);

                /* Insert mapping for music service */
		for (i = 0; music_keys[i].metadata_key; i++)
		{
			g_hash_table_insert(table->music_keys,
                                    music_keys[i].metadata_key,
                                    &music_keys[i].keydata);
		}

                /* Insert mapping for videos service */
 		for (i = 0; video_keys[i].metadata_key; i++)
		{
			g_hash_table_insert(table->videos_keys,
                                    video_keys[i].metadata_key,
                                    &video_keys[i].keydata);
		}

                /* Insert mapping for playlist service */
 		for (i = 0; playlist_keys[i].metadata_key; i++)
		{
			g_hash_table_insert(table->playlist_keys,
                                    playlist_keys[i].metadata_key,
                                    &playlist_keys[i].keydata);
		}

                /* Insert mapping common for all services */
		for (i = 0; common_keys[i].metadata_key; i++)
		{
			g_hash_table_insert(table->common_keys,
                                    common_keys[i].metadata_key,
                                    &common_keys[i].keydata);
		}

                /* Insert metadata assocciated with each key */
		for (i = 0; mdata_keys[i].metadata_key; i++)
		{
			g_hash_table_insert(table->metadata_keys,
                                    mdata_keys[i].metadata_key,
                                    &mdata_keys[i].keydata);
		}
        }

	return table;
}

gchar **keymap_mafw_keys_to_tracker_keys(gchar **mafw_keys,
					 ServiceType service)
{
	gchar **tracker_keys;
	gint i, count;
        TrackerKey *tracker_key;

	if (mafw_keys == NULL) {
		return NULL;
	}

	/* Count the number of keys */
	for (i=0, count=0; mafw_keys[i] != NULL; i++) {
                /* Check if the key is translatable to tracker */
                if (keymap_get_tracker_info(mafw_keys[i], service) != NULL) {
                        count++;
                }
	}

	/* Allocate memory for the converted array (include trailing NULL) */
	tracker_keys = g_new(gchar *, count + 1);

	/* Now, translate the keys supported in tracker */
	for (i=0, count=0; mafw_keys[i] != NULL; i++) {
                tracker_key = keymap_get_tracker_info(mafw_keys[i], service);
                if (tracker_key) {
                        tracker_keys[count++] =
                                keymap_mafw_key_to_tracker_key(mafw_keys[i],
                                                               service);
                }
        }
	tracker_keys[count] = NULL;

	return tracker_keys;
}

gchar **keymap_mafw_sort_keys_to_tracker_keys(gchar **mafw_keys,
                                              ServiceType service)
{
	gchar **tracker_keys;
	gint i, count;
        TrackerKey *tracker_key;
        gchar *key;
        gchar *sort_type;

	if (mafw_keys == NULL) {
		return NULL;
	}

	/* Count the number of keys */
	for (i=0, count=0; mafw_keys[i] != NULL; i++) {
                key = mafw_keys[i];
                /* Skip the sort type (+/-) */
                if (key[0] == '+' || key[0] == '-') {
                        key++;
                }
                /* Check if the key is translatable to tracker */
                if (keymap_get_tracker_info(key, service) != NULL) {
                        count++;
                }
	}

	/* Allocate memory for the converted array (include trailing NULL) */
	tracker_keys = g_new(gchar *, count + 1);

	/* Now, translate the keys supported in tracker */
	for (i=0, count=0; mafw_keys[i] != NULL; i++) {
                key = mafw_keys[i];
                /* Set the sort type */
                if (key[0] == '-') {
                        sort_type = " DESC";
                        key++;
                } else {
                        sort_type = " ASC";
                        if (key[0] == '+') {
                                key++;
                        }
                }
                tracker_key = keymap_get_tracker_info(key, service);
                if (tracker_key) {
                        tracker_keys[count++] =
                                g_strconcat (
                                        keymap_mafw_key_to_tracker_key(
                                                key,
                                                service),
                                        sort_type,
                                        NULL);
                }
        }
	tracker_keys[count] = NULL;

	return tracker_keys;
}

MetadataKey *keymap_get_metadata(const gchar *mafw_key)
{
        static InfoKeyTable *table = NULL;

        if (!table) {
                table = keymap_get_info_key_table();
        }

        return g_hash_table_lookup(table->metadata_keys, mafw_key);
}

TrackerKey *keymap_get_tracker_info(const gchar *mafw_key,
                                    ServiceType service)
{
        static InfoKeyTable *table = NULL;
        TrackerKey *tracker_key;

        if (!table) {
                table = keymap_get_info_key_table();
        }

        switch (service) {
        case SERVICE_VIDEOS:
                tracker_key = g_hash_table_lookup(table->videos_keys,
                                                  mafw_key);
                break;
        case SERVICE_PLAYLISTS:
                tracker_key = g_hash_table_lookup(table->playlist_keys,
                                                  mafw_key);
                break;
        default:
                tracker_key = g_hash_table_lookup(table->music_keys,
                                                  mafw_key);
        }

        if (tracker_key) {
                return tracker_key;
        } else {
                return g_hash_table_lookup(table->common_keys,
                                           mafw_key);
        }
}

enum ValueType keymap_get_tracker_type(const gchar *mafw_key,
                              ServiceType service)
{
        TrackerKey *tracker_key;

        tracker_key = keymap_get_tracker_info(mafw_key, service);

        if (tracker_key) {
                return tracker_key->value_type;
        } else {
                return VALUE_TYPE_NONE;
        }
}
