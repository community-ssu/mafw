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

#include <string.h>

#include "mafw-tracker-source.h"
#include "tracker-iface.h"
#include "util.h"
#include "definitions.h"
#include "key-mapping.h"

struct _metadatas_common_closure {
	/* Source instance */
	MafwSource *source;
	/* Metadatas */
        GHashTable *metadatas;
	/* The user callback used to emit the browse results to the user */
	MafwSourceMetadataResultsCb callback;
	/* User data for the user callback  */
	gpointer user_data;
        /* Error obtained */
        GError *error;
        /* How many elements remains to insert in metadatas */
        gint remaining;
	guint64 asked_keys;
};

struct _metadatas_closure {
        /* Shared struct */
        struct _metadatas_common_closure *common;
	/* The objectid to get metadata from */
        union {
                gchar *object_id;
                GList *object_ids;
        };
	/* Metadata values obtained */
	GHashTable *metadata_value;
};

struct _metadata_closure {
        /* Object id */
        gchar *object_id;
        /* User's callback */
        MafwSourceMetadataResultCb cb;
        /* User's data */
        gpointer user_data;
};

struct _update_metadata_closure {
        /* Source instance */
        MafwSource *source;
        /* The objectid to be updated */
        gchar *object_id;
        /* Metadata keys&values to be set */
        GHashTable *metadata;
        /* The user callback to notify what has happened */
        MafwSourceMetadataSetCb cb;
        /* User data for callback */
        gpointer user_data;
        /* The clip to be updated */
        gchar *clip;
	CategoryType category;
};

typedef gboolean (*_GetMetadataFunc)(struct _metadatas_closure *mc,
				     GList *child,
				     GError **error);

static void _metadatas_closure_free(gpointer data)
{
        struct _metadatas_common_closure *mcc;

        mcc = (struct _metadatas_common_closure *) data;

        /* Free metadatas */
        g_hash_table_unref(mcc->metadatas);

        /* Free error */
        if (mcc->error) {
                g_error_free(mcc->error);
        }

        /* Free common structure */
        g_slice_free(struct _metadatas_common_closure, mcc);
}

static gboolean _emit_metadatas_results_idle(gpointer data)
{
	struct _metadatas_common_closure *mcc;

	mcc = (struct _metadatas_common_closure *) data;

	mcc->callback(mcc->source,
                      mcc->metadatas,
                      mcc->user_data,
                      mcc->error);

	return FALSE;
}

static void _emit_metadatas_results(struct _metadatas_common_closure *mcc)
{
	/* Emit results */
	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
			(GSourceFunc) _emit_metadatas_results_idle,
			mcc,
			(GDestroyNotify) _metadatas_closure_free);
}

static void _get_metadata_tracker_cb(GHashTable *result,
                                     GError *error,
                                     gpointer user_data)
{
        struct _metadatas_closure *mc = (struct _metadatas_closure *) user_data;

        /* Save error */
        if (error) {
               /* This error, if emitted with send_error is freed after the callback
                  chain, so we have to make a copy when we add it to the closure */
                if (!mc->common->error) {
                        mc->common->error = g_error_copy(error);
                }
        }

        /* Save result */
	if (result)
        	g_hash_table_insert(mc->common->metadatas,
                            mc->object_id,
                            result);
        mc->common->remaining--;

        /* If there aren't more elements, send results */
        if (mc->common->remaining == 0) {
                _emit_metadatas_results(mc->common);
        }

        /* Free closure */
        g_slice_free(struct _metadatas_closure, mc);
}

static void _get_metadatas_tracker_cb(GList *results,
                                      GError *error,
                                      gpointer user_data)
{
        struct _metadatas_closure *mc = (struct _metadatas_closure *) user_data;
        GList *current_obj;
        GList *current_result;

        /* Save error */
        if (error) {
                if (!mc->common->error) {
			/* TODO: Check if copying this error causes leaks in
			 some parts of the code. If that's the case fix them. */
                        mc->common->error =  g_error_copy(error);
                } else {
                        g_error_free(error);
                }
        }

        /* Save results */
        current_obj = mc->object_ids;
        current_result = results;

        while (current_obj && current_result) {
                /* Check there's a hashtable for this object id */
                if (current_result->data) {
                        g_hash_table_insert(
                                mc->common->metadatas,
                                current_obj->data,
                                g_hash_table_ref(current_result->data));
                }
                current_obj = current_obj->next;
                current_result = current_result->next;
                mc->common->remaining--;
        }

        /* If there aren't more elements or there was an error, send results */
        if ((mc->common->remaining == 0) || error) {
                _emit_metadatas_results(mc->common);
        }

        /* Free closure */
        g_list_free(mc->object_ids);
        g_slice_free(struct _metadatas_closure, mc);
}

static gboolean _calculate_duration_is_needed(GHashTable *metadata,
					      guint64 *metadata_keys,
					      const gchar *object_id)
{
	CategoryType category;
	gchar *pls_uri = NULL;
	gboolean calculate = FALSE;

	if (util_is_duration_requested(*metadata_keys)) {
		category = util_extract_category_info(object_id,
						      NULL,
						      NULL,
						      NULL,
						      &pls_uri);

		if (category == CATEGORY_MUSIC_PLAYLISTS) {
			if (pls_uri) {
				/* Single playlist. */
			        calculate =
					util_calculate_playlist_duration_is_needed(
						metadata);
				/* Remove the non-Mafw data used to check if
				   MAFW has to calculate the playlist
				   duration. */
				*metadata_keys = util_remove_tracker_data_to_check_pls_duration(
					metadata, *metadata_keys);

				g_free(pls_uri);
			} else {
				/* Playlists category. When duration is requested,
				   calculate it always. */
				calculate = TRUE;
			}
		}
	}
	return calculate;
}


static void _add_playlist_duration_cb(MafwSource *self,
				      guint browse_id,
				      gint remaining_count,
				      guint index,
				      const gchar *object_id,
				      GHashTable *metadata,
				      gpointer user_data,
				      const GError *error)
{
	GValue *gval = NULL;
        gint pls_duration;

	struct _metadatas_closure *pls_mc =
		(struct _metadatas_closure *) user_data;

	/* Get the calculated duration and add it to the results. */
	if (!error && metadata) {
		gval = mafw_metadata_first(
			metadata,
			MAFW_METADATA_KEY_DURATION);

		if (gval) {
			pls_duration = 	g_value_get_int(gval);

			gval = mafw_metadata_first(
				pls_mc->metadata_value,
				MAFW_METADATA_KEY_DURATION);

			if (gval) {
				g_value_set_int(gval, pls_duration);
			} else {
				mafw_metadata_add_int(
                                        pls_mc->metadata_value,
                                        MAFW_METADATA_KEY_DURATION,
                                        pls_duration);
			}
		}
	}

	_get_metadata_tracker_cb(pls_mc->metadata_value, (GError *) error,
				 user_data);

	mafw_metadata_release(metadata);
}

static void _get_metadata_tracker_from_playlist_cb(GHashTable *result,
						   GError *error,
						   gpointer user_data)
{
	if (!error && result) {
		struct _metadatas_closure *mc =
			(struct _metadatas_closure *) user_data;

		/* Check if we need to calculate the duration exhaustively. */
		if (_calculate_duration_is_needed(
			    result,
			    &mc->common->asked_keys,
			    mc->object_id)) {

			/* Store the result in the closure. */
			mc->metadata_value = result;

			/* Calculate the duration and add it to the MAFW
			   results. */
			mafw_tracker_source_get_playlist_duration(
				mc->common->source,
				mc->object_id,
				_add_playlist_duration_cb,
				(gpointer) mc);

			return;
		}
	}

	_get_metadata_tracker_cb(result, error, user_data);
}

static void _get_metadatas_tracker_from_playlist_cb(GList *results,
                                                    GError *error,
                                                    gpointer user_data)
{
        GList *current_obj;
        GList *current_result;
        struct _metadatas_closure *nmc = NULL;
        struct _metadatas_closure *mc = (struct _metadatas_closure *) user_data;

        /* Save error */
        if (error) {
                if (!mc->common->error) {
			/* TODO: Check if copying this error causes leaks in
			 some parts of the code. If that's the case fix them. */
                        mc->common->error = g_error_copy(error);
                } else {
                        g_error_free(error);
                }

		/* Emit, there aren't any results. */
		_emit_metadatas_results(mc->common);

		/* Free closure */
		g_list_foreach(mc->object_ids, (GFunc) g_free, NULL);
		g_list_free(mc->object_ids);
		g_slice_free(struct _metadatas_closure, mc);

		return;
        }


        /* For each result, check if we need to calculate the duration
         * exhaustively. If not, save the result*/
        current_obj = mc->object_ids;
        current_result = results;
        while (current_obj && current_result) {
                if (_calculate_duration_is_needed(current_result->data,
                                                  &mc->common->asked_keys,
                                                  current_obj->data)) {
                        /* Create a new closure to store data */
                        nmc = g_slice_new(struct _metadatas_closure);
                        nmc->object_id = current_obj->data;
                        nmc->metadata_value = g_hash_table_ref(current_result->data);
                        nmc->common = mc->common;

			/* If there aren't more playlists (remaining == 0), this
			   will send the results after calculating the duration
			   of the playlist. So, don't call emit_metadatas_results
			   in this case. */
                        mafw_tracker_source_get_playlist_duration(
                                nmc->common->source,
                                nmc->object_id,
                                _add_playlist_duration_cb,
                                nmc);
                } else {
                        g_hash_table_insert(
                                mc->common->metadatas,
                                current_obj->data,
                                g_hash_table_ref(current_result->data));
                        mc->common->remaining--;

			/* If there aren't more playlists, send results. */
			if (mc->common->remaining == 0) {
				_emit_metadatas_results(mc->common);
			}
                }

                current_obj = current_obj->next;
                current_result = current_result->next;
        }

        /* Free closure */
	/* The elements of the object_ids list were added to the metadatas
	   results and will be released when freeing the common closure. */
        g_list_free(mc->object_ids);
        g_slice_free(struct _metadatas_closure, mc);
}

static void _get_metadata_tracker_from_music_cb(GHashTable *result,
						GError *error,
						gpointer user_data)
{
        GValue *gval;

        if (result) {
                /* The count on 'music' category is 5: albums,
                 * artists, genres, songs and playlists */
                gval = mafw_metadata_first(result,
                                           MAFW_METADATA_KEY_CHILDCOUNT_1);
                if (gval) {
                        g_value_set_int(gval, 5);
                }
        }

        _get_metadata_tracker_cb(result, error, user_data);
}

static void _get_metadata_tracker_from_root_videos_cb(GHashTable *result,
                                                      GError *error,
                                                      gpointer user_data)
{
        GValue *gvalcur;
        GValue *gvalnew;
        struct _metadatas_closure *mc = (struct _metadatas_closure *) user_data;

        /* If there are previous results, aggregate durations */
        if (result) {
                if (mc->metadata_value) {
                        gvalcur = mafw_metadata_first(
                                result,
                                MAFW_METADATA_KEY_DURATION);
                        gvalnew = mafw_metadata_first(
                                mc->metadata_value,
                                MAFW_METADATA_KEY_DURATION);
                        if (gvalcur && gvalnew) {
                                g_value_set_int(gvalcur,
                                                g_value_get_int(gvalcur) +
                                                g_value_get_int(gvalnew));
                        }
			mafw_metadata_release(mc->metadata_value);
                }
        } else {
                result = mc->metadata_value;
        }

        if (result) {
                /* Change childcount: it is 2 (music and videos) */
                gvalcur = mafw_metadata_first(result,
                                              MAFW_METADATA_KEY_CHILDCOUNT_1);
                if (gvalcur) {
                        g_value_set_int(gvalcur, 2);
                }
        }
        _get_metadata_tracker_cb(result, error, user_data);
}

static void _get_metadata_tracker_from_root_music_cb(GHashTable *result,
                                                     GError *error,
                                                     gpointer user_data)
{
        struct _metadatas_closure *mc = (struct _metadatas_closure *) user_data;

        /* Store the current value */
        mc->metadata_value = result;

        /* Ask for videos and enqueue results */
        ti_get_metadata_from_videos(mc->common->asked_keys,
                                    ROOT_TITLE,
                                    _get_metadata_tracker_from_root_videos_cb,
                                    mc);
}

static void _get_metadata_cb(MafwSource *source,
                             GHashTable *metadatas,
                             gpointer user_data,
                             const GError *error)
{
        struct _metadata_closure *mc = (struct _metadata_closure *) user_data;

        if (error) {
                mc->cb(source,
                       mc->object_id,
                       NULL,
                       mc->user_data,
                       error);
        } else {
                mc->cb(source,
                       mc->object_id,
                       g_hash_table_lookup(metadatas, mc->object_id),
                       mc->user_data,
                       NULL);
        }

        /* Free closure */
        g_free(mc->object_id);
        g_slice_free(struct _metadata_closure, mc);
}

static gchar ** _get_keys(GHashTable *metadata)
{
	GList *keys;
	gchar **key_array;
	gint i, n;

	if (!metadata)
		return NULL;

	keys = g_hash_table_get_keys(metadata);
	if (!keys)
		return NULL;

	n = g_list_length(keys);
        key_array = g_new0(gchar *, n + 1);

	for (i = 0; i < n; i++) {
		key_array[i] = g_strdup((gchar *) g_list_nth_data(keys, i));
	}

	g_list_free(keys);
	return key_array;
}

static gboolean _update_metadata_idle(gpointer data)
{
        struct _update_metadata_closure *umc = NULL;
        GError *error = NULL;
        gchar **non_updated_keys = NULL;
	gboolean updated;

        umc = (struct _update_metadata_closure *) data;

        non_updated_keys = ti_set_metadata(umc->clip, umc->metadata,
					   umc->category, &updated);

        if (!non_updated_keys) {
                error = NULL;
        } else {
                error = g_error_new(
                        MAFW_SOURCE_ERROR,
                        MAFW_SOURCE_ERROR_UNSUPPORTED_METADATA_KEY,
                        "Some keys could not be set.");
        }

	if (umc->cb != NULL) {
		umc->cb(umc->source,
			umc->object_id,
			(const gchar **) non_updated_keys,
			umc->user_data,
			error);
	}

	if (updated) {
		g_signal_emit_by_name(MAFW_SOURCE(umc->source),
				      "metadata-changed",
				      umc->object_id);
	}

        if (error) {
                g_error_free(error);
        }
        g_hash_table_unref(umc->metadata);
        g_free(umc->object_id);
        g_free(umc->clip);
        g_slice_free(struct _update_metadata_closure, umc);
        g_strfreev(non_updated_keys);

        return FALSE;
}

void
mafw_tracker_source_get_metadata_by_flags(MafwSource *self,
                                 const gchar *object_id,
                                 guint64 metadata_keys,
                                 MafwSourceMetadataResultCb metadata_cb,
                                 gpointer user_data)
{
        struct _metadata_closure *mc;
        gchar *object_ids[2] = { 0, };

        g_return_if_fail(MAFW_IS_TRACKER_SOURCE(self));

        mc = g_slice_new(struct _metadata_closure);
        mc->object_id = g_strdup(object_id);
        mc->cb = metadata_cb;
        mc->user_data = user_data;

        object_ids[0] = mc->object_id;

        mafw_tracker_source_get_metadatas_by_flags(self,
                                          (const gchar **) object_ids,
                                          metadata_keys,
                                          _get_metadata_cb,
                                          mc);
}


void
mafw_tracker_source_get_metadata(MafwSource *self,
                                 const gchar *object_id,
                                 const gchar *const *metadata_keys,
                                 MafwSourceMetadataResultCb metadata_cb,
                                 gpointer user_data)
{
	guint64 mdata_flags;

	g_return_if_fail(MAFW_IS_TRACKER_SOURCE(self));
	
	mdata_flags = keymap_compile_mdata_keys(metadata_keys);

	mafw_tracker_source_get_metadata_by_flags(self, object_id, mdata_flags,
						metadata_cb, user_data);
}

void
mafw_tracker_source_get_metadatas_by_flags(MafwSource *self,
                                  const gchar **object_ids,
                                  guint64 metadata_keys,
                                  MafwSourceMetadataResultsCb metadatas_cb,
                                  gpointer user_data)
{
	GError *error = NULL;
        CategoryType category;
        gchar *album = NULL;
        gchar *artist = NULL;
        gchar *clip = NULL;
        guint64 playlist_metadata_keys = 0;
        gchar *genre = NULL;
        struct _metadatas_common_closure *mcc = NULL;
        struct _metadatas_closure *mc = NULL;
        struct _metadatas_closure *video_mc = NULL;
        struct _metadatas_closure *audio_mc = NULL;
        struct _metadatas_closure *playlist_mc = NULL;
        GPtrArray *video_clips = NULL;
        GPtrArray *audio_clips = NULL;
        GPtrArray *playlist_clips = NULL;
        gint i;

	g_return_if_fail(MAFW_IS_TRACKER_SOURCE(self));

	/* Prepare common structure operation */
        mcc = g_slice_new(struct _metadatas_common_closure);
	mcc->source = self;
        mcc->asked_keys = metadata_keys;
	mcc->callback = metadatas_cb;
	mcc->user_data = user_data;
        mcc->remaining = object_ids? g_strv_length((gchar **) object_ids): 0;
        mcc->metadatas =
                g_hash_table_new_full(g_str_hash,
                                      g_str_equal,
                                      (GDestroyNotify) g_free,
                                      (GDestroyNotify) mafw_metadata_release);
	mcc->error = NULL;

	if (!object_ids || !object_ids[0]) {
		mcc->error = g_error_new(MAFW_SOURCE_ERROR,
                                         MAFW_SOURCE_ERROR_INVALID_OBJECT_ID,
                                         "No object id was specified");
		_emit_metadatas_results(mcc);
		return;
        }

	if (metadata_keys == 0) {
		/* No requested metadata, emit an empty result */
		_emit_metadatas_results(mcc);
		return;
        }

        i = 0;
        while (object_ids[i]) {
                category = util_extract_category_info(object_ids[i],
                                                      &genre, &artist,
                                                      &album, &clip);

                if (category == CATEGORY_ERROR) {
			g_free(genre);
			g_free(artist);
			g_free(album);

                        mc = g_slice_new(struct _metadatas_closure);
                        mc->object_id = g_strdup(object_ids[i]);
                        mc->common = mcc;
			mc->metadata_value = NULL;

                        error = g_error_new(MAFW_SOURCE_ERROR,
                                            MAFW_SOURCE_ERROR_INVALID_OBJECT_ID,
                                            "Invalid object id");

                        _get_metadata_tracker_cb(NULL, error, mc);
                } else if (clip) {
			g_free(genre);
			g_free(artist);
			g_free(album);

                        /* Accumulate clips for later processing */
                        switch (category) {
                        case CATEGORY_VIDEO:
                                if (!video_mc) {
                                        video_mc = g_slice_new(
                                                struct _metadatas_closure);
                                        video_mc->common = mcc;
					video_mc->object_ids = NULL;
					video_mc->metadata_value = NULL;
					video_clips = g_ptr_array_new();
                                }
                                video_mc->object_ids =
                                        g_list_prepend(video_mc->object_ids,
                                                       g_strdup(object_ids[i]));
                                g_ptr_array_add(video_clips, clip);
                                break;
                        case CATEGORY_MUSIC_PLAYLISTS:
                                if (!playlist_mc) {
                                        playlist_mc = g_slice_new(
                                                struct _metadatas_closure);
                                        playlist_mc->common = mcc;
					playlist_mc->metadata_value = NULL;
					playlist_mc->object_ids = NULL;
					playlist_clips = g_ptr_array_new();

                                        /* If duration is required, then we need
                                         * to add a new key in order to check if
                                         * duration is right or need to be
                                         * calculated */
                                        playlist_metadata_keys = mcc->asked_keys;
                                        if (util_is_duration_requested(mcc->asked_keys)) {
                                                playlist_metadata_keys |= MTrackerSrc_TRACKER_PKEY_VALID_DURATION;
                                        }
                                }
                                playlist_mc->object_ids =
                                        g_list_prepend(playlist_mc->object_ids,
                                                       g_strdup(object_ids[i]));
                                g_ptr_array_add(playlist_clips, clip);

                                break;
                        default:
                                if (!audio_mc) {
                                        audio_mc = g_slice_new(
                                                struct _metadatas_closure);
                                        audio_mc->common = mcc;
					audio_mc->metadata_value = NULL;
					audio_mc->object_ids = NULL;
					audio_clips = g_ptr_array_new();
                                }
                                audio_mc->object_ids =
                                        g_list_prepend(audio_mc->object_ids,
                                                       g_strdup(object_ids[i]));
                                g_ptr_array_add(audio_clips, clip);
                                break;
                        }
                } else {
                        mc = g_slice_new(struct _metadatas_closure);
                        mc->object_id = g_strdup(object_ids[i]);
                        mc->common = mcc;
			mc->metadata_value = NULL;

                        switch (category) {
                        case CATEGORY_ROOT:
                                ti_get_metadata_from_music(
                                        mcc->asked_keys,
                                        ROOT_TITLE,
                                        _get_metadata_tracker_from_root_music_cb,
                                        mc);
				g_free(genre);
				g_free(artist);
				g_free(album);
                                break;

                        case CATEGORY_VIDEO:
                                ti_get_metadata_from_videos(mcc->asked_keys,
                                                            ROOT_VIDEOS_TITLE,
                                                            _get_metadata_tracker_cb,
                                                            mc);
				g_free(genre);
				g_free(artist);
				g_free(album);
                                break;

                        case CATEGORY_MUSIC:
                                ti_get_metadata_from_music(
                                        mcc->asked_keys,
                                        ROOT_MUSIC_TITLE,
                                        _get_metadata_tracker_from_music_cb,
                                        mc);
				g_free(genre);
				g_free(artist);
				g_free(album);
                                break;

                        case CATEGORY_MUSIC_ARTISTS:
                                ti_get_metadata_from_category(
                                        genre, artist, album,
                                        MTrackerSrc_ID_ARTIST,
                                        ROOT_MUSIC_ARTISTS_TITLE,
                                        mcc->asked_keys,
                                        _get_metadata_tracker_cb,
                                        mc);
                                break;
                        case CATEGORY_MUSIC_ALBUMS:
                                ti_get_metadata_from_category(
                                        genre, artist, album,
                                        MTrackerSrc_ID_ALBUM,
                                        ROOT_MUSIC_ALBUMS_TITLE,
                                        mcc->asked_keys,
                                        _get_metadata_tracker_cb,
                                        mc);
                                break;

                        case CATEGORY_MUSIC_GENRES:
                                ti_get_metadata_from_category(
                                        genre, artist, album,
                                        MTrackerSrc_ID_GENRE,
                                        ROOT_MUSIC_GENRES_TITLE,
                                        mcc->asked_keys,
                                        _get_metadata_tracker_cb,
                                        mc);
                                break;

                        case CATEGORY_MUSIC_SONGS:
                                ti_get_metadata_from_music(
                                        mcc->asked_keys,
                                        ROOT_MUSIC_SONGS_TITLE,
                                        _get_metadata_tracker_cb,
                                        mc);
				g_free(genre);
				g_free(artist);
				g_free(album);
                                break;

                        case CATEGORY_MUSIC_PLAYLISTS:
                                ti_get_metadata_from_playlists(
                                        mcc->asked_keys,
                                        ROOT_MUSIC_PLAYLISTS_TITLE,
                                        _get_metadata_tracker_from_playlist_cb,
                                        mc);
				g_free(genre);
				g_free(artist);
				g_free(album);
                                break;

                        default:
                                /* CATEGORY_ERROR was already considered
                                 * above */
				g_free(genre);
				g_free(artist);
				g_free(album);
                                break;
                        }
                }
		

                i++;
        }

        if (audio_mc) {
		g_ptr_array_add(audio_clips, NULL);
                ti_get_metadata_from_audioclip((gchar**)audio_clips->pdata, mcc->asked_keys,
                                               _get_metadatas_tracker_cb,
                                               audio_mc);
                g_strfreev((gchar**)audio_clips->pdata);
                g_ptr_array_free(audio_clips, FALSE);
        }

        if (video_mc) {
		g_ptr_array_add(video_clips, NULL);
                ti_get_metadata_from_videoclip((gchar**)video_clips->pdata, mcc->asked_keys,
                                               _get_metadatas_tracker_cb,
                                               video_mc);
                g_strfreev((gchar**)video_clips->pdata);
                g_ptr_array_free(video_clips, FALSE);
        }

        if (playlist_mc) {
		g_ptr_array_add(playlist_clips, NULL);
                ti_get_metadata_from_playlist(
                        (gchar**)playlist_clips->pdata,
                        playlist_metadata_keys,
                        _get_metadatas_tracker_from_playlist_cb,
                        playlist_mc);
                g_strfreev((gchar**)playlist_clips->pdata);
                g_ptr_array_free(playlist_clips, FALSE);
        }
}

void
mafw_tracker_source_get_metadatas(MafwSource *self,
                                  const gchar **object_ids,
				  const gchar *const *metadata_keys,
                                  MafwSourceMetadataResultsCb metadatas_cb,
                                  gpointer user_data)
{
	guint64 mdata_flags;

	g_return_if_fail(MAFW_IS_TRACKER_SOURCE(self));
	
	mdata_flags = keymap_compile_mdata_keys(metadata_keys);

	mafw_tracker_source_get_metadatas_by_flags(self, object_ids, mdata_flags,
					metadatas_cb, user_data);
}

void
mafw_tracker_source_set_metadata(MafwSource *self,
                                 const gchar *object_id,
                                 GHashTable *metadata,
                                 MafwSourceMetadataSetCb cb,
                                 gpointer user_data)
{
	GError *error = NULL;
        gchar *clip = NULL;
	gchar **failed_keys;
	CategoryType category;

        struct _update_metadata_closure *_update_metadata_data = NULL;

        g_return_if_fail(MAFW_IS_TRACKER_SOURCE(self));

        if (!object_id) {
		g_set_error(&error,
			MAFW_SOURCE_ERROR,
			MAFW_SOURCE_ERROR_INVALID_OBJECT_ID,
			"No browse id was specified");
		failed_keys = _get_keys(metadata);

		cb(self, object_id, (const gchar **) failed_keys,
		   user_data, error);

		g_strfreev(failed_keys);
		g_error_free(error);
		return;
        }

        if (!metadata) {
		g_set_error(&error,
			MAFW_SOURCE_ERROR,
			MAFW_SOURCE_ERROR_UNSUPPORTED_METADATA_KEY,
			"No metadata was specified");
		failed_keys = _get_keys(metadata);

		cb(self, object_id, (const gchar **) failed_keys,
		   user_data, error);

		g_strfreev(failed_keys);
		g_error_free(error);
                return;
        }

        /* Only audio and video clips can be changed */
	category = util_extract_category_info(object_id, NULL,
                                              NULL, NULL, &clip);
        if ((clip) && (category != CATEGORY_MUSIC_PLAYLISTS)) {
                _update_metadata_data =
                        g_slice_new(struct _update_metadata_closure);
                _update_metadata_data->source = self;
                _update_metadata_data->object_id = g_strdup(object_id);
                _update_metadata_data->metadata = metadata;
                _update_metadata_data->cb = cb;
                _update_metadata_data->user_data = user_data;
                _update_metadata_data->clip = clip;
		_update_metadata_data->category = category;

                /* Block hashtable while not finishing */
                g_hash_table_ref(metadata);

                g_idle_add(_update_metadata_idle, _update_metadata_data);

                return;
        } else {
		g_set_error(&error,
			MAFW_SOURCE_ERROR,
			MAFW_SOURCE_ERROR_INVALID_OBJECT_ID,
			"Only objectids referencing clips are allowed");
		failed_keys = _get_keys(metadata);

              	cb(self, object_id, (const gchar **) failed_keys,
		   user_data, error);

		g_strfreev(failed_keys);
		g_error_free(error);
                return;
        }
}
