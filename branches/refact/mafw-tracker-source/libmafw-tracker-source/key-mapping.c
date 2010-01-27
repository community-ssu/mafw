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



struct _tracker_hash_data {
	const gchar *mafw_key;
	guint64 flag;
	TrackerKey tracker_musickeydata;
	TrackerKey tracker_videokeydata;
	TrackerKey tracker_playlistkeydata;
	TrackerKey tracker_commonkeydata;
	MetadataKey metadata_key;
};

#define VALUE_TYPE_STRING "String"
#define VALUE_TYPE_INT "Integer"
#define VALUE_TYPE_DATE "Date"
#define VALUE_TYPE_DOUBLE "Double"

static struct _tracker_hash_data tracker_hash_data[] = {
	{MAFW_METADATA_KEY_URI, MTrackerSrc_KEY_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_FKEY_FULLNAME, VALUE_TYPE_STRING},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, SPECIAL_KEY_URI}},
	{MAFW_METADATA_KEY_MIME, MTrackerSrc_KEY_MIME,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_FKEY_MIME, VALUE_TYPE_STRING},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, SPECIAL_KEY_MIME}},
	{MAFW_METADATA_KEY_TITLE, MTrackerSrc_KEY_TITLE,
		{TRACKER_AKEY_TITLE, VALUE_TYPE_STRING},
			{TRACKER_VKEY_TITLE, VALUE_TYPE_STRING},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, TRUE, 0, G_MAXINT, SPECIAL_KEY_TITLE}},
	{MAFW_METADATA_KEY_DURATION, MTrackerSrc_KEY_DURATION,
		{TRACKER_AKEY_DURATION, VALUE_TYPE_INT},
			{TRACKER_VKEY_DURATION, VALUE_TYPE_INT},
				{TRACKER_PKEY_DURATION, VALUE_TYPE_INT},
					{NULL, 0},
						{G_TYPE_INT, TRUE, FALSE, 0, G_MAXINT, SPECIAL_KEY_DURATION}},
	{MAFW_METADATA_KEY_ARTIST, MTrackerSrc_KEY_ARTIST,
		{TRACKER_AKEY_ARTIST, VALUE_TYPE_STRING},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_ALBUM, MTrackerSrc_KEY_ALBUM,
		{TRACKER_AKEY_ALBUM, VALUE_TYPE_STRING},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_GENRE, MTrackerSrc_KEY_GENRE,
		{TRACKER_AKEY_GENRE, VALUE_TYPE_STRING},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_TRACK, MTrackerSrc_KEY_TRACK,
		{TRACKER_AKEY_TRACK, VALUE_TYPE_INT},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_INT, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_YEAR, MTrackerSrc_KEY_YEAR,
		{TRACKER_AKEY_YEAR, VALUE_TYPE_DATE},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_INT, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_BITRATE, MTrackerSrc_KEY_BITRATE,
		{TRACKER_AKEY_BITRATE, VALUE_TYPE_DOUBLE},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_INT, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_PLAY_COUNT, MTrackerSrc_KEY_PLAY_COUNT, 
		{TRACKER_AKEY_PLAY_COUNT, VALUE_TYPE_INT},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, TRUE, TRUE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_LAST_PLAYED, MTrackerSrc_KEY_LAST_PLAYED,
		{TRACKER_AKEY_LAST_PLAYED, VALUE_TYPE_DATE},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, TRUE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_ADDED, MTrackerSrc_KEY_ADDED,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_FKEY_ADDED, VALUE_TYPE_DATE},
						{G_TYPE_LONG, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_MODIFIED, MTrackerSrc_KEY_MODIFIED,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_FKEY_MODIFIED, VALUE_TYPE_DATE},
						{G_TYPE_LONG, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_THUMBNAIL_URI, MTrackerSrc_KEY_THUMBNAIL_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, THUMBNAIL_KEY, MTrackerSrc_ID_URI, 0}},
	{MAFW_METADATA_KEY_THUMBNAIL_SMALL_URI, MTrackerSrc_KEY_THUMBNAIL_SMALL_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, THUMBNAIL_KEY, MTrackerSrc_ID_URI, 0}},
	{MAFW_METADATA_KEY_THUMBNAIL_MEDIUM_URI, MTrackerSrc_KEY_THUMBNAIL_MEDIUM_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, THUMBNAIL_KEY, MTrackerSrc_ID_URI, 0}},
	{MAFW_METADATA_KEY_THUMBNAIL_LARGE_URI, MTrackerSrc_KEY_THUMBNAIL_LARGE_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, THUMBNAIL_KEY, MTrackerSrc_ID_URI, 0}},
	{MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, MTrackerSrc_KEY_PAUSED_THUMBNAIL_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_VKEY_PAUSED_THUMBNAIL, VALUE_TYPE_STRING},
						{G_TYPE_STRING, TRUE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_PAUSED_POSITION, MTrackerSrc_KEY_PAUSED_POSITION,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_VKEY_PAUSED_POSITION, VALUE_TYPE_INT},
						{G_TYPE_INT, TRUE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_RES_X, MTrackerSrc_KEY_RES_X,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_VKEY_RES_X, VALUE_TYPE_INT},
						{G_TYPE_INT, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_RES_Y, MTrackerSrc_KEY_RES_Y,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_VKEY_RES_Y, VALUE_TYPE_INT},
						{G_TYPE_INT, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_FILENAME, MTrackerSrc_KEY_FILENAME,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_FKEY_FILENAME, VALUE_TYPE_STRING},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_FILESIZE, MTrackerSrc_KEY_FILESIZE,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_FKEY_FILESIZE, VALUE_TYPE_INT},
						{G_TYPE_INT, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_COPYRIGHT, MTrackerSrc_KEY_COPYRIGHT,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_FKEY_COPYRIGHT, VALUE_TYPE_STRING},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_ALBUM_ART_URI, MTrackerSrc_KEY_ALBUM_ART_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, ALBUM_ART_KEY, MTrackerSrc_ID_ALBUM, 0}},
	{MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI, MTrackerSrc_KEY_ALBUM_ART_SMALL_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, ALBUM_ART_KEY, MTrackerSrc_ID_ALBUM_ART_URI, 0}},
	{MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI, MTrackerSrc_KEY_ALBUM_ART_MEDIUM_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, ALBUM_ART_KEY, MTrackerSrc_ID_ALBUM_ART_URI, 0}},
	{MAFW_METADATA_KEY_ALBUM_ART_LARGE_URI, MTrackerSrc_KEY_ALBUM_ART_LARGE_URI,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_STRING, FALSE, FALSE, ALBUM_ART_KEY, MTrackerSrc_ID_ALBUM_ART_URI, 0}},
	{MAFW_METADATA_KEY_VIDEO_FRAMERATE, MTrackerSrc_KEY_VIDEO_FRAMERATE,
		{NULL, 0},
			{TRACKER_VKEY_FRAMERATE, VALUE_TYPE_DOUBLE},
				{NULL, 0},
					{NULL, 0},
						{G_TYPE_FLOAT, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_VIDEO_SOURCE, MTrackerSrc_KEY_VIDEO_SOURCE,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_VKEY_SOURCE, VALUE_TYPE_STRING},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, 0}},
	{MAFW_METADATA_KEY_CHILDCOUNT_1, MTrackerSrc_KEY_CHILDCOUNT_1,
		{NULL, 0},
			{NULL, 0},
				{TRACKER_PKEY_COUNT, VALUE_TYPE_INT},
					{NULL, 0},
						{G_TYPE_INT, FALSE, TRUE, 0, G_MAXINT, SPECIAL_KEY_CHILDCOUNT}},
	{MAFW_METADATA_KEY_CHILDCOUNT_2, MTrackerSrc_KEY_CHILDCOUNT_2,
		{NULL, 0},
			{NULL, 0},
				{TRACKER_PKEY_COUNT, VALUE_TYPE_INT},
					{NULL, 0},
						{G_TYPE_INT, FALSE, TRUE, 0, G_MAXINT, SPECIAL_KEY_CHILDCOUNT}},
	{MAFW_METADATA_KEY_CHILDCOUNT_3, MTrackerSrc_KEY_CHILDCOUNT_3,
		{NULL, 0},
			{NULL, 0},
				{TRACKER_PKEY_COUNT, VALUE_TYPE_INT},
					{NULL, 0},
						{G_TYPE_INT, FALSE, TRUE, 0, G_MAXINT, SPECIAL_KEY_CHILDCOUNT}},
	{MAFW_METADATA_KEY_CHILDCOUNT_4, MTrackerSrc_KEY_CHILDCOUNT_4,
		{NULL, 0},
			{NULL, 0},
				{TRACKER_PKEY_COUNT, VALUE_TYPE_INT},
					{NULL, 0},
						{G_TYPE_INT, FALSE, TRUE, 0, G_MAXINT, SPECIAL_KEY_CHILDCOUNT}},
	{TRACKER_PKEY_VALID_DURATION, MTrackerSrc_TRACKER_PKEY_VALID_DURATION,
		{NULL, 0},
			{NULL, 0},
				{TRACKER_PKEY_VALID_DURATION, VALUE_TYPE_INT},
					{NULL, 0},
						{G_TYPE_BOOLEAN, FALSE, FALSE, 0, G_MAXINT, 0}},
	{TRACKER_FKEY_PATH, MTrackerSrc_TRACKER_FKEY_PATH,
		{NULL, 0},
			{NULL, 0},
				{NULL, 0},
					{TRACKER_FKEY_PATH, VALUE_TYPE_STRING},
						{G_TYPE_STRING, FALSE, FALSE, 0, G_MAXINT, 0}}
};

static GHashTable *_tracker_key_hash;
gint maxid = -1;

void keymap_init_hash(void)
{
	gint i = 0;

	maxid = G_N_ELEMENTS(tracker_hash_data) - 1;

	if (_tracker_key_hash)
		return;

	_tracker_key_hash = g_hash_table_new(g_str_hash, g_str_equal);
	g_assert(_tracker_key_hash);
	
	for(i=0; i <= maxid; i++)
	{
		g_hash_table_insert(_tracker_key_hash,
					(gpointer)tracker_hash_data[i].mafw_key,
					GINT_TO_POINTER(i+1));
	}
}

const gchar *keymap_get_mafwkey_from_keyid(gint keyid)
{
	if (keyid >= G_N_ELEMENTS(tracker_hash_data))
		return NULL;
	return tracker_hash_data[keyid].mafw_key;
}

guint64 keymap_get_flag_from_keyid(gint keyid)
{
	if (keyid >= G_N_ELEMENTS(tracker_hash_data))
		return 0;
	return tracker_hash_data[keyid].flag;
}

gint keymap_get_id_from_mafwkey(const gchar *mafw_key)
{
	gint id = GPOINTER_TO_INT(g_hash_table_lookup(_tracker_key_hash,
					mafw_key));
	if (id != 0)
		return  id - 1;
	return G_MAXINT;
}

/**
 * keymap_get_flag_from_mafwkey:
 * @mafwkey:	MAFW metadata key
 *
 * Returns the flag of the asked metadata-key, or 0, if not supported
 */
guint64 keymap_get_flag_from_mafwkey(const gchar *mafwkey)
{
	guint64 flagn = 1;
	gint id = keymap_get_id_from_mafwkey(mafwkey);

	if (id == G_MAXINT)
		return 0;

	flagn <<= id;
	return flagn;
}

/**
 * keymap_compile_mdata_keys:
 * @original:	metadatakeys
 *
 * Converts the list of metadata-keys into flags.
 */
guint64 keymap_compile_mdata_keys(const gchar* const* original)
{
	guint64 mkeys = 0;
	gint i;
	
	if (original == NULL || original[0] == NULL)
		return 0;

	if (strcmp(MAFW_SOURCE_ALL_KEYS[0], original[0]) == 0)
	{
		mkeys = G_MAXUINT64;
		mkeys &= ~MTrackerSrc_TRACKER_PKEY_VALID_DURATION;
		mkeys &= ~MTrackerSrc_TRACKER_FKEY_PATH;
		return mkeys;
	}
	
	for (i = 0; original[i]; i++)
	{
		mkeys |= keymap_get_flag_from_mafwkey(original[i]);
	}
	return mkeys;
}

const TrackerKey *keymap_get_tracker_info_by_id(gint mafw_key_id, 
						ServiceType service)
{
        TrackerKey *tracker_key;

	if (mafw_key_id >= G_N_ELEMENTS(tracker_hash_data))
		return NULL;

        switch (service) {
        case SERVICE_VIDEOS:
                tracker_key = &tracker_hash_data[mafw_key_id].tracker_videokeydata;
                break;
        case SERVICE_PLAYLISTS:
                tracker_key = &tracker_hash_data[mafw_key_id].tracker_playlistkeydata;
                break;
        default:
                tracker_key = &tracker_hash_data[mafw_key_id].tracker_musickeydata;
        }

        if (tracker_key->tracker_key) {
                return tracker_key;
        } else {
		tracker_key = &tracker_hash_data[mafw_key_id].tracker_commonkeydata;
		if (tracker_key->tracker_key) {
                	return tracker_key;
		}
        }
	return NULL;
}

const TrackerKey *keymap_get_tracker_info(const gchar *mafw_key,
                                    ServiceType service)
{
	gint id = keymap_get_id_from_mafwkey(mafw_key);

	return keymap_get_tracker_info_by_id(id, service);
}

const gchar *keymap_mafw_key_to_tracker_key(const gchar *mafw_key,
				      ServiceType service)
{
        const TrackerKey *tracker_key;

        tracker_key = keymap_get_tracker_info(mafw_key, service);

        if (tracker_key) {
                return tracker_key->tracker_key;
        } else {
                return NULL;
        }
}

gint keymap_get_childcount_level(gint childcount_key_id)
{
        return childcount_key_id - MTrackerSrc_ID_CHILDCOUNT_1 + 1;
}

const gchar *keymap_get_tracker_key_by_id(gint mafw_key_id,
				      ServiceType service)
{
        const TrackerKey *tracker_key;

        tracker_key = keymap_get_tracker_info_by_id(mafw_key_id, service);

        if (tracker_key) {
                return tracker_key->tracker_key;
        }
	return NULL;
}

MetadataKey *keymap_get_metadata_by_id(gint key_id)
{
	if (key_id >= G_N_ELEMENTS(tracker_hash_data))
		return NULL;
        return &tracker_hash_data[key_id].metadata_key;
}




gchar **keymap_mafw_sort_keys_to_tracker_keys(gchar **mafw_keys,
                                              ServiceType service)
{
	gint i;
        const TrackerKey *tracker_key;
        gchar *key;
        gchar *sort_type;
	GPtrArray *str_array;

	if (mafw_keys == NULL) {
		return NULL;
	}

	str_array = g_ptr_array_new();

	/* Now, translate the keys supported in tracker */
	for (i=0; mafw_keys[i] != NULL; i++) {
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
                        g_ptr_array_add(str_array,
                                g_strconcat (
                                        keymap_mafw_key_to_tracker_key(
                                                key,
                                                service),
                                        sort_type,
                                        NULL));
                }
        }
	g_ptr_array_add(str_array, NULL);

	return (gchar**)g_ptr_array_free(str_array, FALSE);;
}

gboolean keymap_mafw_key_is_writable(gchar *mafw_key)
{
        MetadataKey *metadata_key;

        metadata_key = keymap_get_metadata_by_id(keymap_get_id_from_mafwkey(mafw_key));

        /* If key is not found, return FALSE */
        return metadata_key && metadata_key->writable;
}
