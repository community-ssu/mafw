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

#ifndef _MAFW_TRACKER_SOURCE_DEFINITIONS_H_
#define  _MAFW_TRACKER_SOURCE_DEFINITIONS_H_

/* Nodes in the tracker source path */
#define TRACKER_SOURCE_ARTISTS   "artists"
#define TRACKER_SOURCE_ALBUMS    "albums"
#define TRACKER_SOURCE_SONGS     "songs"
#define TRACKER_SOURCE_GENRES    "genres"
#define TRACKER_SOURCE_MUSIC     "music"
#define TRACKER_SOURCE_PLAYLISTS "playlists"
#define TRACKER_SOURCE_VIDEOS    "videos"

/* Metadata "Title" for fixed categories */
#define ROOT_TITLE                           "Root"
#define ROOT_VIDEOS_TITLE                    "Videos"
#define ROOT_MUSIC_TITLE                     "Music"
#define ROOT_MUSIC_ARTISTS_TITLE             "Artists"
#define ROOT_MUSIC_SONGS_TITLE               "Songs"
#define ROOT_MUSIC_ALBUMS_TITLE              "Albums"
#define ROOT_MUSIC_GENRES_TITLE              "Genres"
#define ROOT_MUSIC_PLAYLISTS_TITLE           "Playlists"

#define KNOWN_METADATA_KEYS                                             \
        MAFW_METADATA_KEY_URI,						\
		MAFW_METADATA_KEY_MIME,                                 \
                MAFW_METADATA_KEY_TITLE,				\
		MAFW_METADATA_KEY_DURATION,				\
                MAFW_METADATA_KEY_ARTIST,				\
		MAFW_METADATA_KEY_ALBUM,				\
                MAFW_METADATA_KEY_GENRE,				\
		MAFW_METADATA_KEY_TRACK,				\
                MAFW_METADATA_KEY_YEAR,                                 \
		MAFW_METADATA_KEY_BITRATE,				\
                MAFW_METADATA_KEY_COUNT,				\
		MAFW_METADATA_KEY_PLAY_COUNT,				\
                MAFW_METADATA_KEY_LAST_PLAYED,                          \
                MAFW_METADATA_KEY_DESCRIPTION,                          \
                MAFW_METADATA_KEY_ENCODING,				\
		MAFW_METADATA_KEY_ADDED,				\
                MAFW_METADATA_KEY_MODIFIED,                             \
                MAFW_METADATA_KEY_THUMBNAIL_URI,                        \
                MAFW_METADATA_KEY_THUMBNAIL_SMALL_URI,			\
                MAFW_METADATA_KEY_THUMBNAIL_MEDIUM_URI,                 \
                MAFW_METADATA_KEY_THUMBNAIL_LARGE_URI,			\
                MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI,                 \
                MAFW_METADATA_KEY_PAUSED_POSITION,                      \
                MAFW_METADATA_KEY_THUMBNAIL,				\
		MAFW_METADATA_KEY_IS_SEEKABLE,				\
                MAFW_METADATA_KEY_RES_X,				\
                MAFW_METADATA_KEY_RES_Y,				\
		MAFW_METADATA_KEY_COMMENT,				\
                MAFW_METADATA_KEY_TAGS,                                 \
		MAFW_METADATA_KEY_DIDL,                                 \
                MAFW_METADATA_KEY_ARTIST_INFO_URI,                      \
                MAFW_METADATA_KEY_ALBUM_INFO_URI,                       \
                MAFW_METADATA_KEY_LYRICS_URI,				\
		MAFW_METADATA_KEY_LYRICS,				\
                MAFW_METADATA_KEY_RATING,				\
		MAFW_METADATA_KEY_COMPOSER,				\
                MAFW_METADATA_KEY_FILENAME,				\
		MAFW_METADATA_KEY_FILESIZE,				\
                MAFW_METADATA_KEY_COPYRIGHT,                            \
                MAFW_METADATA_KEY_PROTOCOL_INFO,                        \
                MAFW_METADATA_KEY_AUDIO_BITRATE,                        \
                MAFW_METADATA_KEY_AUDIO_CODEC,                          \
                MAFW_METADATA_KEY_ALBUM_ART_URI,			\
                MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI,                  \
                MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI,                 \
                MAFW_METADATA_KEY_ALBUM_ART_LARGE_URI,                  \
                MAFW_METADATA_KEY_ALBUM_ART,                            \
                MAFW_METADATA_KEY_VIDEO_BITRATE,                        \
                MAFW_METADATA_KEY_VIDEO_CODEC,                          \
                MAFW_METADATA_KEY_VIDEO_FRAMERATE,                      \
                MAFW_METADATA_KEY_VIDEO_SOURCE,                         \
		MAFW_METADATA_KEY_BPP,					\
                MAFW_METADATA_KEY_EXIF_XML,				\
                MAFW_METADATA_KEY_ICON_URI,				\
		MAFW_METADATA_KEY_ICON,                                 \
                MAFW_METADATA_KEY_CHILDCOUNT_1,                         \
                MAFW_METADATA_KEY_CHILDCOUNT_2,                         \
                MAFW_METADATA_KEY_CHILDCOUNT_3,                         \
                MAFW_METADATA_KEY_CHILDCOUNT_4

/* Known mdata key flags */
#define	MTrackerSrc_KEY_URI			G_GUINT64_CONSTANT(0x1)
#define	MTrackerSrc_KEY_MIME			G_GUINT64_CONSTANT(0x2)
#define MTrackerSrc_KEY_TITLE			G_GUINT64_CONSTANT(0x4)
#define	MTrackerSrc_KEY_DURATION		G_GUINT64_CONSTANT(0x8)
#define MTrackerSrc_KEY_ARTIST			G_GUINT64_CONSTANT(0x10)
#define MTrackerSrc_KEY_ALBUM			G_GUINT64_CONSTANT(0x20)
#define MTrackerSrc_KEY_GENRE			G_GUINT64_CONSTANT(0x40)
#define MTrackerSrc_KEY_TRACK			G_GUINT64_CONSTANT(0x80)
#define MTrackerSrc_KEY_YEAR			G_GUINT64_CONSTANT(0x100)
#define MTrackerSrc_KEY_BITRATE			G_GUINT64_CONSTANT(0x200)
#define MTrackerSrc_KEY_PLAY_COUNT		G_GUINT64_CONSTANT(0x400)
#define MTrackerSrc_KEY_LAST_PLAYED		G_GUINT64_CONSTANT(0x800)
#define MTrackerSrc_KEY_ADDED			G_GUINT64_CONSTANT(0x1000)
#define MTrackerSrc_KEY_MODIFIED		G_GUINT64_CONSTANT(0x2000)
#define MTrackerSrc_KEY_THUMBNAIL_URI		G_GUINT64_CONSTANT(0x4000)
#define MTrackerSrc_KEY_THUMBNAIL_SMALL_URI	G_GUINT64_CONSTANT(0x8000)
#define MTrackerSrc_KEY_THUMBNAIL_MEDIUM_URI	G_GUINT64_CONSTANT(0x10000)
#define MTrackerSrc_KEY_THUMBNAIL_LARGE_URI	G_GUINT64_CONSTANT(0x20000)
#define MTrackerSrc_KEY_PAUSED_THUMBNAIL_URI	G_GUINT64_CONSTANT(0x40000)
#define MTrackerSrc_KEY_PAUSED_POSITION		G_GUINT64_CONSTANT(0x80000)
#define MTrackerSrc_KEY_RES_X			G_GUINT64_CONSTANT(0x100000)
#define MTrackerSrc_KEY_RES_Y			G_GUINT64_CONSTANT(0x200000)
#define MTrackerSrc_KEY_FILENAME		G_GUINT64_CONSTANT(0x400000)
#define MTrackerSrc_KEY_FILESIZE		G_GUINT64_CONSTANT(0x800000)
#define MTrackerSrc_KEY_COPYRIGHT		G_GUINT64_CONSTANT(0x1000000)
#define MTrackerSrc_KEY_ALBUM_ART_URI		G_GUINT64_CONSTANT(0x2000000)
#define MTrackerSrc_KEY_ALBUM_ART_SMALL_URI	G_GUINT64_CONSTANT(0x4000000)
#define MTrackerSrc_KEY_ALBUM_ART_MEDIUM_URI	G_GUINT64_CONSTANT(0x8000000)
#define MTrackerSrc_KEY_ALBUM_ART_LARGE_URI	G_GUINT64_CONSTANT(0x10000000)
#define MTrackerSrc_KEY_VIDEO_FRAMERATE		G_GUINT64_CONSTANT(0x20000000)
#define MTrackerSrc_KEY_VIDEO_SOURCE		G_GUINT64_CONSTANT(0x40000000)
#define MTrackerSrc_KEY_CHILDCOUNT_1		G_GUINT64_CONSTANT(0x80000000)
#define MTrackerSrc_KEY_CHILDCOUNT_2		G_GUINT64_CONSTANT(0x100000000)
#define MTrackerSrc_KEY_CHILDCOUNT_3		G_GUINT64_CONSTANT(0x200000000)
#define MTrackerSrc_KEY_CHILDCOUNT_4		G_GUINT64_CONSTANT(0x400000000)
#define MTrackerSrc_TRACKER_PKEY_VALID_DURATION	G_GUINT64_CONSTANT(0x800000000)
#define MTrackerSrc_TRACKER_FKEY_PATH		G_GUINT64_CONSTANT(0x1000000000)

/* Known mdata key ids */
#define	MTrackerSrc_ID_URI			0
#define	MTrackerSrc_ID_MIME			1
#define MTrackerSrc_ID_TITLE			2
#define	MTrackerSrc_ID_DURATION			3
#define MTrackerSrc_ID_ARTIST			4
#define MTrackerSrc_ID_ALBUM			5
#define MTrackerSrc_ID_GENRE			6
#define MTrackerSrc_ID_TRACK			7
#define MTrackerSrc_ID_YEAR			8
#define MTrackerSrc_ID_BITRATE			9
#define MTrackerSrc_ID_PLAY_COUNT		10
#define MTrackerSrc_ID_LAST_PLAYED		11
#define MTrackerSrc_ID_ADDED			12
#define MTrackerSrc_ID_MODIFIED			13
#define MTrackerSrc_ID_THUMBNAIL_URI		14
#define MTrackerSrc_ID_THUMBNAIL_SMALL_URI	15
#define MTrackerSrc_ID_THUMBNAIL_MEDIUM_URI	16
#define MTrackerSrc_ID_THUMBNAIL_LARGE_URI	17
#define MTrackerSrc_ID_PAUSED_THUMBNAIL_URI	18
#define MTrackerSrc_ID_PAUSED_POSITION		19
#define MTrackerSrc_ID_RES_X			20
#define MTrackerSrc_ID_RES_Y			21
#define MTrackerSrc_ID_FILENAME			22
#define MTrackerSrc_ID_FILESIZE			23
#define MTrackerSrc_ID_COPYRIGHT		24
#define MTrackerSrc_ID_ALBUM_ART_URI		25
#define MTrackerSrc_ID_ALBUM_ART_SMALL_URI	26
#define MTrackerSrc_ID_ALBUM_ART_MEDIUM_URI	27
#define MTrackerSrc_ID_ALBUM_ART_LARGE_URI	28
#define MTrackerSrc_ID_VIDEO_FRAMERATE		29
#define MTrackerSrc_ID_VIDEO_SOURCE		30
#define MTrackerSrc_ID_CHILDCOUNT_1		31
#define MTrackerSrc_ID_CHILDCOUNT_2		32
#define MTrackerSrc_ID_CHILDCOUNT_3		33
#define MTrackerSrc_ID_CHILDCOUNT_4		34

/* Flag for date types */
#define MTrackerSrc_Date_Types (MTrackerSrc_KEY_YEAR | MTrackerSrc_KEY_LAST_PLAYED | \
				MTrackerSrc_KEY_ADDED | MTrackerSrc_KEY_MODIFIED)
/* Tracker metadata keys */
#define TRACKER_AKEY_ALBUM            "Audio:Album"
#define TRACKER_AKEY_ARTIST           "Audio:Artist"
#define TRACKER_AKEY_BITRATE          "Audio:Bitrate"
#define TRACKER_AKEY_DURATION         "Audio:Duration"
#define TRACKER_AKEY_GENRE            "Audio:Genre"
#define TRACKER_AKEY_LAST_PLAYED      "Audio:LastPlay"
#define TRACKER_AKEY_PLAY_COUNT       "Audio:PlayCount"
#define TRACKER_AKEY_TITLE            "Audio:Title"
#define TRACKER_AKEY_TRACK            "Audio:TrackNo"
#define TRACKER_AKEY_YEAR             "Audio:ReleaseDate"
#define TRACKER_FKEY_ADDED            "File:Added"
#define TRACKER_FKEY_MODIFIED         "File:Modified"
#define TRACKER_FKEY_COPYRIGHT        "File:Copyright"
#define TRACKER_FKEY_FILENAME         "File:Name"
#define TRACKER_FKEY_FILESIZE         "File:Size"
#define TRACKER_FKEY_FULLNAME         "File:NameDelimited"
#define TRACKER_FKEY_MIME             "File:Mime"
#define TRACKER_FKEY_PATH             "File:Path"
#define TRACKER_PKEY_COUNT            "Playlist:Songs"
#define TRACKER_PKEY_DURATION         "Playlist:Duration"
#define TRACKER_PKEY_VALID_DURATION   "Playlist:ValidDuration"
#define TRACKER_VKEY_DURATION         "Video:Duration"
#define TRACKER_VKEY_FRAMERATE        "Video:FrameRate"
#define TRACKER_VKEY_PAUSED_POSITION  "Video:PausePosition"
#define TRACKER_VKEY_PAUSED_THUMBNAIL "Video:LastPlayedFrame"
#define TRACKER_VKEY_RES_X            "Video:Width"
#define TRACKER_VKEY_RES_Y            "Video:Height"
#define TRACKER_VKEY_SOURCE           "Video:Source"
#define TRACKER_VKEY_TITLE            "Video:Title"

#define RDF_QUERY_BEGIN \
	"<rdfq:Condition>"

#define RDF_QUERY_END \
	"</rdfq:Condition>"

#define RDF_QUERY_AND_BEGIN \
        RDF_QUERY_BEGIN                                                 \
	"  <rdfq:and>"							\

#define RDF_QUERY_AND_END \
	"  </rdfq:and>"							\
        RDF_QUERY_END

#define RDF_QUERY_BY_ARTIST						\
	"  <rdfq:equals>"						\
	"    <rdfq:Property name=\"Audio:Artist\"/>"			\
	"    <rdf:String>%s</rdf:String>"				\
	"  </rdfq:equals>"						\

#define RDF_QUERY_BY_ALBUM \
	"  <rdfq:equals>"						\
	"    <rdfq:Property name=\"Audio:Album\"/>"			\
	"    <rdf:String>%s</rdf:String>"				\
	"  </rdfq:equals>"						\

#define RDF_QUERY_BY_GENRE \
	"  <rdfq:equals>"						\
	"    <rdfq:Property name=\"Audio:Genre\"/>"			\
	"    <rdf:String>%s</rdf:String>"				\
	"  </rdfq:equals>"						\

#define RDF_QUERY_BY_FILE						\
	"  <rdfq:equals>"						\
	"    <rdfq:Property name=\"File:NameDelimited\"/>"		\
	"    <rdf:String>%s</rdf:String>"				\
	"  </rdfq:equals>"						\

#define RDF_QUERY_BY_FILE_SET						\
	"  <rdfq:inSet>"						\
	"    <rdfq:Property name=\"File:NameDelimited\"/>"		\
	"    <rdf:String>%s</rdf:String>"				\
	"  </rdfq:inSet>"						\

/* Some object identifiers */
#define VIDEOS_OBJECT_ID     MAFW_TRACKER_SOURCE_UUID "::videos"
#define MUSIC_OBJECT_ID      MAFW_TRACKER_SOURCE_UUID "::music"
#define PLAYLISTS_OBJECT_ID  MAFW_TRACKER_SOURCE_UUID "::music/playlists"

/* Private data of MAFW_TRACKER_SOURCE */
struct _MafwTrackerSourcePrivate {
        /* A List of pending browse operations */
        GList *pending_browse_ops;
        /* Last value of update progress */
        gint last_progress;
        /* Number of processed items when updating */
        gint processed_items;
        /* Number of remaining items to be processed when updating */
        gint remaining_items;
        /* Remaining time (in seconds) to finish the update */
        gint remaining_time;
};

#endif				/* _MAFW_TRACKER_SOURCE_DEFINITIONS_H_ */
