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

#ifndef __MAFW_METADATA_H__
#define __MAFW_METADATA_H__

#include <glib.h>
#include <glib-object.h>

#include <libmafw/mafw-filter.h>

/*
 * Returns the number of varadic arguments, which must be of @type.
 * Also checks for type-correctness.  The number of varadic arguments
 * can be zero.  This macro is private and one should not rely on it.
 * (You may copy the idea, of course.)
 */
#define _MAFW_NARGS(type, ...) \
	(sizeof((type[]){ __VA_ARGS__ }) / sizeof(type))

/**
 * mafw_metadata_add_int:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of integer values to the metadata hash table @md.
 * If @key already exists in the hash table the new values are appended.
 * @key will be duplicated if necessary.  If the varadic argument list
 * is empty, nothing is added to @md.  The value arguments are evaluated
 * only once.  The integers can, in fact, either be signed or unsigned;
 * their interpretation is up to the one reading the values.
 */
#define mafw_metadata_add_int(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_INT,		\
				    _MAFW_NARGS(gint, ##__VA_ARGS__),	\
				    ##__VA_ARGS__)

/* Some more macros for numeric types. */
/**
 * mafw_metadata_add_uint:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of unsigned integer values to the metadata
 * hash table @md.  If @key already exists in the hash table the new
 * values are appended.  @key will be duplicated if necessary.  If the
 * varadic argument list is empty, nothing is added to @md.  The value
 * arguments are evaluated only once.  The integers can, in fact,
 * either be signed or unsigned; their interpretation is up to the one
 * reading the values.
 */
#define mafw_metadata_add_uint(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_UINT,		\
				    _MAFW_NARGS(guint, ##__VA_ARGS__),	\
				    ##__VA_ARGS__)

/**
 * mafw_metadata_add_long:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of long integer values to the metadata hash
 * table @md.  If @key already exists in the hash table the new values
 * are appended.  @key will be duplicated if necessary.  If the
 * varadic argument list is empty, nothing is added to @md.  The value
 * arguments are evaluated only once.  The integers can, in fact,
 * either be signed or unsigned; their interpretation is up to the one
 * reading the values.
 */
#define mafw_metadata_add_long(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_LONG,		\
				    _MAFW_NARGS(glong, ##__VA_ARGS__),	\
				    ##__VA_ARGS__)

/**
 * mafw_metadata_add_ulong:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of unsigned long integer values to the metadata
 * hash table @md.  If @key already exists in the hash table the new
 * values are appended.  @key will be duplicated if necessary.  If the
 * varadic argument list is empty, nothing is added to @md.  The value
 * arguments are evaluated only once.  The integers can, in fact,
 * either be signed or unsigned; their interpretation is up to the one
 * reading the values.
 */
#define mafw_metadata_add_ulong(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_ULONG,		\
				     _MAFW_NARGS(gulong, ##__VA_ARGS__), \
				     ##__VA_ARGS__)

/**
 * mafw_metadata_add_int64
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of 64 bits integer values to the metadata
 * hash table @md.  If @key already exists in the hash table the new
 * values are appended.  @key will be duplicated if necessary.  If the
 * varadic argument list is empty, nothing is added to @md.  The value
 * arguments are evaluated only once.  The integers can, in fact,
 * either be signed or unsigned; their interpretation is up to the one
 * reading the values.
 */
#define mafw_metadata_add_int64(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_INT64,		\
				     _MAFW_NARGS(gint64, ##__VA_ARGS__), \
				     ##__VA_ARGS__)

/**
 * mafw_metadata_add_uint64:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of unsigned 64 bits integer values to the
 * metadata hash table @md.  If @key already exists in the hash table
 * the new values are appended.  @key will be duplicated if necessary.
 * If the varadic argument list is empty, nothing is added to @md.
 * The value arguments are evaluated only once.  The integers can, in
 * fact, either be signed or unsigned; their interpretation is up to
 * the one reading the values.
 */
#define mafw_metadata_add_uint64(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_UINT64,		\
				     _MAFW_NARGS(guint64, ##__VA_ARGS__), \
				     ##__VA_ARGS__)

/**
 * mafw_metadata_add_double:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of double floating pointer values to the
 * metadata hash table @md.  If @key already exists in the hash table
 * the new values are appended.  @key will be duplicated if necessary.
 * If the varadic argument list is empty, nothing is added to @md.
 * The value arguments are evaluated only once.
 */
#define mafw_metadata_add_double(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_DOUBLE,		\
				     _MAFW_NARGS(gdouble, ##__VA_ARGS__), \
				     ##__VA_ARGS__)

/**
 * mafw_metadata_add_boolean:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of boolean values to the metadata hash table
 * @md.  If @key already exists in the hash table the new values are
 * appended.  @key will be duplicated if necessary.  If the varadic
 * argument list is empty, nothing is added to @md.  The value
 * arguments are evaluated only once. 
 */
#define mafw_metadata_add_boolean(md, key, ...)			\
	mafw_metadata_add_something(md, key, G_TYPE_BOOLEAN,		\
				     _MAFW_NARGS(gboolean, ##__VA_ARGS__), \
				     ##__VA_ARGS__)

/**
 * mafw_metadata_add_str:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of string values to the metadata hash table
 * @md.  If @key already exists in the hash table the new values are
 * appended.  @key will be duplicated if necessary.  If the varadic
 * argument list is empty, nothing is added to @md.  The value
 * arguments are duplicated.
 */
#define mafw_metadata_add_str(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_STRING,		\
				    _MAFW_NARGS(const gchar *, ##__VA_ARGS__),\
				    ##__VA_ARGS__)

/**
 * mafw_metadata_add_val:
 * @md: hash table
 * @key: key to use
 * @...: list of values
 * 
 * Adds arbitrary number of _values_ of arbitrary #GValues to the
 * metadata hash table @md.  If @key already exists in the hash table
 * the new values are appended.  @key will be duplicated if necessary.
 * If the varadic argument list is empty, nothing is added to @md.
 * The value arguments are duplicated. Retrieval of metadata values
 * from @md can be done as usual.
 */
#define mafw_metadata_add_val(md, key, ...)				\
	mafw_metadata_add_something(md, key, G_TYPE_VALUE,		\
				    _MAFW_NARGS(GValue *, ##__VA_ARGS__),\
				    ##__VA_ARGS__)

/**
 * SECTION: mafwmetadata
 * @short_description: metadata handling.
 *
 * List of well-known metadata keys to be used in mafw_source_get_metadata(),
 * mafw_renderer_get_metadata() and mafw_source_browse().  It is not guaranteed
 * all sources understand all of the keys listed here, but if they do they
 * will recognize the referred metadata by these names.
 */

/*
 * This file can be extended later, if we support more complex
 * metadata values.
 */

/**
 * MAFW_METADATA_VALUE_VARIOUS_VALUES:
 *
 * This value is used when the key has several values (e.g., several artists or several albums).
 */
#define MAFW_METADATA_VALUE_VARIOUS_VALUES      "__VV__"

/**
 * MAFW_METADATA_KEY_URI:
 *
 * The URI by which any renderer supporting the schema should be able to
 * locate the item.  The value of this metadata is a string, as
 * defined in RFC 3986.metadata
 */
#define MAFW_METADATA_KEY_URI			"uri"

/**
 * MAFW_METADATA_KEY_MIME:
 *
 * Describes the type and format of the item.  Its value is a string
 * of &lt;type&gt; '/' &lt;subtype&gt; defined in RFC2045 section 5.1.
 * The value should be interpreted according to that standard.
 */
#define MAFW_METADATA_KEY_MIME			"mime-type"

/**
 * MAFW_METADATA_VALUE_MIME_CONTAINER:
 *
 * This is the MIME type of a container provided by MAFW source components.
 */
#define MAFW_METADATA_VALUE_MIME_CONTAINER      "x-mafw/container"

/**
 * MAFW_METADATA_VALUE_MIME_AUDIO:
 *
 * This is the MIME type of an audio item provided by MAFW source components.
 */
#define MAFW_METADATA_VALUE_MIME_AUDIO      "x-mafw/audio"

/**
 * MAFW_METADATA_VALUE_MIME_VIDEO:
 *
 * This is the MIME type of a video item provided by MAFW source components.
 */
#define MAFW_METADATA_VALUE_MIME_VIDEO      "x-mafw/video"

/**
 * MAFW_METADATA_KEY_TITLE:
 *
 * Describes the title of the item. Its value is a string.
 */
#define MAFW_METADATA_KEY_TITLE		"title"

/**
 * MAFW_METADATA_KEY_DURATION:
 *
 * Describes the duration of the item (in seconds).  Its value is an integer.
 */
#define MAFW_METADATA_KEY_DURATION		"duration"

/**
 * MAFW_METADATA_KEY_ARTIST:
 *
 * Describes the artist of the item.  Its value is a string.
 */
#define MAFW_METADATA_KEY_ARTIST		"artist"

/**
 * MAFW_METADATA_KEY_ALBUM:
 *
 * Describes the album of the item.  Its value is a string.
 */
#define MAFW_METADATA_KEY_ALBUM		"album"

/**
 * MAFW_METADATA_KEY_ORGANIZATION:
 *
 * Describes the organization of the item.  Its value is a string.
 */
#define MAFW_METADATA_KEY_ORGANIZATION		"organization"

/**
 * MAFW_METADATA_KEY_GENRE:
 *
 * Describes the genre of the item.  Its value is a string.
 */
#define MAFW_METADATA_KEY_GENRE		"genre"

/**
 * MAFW_METADATA_KEY_TRACK:
 *
 * Describes the track of the item.  Its value is an integer.
 */
#define MAFW_METADATA_KEY_TRACK		"track"

/**
 * MAFW_METADATA_KEY_YEAR:
 *
 * Describes the year of the item.  Its value is an integer.
 */
#define MAFW_METADATA_KEY_YEAR			"year"

/**
 * MAFW_METADATA_KEY_BITRATE:
 *
 * Describes the bitrate of the item.  Its value is an integer.
 */
#define MAFW_METADATA_KEY_BITRATE		"bitrate"

/**
 * MAFW_METADATA_KEY_COUNT:
 *
 * Describes how may times the item exists. Its value is an integer.
 */
#define MAFW_METADATA_KEY_COUNT		"count"

/**
 * MAFW_METADATA_KEY_PLAY_COUNT:
 *
 * Describes how may times the item has been played/viewed. Its value is an
 * integer.
 */
#define MAFW_METADATA_KEY_PLAY_COUNT		"play-count"

/**
 * MAFW_METADATA_KEY_LAST_PLAYED:
 *
 * Describes the time in epoch, when the item was played last time. Its value
 * is a long integer.
 */
#define MAFW_METADATA_KEY_LAST_PLAYED		"last-played"

/**
 * MAFW_METADATA_KEY_DESCRIPTION:
 *
 * A human readable description of the item. Its value is string.
 */
#define MAFW_METADATA_KEY_DESCRIPTION		"description"

/**
 * MAFW_METADATA_KEY_ENCODING:
 *
 * Describes how the item is encoded. Its value is a string.
 */
#define MAFW_METADATA_KEY_ENCODING		"encoding"

/**
 * MAFW_METADATA_KEY_ADDED:
 *
 * Date when item was added into the database. Its value is a long
 * integer.
 */
#define MAFW_METADATA_KEY_ADDED		"added"

/**
 * MAFW_METADATA_KEY_THUMBNAIL_URI:
 *
 * URI pointing to a thumbnail. Its value is a string.
 */
#define MAFW_METADATA_KEY_THUMBNAIL_URI	"thumbnail-uri"

/**
 * MAFW_METADATA_KEY_THUMBNAIL_SMALL_URI:
 *
 * URI pointing to a small thumbnail. Its value is a string.
 */
#define MAFW_METADATA_KEY_THUMBNAIL_SMALL_URI	"thumbnail-small-uri"

/**
 * MAFW_METADATA_KEY_THUMBNAIL_MEDIUM_URI:
 *
 * URI pointing to a medium thumbnail. Its value is a string.
 */
#define MAFW_METADATA_KEY_THUMBNAIL_MEDIUM_URI	"thumbnail-medium-uri"

/**
 * MAFW_METADATA_KEY_THUMBNAIL_LARGE_URI:
 *
 * URI pointing to a large thumbnail. Its value is a string.
 */
#define MAFW_METADATA_KEY_THUMBNAIL_LARGE_URI	"thumbnail-large-uri"

/**
 * MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI:
 *
 * URI pointing to the thumbnail of the frame where video was
 * paused. Its value is a string.
 */
#define MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI   "paused-thumbnail-uri"

/**
 * MAFW_METADATA_KEY_PAUSED_POSITION:
 *
 * Position where the item is paused. Its value is an integer.
 */
#define MAFW_METADATA_KEY_PAUSED_POSITION   "paused-position"

/**
 * MAFW_METADATA_KEY_THUMBNAIL:
 *
 * Image thumbnail. Its value is a byte array.
 */
#define MAFW_METADATA_KEY_THUMBNAIL		"thumbnail"

/**
 * MAFW_METADATA_KEY_IS_SEEKABLE:
 *
 * Describes if item is seekable. Its value is a boolean.
 */
#define MAFW_METADATA_KEY_IS_SEEKABLE		"is-seekable"

/**
 * MAFW_METADATA_KEY_RES_X:
 *
 * Describes the horizontal resolution of the item. Its value is an integer.
 */
#define MAFW_METADATA_KEY_RES_X		"res-x"

/**
 * MAFW_METADATA_KEY_RES_Y:
 *
 * Describes the vertical resolution of the item. Its value is an integer.
 */
#define MAFW_METADATA_KEY_RES_Y		"res-y"

/**
 * MAFW_METADATA_KEY_COMMENT:
 *
 * Comment describing the item. Its value is a string.
 */
#define MAFW_METADATA_KEY_COMMENT		"comment"

/**
 * MAFW_METADATA_KEY_TAGS:
 *
 * List of tags describing the item. Its value is a list of strings.
 */
#define MAFW_METADATA_KEY_TAGS			"tags"

/**
 * MAFW_METADATA_KEY_DIDL:
 *
 * Item metadata in DIDL lite format. Its value is a string.
 */
#define MAFW_METADATA_KEY_DIDL			"didl"

/**
 * MAFW_METADATA_KEY_ARTIST_INFO_URI:
 *
 * An URI pointing to artist description. Its value is a string.
 */
#define MAFW_METADATA_KEY_ARTIST_INFO_URI	"artist-info-uri"

/**
 * MAFW_METADATA_KEY_ALBUM_INFO_URI:
 *
 * An URI pointing to album information. Its value is an integer.
 */
#define MAFW_METADATA_KEY_ALBUM_INFO_URI	"album-info-uri"

/**
 * MAFW_METADATA_KEY_LYRICS_URI:
 *
 * An URI pointing to lyrics related to item. Its value is a string.
 */
#define MAFW_METADATA_KEY_LYRICS_URI		"lyrics-uri"

/**
 * MAFW_METADATA_KEY_LYRICS:
 *
 * Item lyrics. Its value is a string.
 */
#define MAFW_METADATA_KEY_LYRICS		"lyrics"

/**
 * MAFW_METADATA_KEY_RATING:
 *
 * Describes the rating of the item. Its value is an integer.
 */
#define MAFW_METADATA_KEY_RATING		"rating"

/**
 * MAFW_METADATA_KEY_COMPOSER:
 *
 * Describes the composer of the item. Its value is a string.
 */
#define MAFW_METADATA_KEY_COMPOSER		"composer"

/**
 * MAFW_METADATA_KEY_FILENAME:
 *
 * Original filename of the item resource. Its value is a string.
 */
#define MAFW_METADATA_KEY_FILENAME		"filename"

/**
 * MAFW_METADATA_KEY_FILESIZE:
 *
 * Size (in bytes) of the resource file pointed by the item. Its value is
 * an integer.
 */
#define MAFW_METADATA_KEY_FILESIZE		"filesize"

/**
 * MAFW_METADATA_KEY_COPYRIGHT:
 *
 * Copyright disclaimer. Its value is a string.
 */
#define MAFW_METADATA_KEY_COPYRIGHT		"copyright"

/**
 * MAFW_METADATA_KEY_PROTOCOL_INFO:
 *
 * Describes how may times the item exists. Its value is a string.
 * Specified in UPnP ConnectionManager documentation. 
 */
#define MAFW_METADATA_KEY_PROTOCOL_INFO	"protocol-info"

/**
 * MAFW_METADATA_KEY_AUDIO_BITRATE:
 *
 * The bitrate used to encode the item. Its value is an integer.
 */
#define MAFW_METADATA_KEY_AUDIO_BITRATE	"audio-bitrate"

/**
 * MAFW_METADATA_KEY_AUDIO_CODEC:
 *
 * The coder/decoder information for item. Its value is a string.
 */
#define MAFW_METADATA_KEY_AUDIO_CODEC		"audio-codec"

/**
 * MAFW_METADATA_KEY_ALBUM_ART_URI:
 *
 * An URI pointing to the original album art image. Its value is a string.
 */
#define MAFW_METADATA_KEY_ALBUM_ART_URI	"album-art-uri"

/**
 * MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI:
 *
 * An URI pointing to small album art image. Its value is a string.
 */
#define MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI	"album-art-small-uri"

/**
 * MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI:
 *
 * An URI pointing to medium album art image. Its value is a string.
 */
#define MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI	"album-art-medium-uri"

/**
 * MAFW_METADATA_KEY_ALBUM_ART_LARGE_URI:
 *
 * An URI pointing to large album art image. Its value is a string.
 */
#define MAFW_METADATA_KEY_ALBUM_ART_LARGE_URI	"album-art-large-uri"

/**
 * MAFW_METADATA_KEY_ALBUM_ART:
 *
 * Album art image. Its value is a byte array.
 */
#define MAFW_METADATA_KEY_ALBUM_ART		"album-art"

/**
 * MAFW_METADATA_KEY_RENDERER_ART_URI:
 *
 * Album art image uri coming from renderer. Its value is a string.
 */
#define MAFW_METADATA_KEY_RENDERER_ART_URI	"renderer-art-uri"

/**
 * MAFW_METADATA_KEY_VIDEO_BITRATE:
 *
 * The bitrate used to encode the item. Its value is an integer.
 */
#define MAFW_METADATA_KEY_VIDEO_BITRATE	"video-bitrate"

/**
 * MAFW_METADATA_KEY_VIDEO_CODEC:
 *
 * The coder/decoder information for item. Its value is a string.
 */
#define MAFW_METADATA_KEY_VIDEO_CODEC		"video-codec"

/**
 * MAFW_METADATA_KEY_VIDEO_FRAMERATE:
 *
 * The amount of frames per second. Its value is a float.
 */
#define MAFW_METADATA_KEY_VIDEO_FRAMERATE	"video-framerate"

/**
 * MAFW_METADATA_KEY_VIDEO_SOURCE:
 *
 * Information about the source of the video. Provides information
 * like, for example, the hardware used to record it. Its value is a
 * string.
 */
#define MAFW_METADATA_KEY_VIDEO_SOURCE         "video-source"

/**
 * MAFW_METADATA_KEY_BPP:
 *
 * Describes how many bits are used to represent a pixel. Its value is an
 * integer.
 */
#define MAFW_METADATA_KEY_BPP			"bpp"

/**
 * MAFW_METADATA_KEY_EXIF_XML:
 *
 * Exif information in XML format. Its value is a string.
 */
#define MAFW_METADATA_KEY_EXIF_XML		"exif-xml"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT:
 * @n: at which level should the count be computed. First is level 1
 * (children)
 *
 * Describes the amount of child items the item has at level
 * @n. #MAFW_METADATA_KEY_CHILDCOUNT(1) is the number of chldren,
 * #MAFW_METADATA_KEY_CHILDCOUNT(2) is the number of grandchildren, and so
 * on. Its value is an integer. Everytime this macro is invoked, new memory is
 * created. User must free it when no needed.
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT(n)		g_strdup_printf("childcount(%d)", (n))

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_1:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(1).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_1          "childcount(1)"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_2:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(2).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_2          "childcount(2)"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_3:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(3).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_3          "childcount(3)"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_4:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(4).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_4          "childcount(4)"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_5:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(5).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_5          "childcount(5)"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_6:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(6).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_6          "childcount(6)"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_7:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(7).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_7          "childcount(7)"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_8:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(8).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_8          "childcount(8)"

/**
 * MAFW_METADATA_KEY_CHILDCOUNT_9:
 *
 * Static version of MAFW_METADATA_KEY_CHILDCOUNT(9).
 *
 */
#define MAFW_METADATA_KEY_CHILDCOUNT_9          "childcount(9)"

/**
 * MAFW_METADATA_KEY_ICON_URI:
 *
 * An URI pointing to icon image. Its value is a string.
 */
#define MAFW_METADATA_KEY_ICON_URI		"icon-uri"

/**
 * MAFW_METADATA_KEY_ICON:
 *
 * Icon image. Its value is a byte array.
 */
#define MAFW_METADATA_KEY_ICON			"icon"

/* Type definitions */
/**
 * MafwMetadataComparator:
 * @rel: the filter
 * @key: the key
 * @lhsgv: left argument
 * @rshgv: right argument
 *
 * Prototype of a function comparing metadata values of @key.  @rel is a
 * relation to be asserted, either #mafw_f_eq, #mafw_f_approx, #mafw_f_lt
 * or #mafw_f_gt.  @lhsgv is the left-hand side of the relation, while
 * @rhsgv is the right-hand side.  The #GValue:s have the same #G_VALUE_TYPE.
 *
 * Returns: %TRUE rel is asserted.
 */
typedef gboolean (*MafwMetadataComparator)(MafwFilterType rel,
					   const gchar *key,
					   const GValue *lhsgv,
					   const GValue *rshgv);

G_BEGIN_DECLS

/* Function prototypes */
extern GHashTable *mafw_metadata_new(void);
extern void mafw_metadata_release(GHashTable *md);
extern void mafw_metadata_add_something(GHashTable *md, const gchar *key,
					GType argvtype, guint nvalues, ...);

extern guint mafw_metadata_nvalues(gconstpointer value);
extern GValue *mafw_metadata_first(GHashTable *md, const gchar *key);

extern void mafw_metadata_print_one(const gchar *key, gpointer val,
				    const gchar *domain);
extern void mafw_metadata_print(GHashTable *md, const gchar *domain);

extern gchar **mafw_metadata_sorting_terms(const gchar *sorting);
extern const gchar **mafw_metadata_relevant_keys(const gchar *const *keys,
						 const MafwFilter *filter,
						 const gchar *const *sorting);

extern gboolean mafw_metadata_ordered(MafwFilterType rel, const gchar *key,
				      const GValue *lhsgv, const GValue *rhsgv);
extern gboolean mafw_metadata_filter(GHashTable *md, const MafwFilter *filter,
				     MafwMetadataComparator funcomp);
extern gint mafw_metadata_compare(GHashTable *md1, GHashTable *md2,
				  const gchar *const *terms,
				  MafwMetadataComparator funcomp);

G_END_DECLS

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
#endif
