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

#ifndef _MAFW_TRACKER_SOURCE_KEY_MAPPING_H_
#define _MAFW_TRACKER_SOURCE_KEY_MAPPING_H_

#include <glib.h>
#include <tracker.h>

#define keymap_is_key_supported(key) (key_map_get_metadata((key)) != NULL)

void keymap_init_hash(void);

enum KeyType {
        /* Key is obtained from tracker (default) */
        TRACKER_KEY = 0,
        /* Key is related with album arts */
        ALBUM_ART_KEY,
        /* Key is related with thumbnails */
        THUMBNAIL_KEY
};

enum SpecialKey {
        NON_SPECIAL = 0,
        SPECIAL_KEY_CHILDCOUNT,
        SPECIAL_KEY_DURATION,
        SPECIAL_KEY_MIME,
        SPECIAL_KEY_TITLE,
        SPECIAL_KEY_URI
};

typedef struct MetadataKey {
        /* The type of the key. NOTE: G_TYPE_DATE will be handle as
         * G_TYPE_INT. But they are separated 'cause in we need to use
         * conversion functions when converting the keys to tracker keys */
        GType value_type;
        /* Is the key writable? Default is FALSE */
        gboolean writable;
        /* Allow the key's value empty (""/0) values? (not by default) */
        gboolean allowed_empty;
        /* Type of the key */
        enum KeyType key_type;
        /* A key needed to solve the current one (none by default) */
        gint depends_on_id;
        /* For special keys (those that are corner cases in the cache), mark its
         * speciality (nothing by default) */
        enum SpecialKey special;
} MetadataKey;

typedef struct TrackerKey {
        /* The name of the key */
        gchar *tracker_key;
        /* It's type */
        const gchar *tracker_valuetype;
} TrackerKey;

const gchar *keymap_mafw_key_to_tracker_key(const gchar *mafw_key,
				      ServiceType service);
gchar **keymap_mafw_sort_keys_to_tracker_keys(gchar **mafw_keys,
                                              ServiceType service);
gboolean keymap_mafw_key_is_writable(gchar *mafw_key);
const TrackerKey *keymap_get_tracker_info(const gchar *mafw_key,
                                    ServiceType service);
const gchar *keymap_get_mafwkey_from_keyid(gint keyid);
guint64 keymap_get_flag_from_keyid(gint keyid);
gint keymap_get_id_from_mafwkey(const gchar *mafw_key);
guint64 keymap_get_flag_from_mafwkey(const gchar *mafwkey);
guint64 keymap_compile_mdata_keys(const gchar* const* original);
const TrackerKey *keymap_get_tracker_info_by_id(gint mafw_key_id, 
						ServiceType service);
gint keymap_get_childcount_level(gint childcount_key_id);
const gchar *keymap_get_tracker_key_by_id(gint mafw_key_id,
				      ServiceType service);
MetadataKey *keymap_get_metadata_by_id(gint key_id);

#endif
