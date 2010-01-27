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

#include "album-art.h"
#include "util.h"
#include "key-mapping.h"
#include <string.h>
#include <stdlib.h>
#include <hildon-thumbnail-factory.h>
#include <hildon-albumart-factory.h>


/* ------------------------- Public API ------------------------- */
static gboolean _file_check(const gchar *path)
{
	return g_file_test(path, G_FILE_TEST_EXISTS);
}

static gboolean _file_check_uri(const gchar *uri)
{
	return _file_check(uri + sizeof("file://"));
}

gchar *albumart_get_thumbnail_uri(const gchar *orig_file_uri,
				  enum thumbnail_size size)
{
        gchar *file_uri;

        if (size == THUMBNAIL_CROPPED) {
                file_uri = hildon_thumbnail_get_uri(orig_file_uri,
                                                    128, 128, TRUE);
                /* Check if file doesn't exist */
		if (_file_check_uri(file_uri))
			return file_uri;
		g_free(file_uri);
		return NULL;
        } else {
                /* Get the original album art */
                file_uri = albumart_get_album_art_uri(orig_file_uri);
        }

        return file_uri;
}

gchar *albumart_get_album_art_uri(const gchar *album)
{
	gchar *file_uri;
        gchar *file_path;

	if (util_tracker_value_is_unknown(album))
                return NULL;

	/* Get the path to the album-art */
	file_path = hildon_albumart_get_path(NULL, album, "album");
	if (_file_check(file_path))
	{
		file_uri = g_filename_to_uri(file_path, NULL, NULL);
        	g_free(file_path);
		return file_uri;
	}
	g_free(file_path);

	return 	NULL;
}

gboolean albumart_key_is_thumbnail_by_id(gint key_id)
{
        MetadataKey *metadata_key;

        metadata_key = keymap_get_metadata_by_id(key_id);

        if (metadata_key) {
                return metadata_key->key_type == THUMBNAIL_KEY;
        } else {
                return FALSE;
        }
}
