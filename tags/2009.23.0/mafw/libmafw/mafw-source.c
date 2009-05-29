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
#include <regex.h>

#include "mafw-extension.h"
#include "mafw-source.h"
#include "mafw-marshal.h"
#include "mafw-uri-source.h"
#include "mafw-errors.h"
#include "mafw-metadata.h"

/**
 * SECTION:mafwsource
 * @short_description: base class for sources
 *
 * #MafwSource is an abstract base class which individual sources should
 * inherit from. Sources represent repositories of multimedia contents, which
 * can either reside in the local file system, on a remote server or being
 * generated on-the-fly for example by aggregating other sources. Sources are
 * comparable to GnomeVFS handlers but they offer extended functionality.
 *
 * Object IDs:
 *
 * Individual content items are identified by their `Object ID'. Sources
 * produce and renderers consume Object IDs, which are only meaningful for the
 * providing sources themselves. An object ID is a string:
 *
 *   `&lt;Source ID&gt;::&lt;Item ID&gt;'
 *
 * consisting of the Source ID of the originating source and a unique (within
 * the individual source's scope) Item ID, separated by two colons. See the
 * #MafwRegistry description for `Source ID's. Each object ID should be unique,
 * persistent and portable, if possible. Applications may assume that an object
 * ID corresponds to a single content item, and this should be maintained
 * across shutdowns, reboots and reinstalls.
 *
 * #MafwSource's can be acquired from a #MafwRegistry. The two main
 * operations of a source are browsing for content items with
 * mafw_source_browse() and acquiring metadata information related to
 * individual items with mafw_source_get_metadata(). These functions
 * are the minimum that each source should provide.
 *
 * In addition to getting content information, one can also modify the contents
 * with optional methods that include mafw_source_create_object() for object
 * creation, mafw_source_destroy_object() for object destruction and
 * mafw_source_set_metadata() for modifying the metadata of individual items.
 */

/* Value usable as @metadata_keys of browse(), denoting that the
 * caller is not interested in any metadata. */
const gchar * const _mafw_source_no_keys[] = { NULL };
/* Value usable as @metadata_keys of browse(), denoting that the
 * caller is interested in all possible metadata. */
const gchar * const _mafw_source_all_keys[] = { MAFW_SOURCE_KEY_WILDCARD, NULL };

enum {
	METADATA_CHANGED = 0,
	CONTAINER_CHANGED,
	LAST_SIGNAL,
};

static guint mafw_source_signals[LAST_SIGNAL];

static guint mafw_source_default_browse(MafwSource *self,
				const gchar *object_id, gboolean recursive,
				const MafwFilter *filter,
				const gchar *sort_criteria,
				const gchar *const *mdkeys,
				guint skip_count, guint item_count,
				MafwSourceBrowseResultCb cb, gpointer user_data)
{
	if (cb != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
		    	MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
		    	"Not implemented");
		cb(self, MAFW_SOURCE_INVALID_BROWSE_ID, 0, 0, NULL, NULL,
					user_data, error);
		g_error_free(error);
	}
	return MAFW_SOURCE_INVALID_BROWSE_ID;
}

static gboolean mafw_source_default_cancel_browse(MafwSource *self, guint browse_id,
				  GError **error)
{
	if (error)
	{
		g_set_error(error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
	}
	return FALSE;
}

static void mafw_source_default_get_metadata(MafwSource *self,
						const gchar *object_id,
						const gchar *const *mdkeys,
						MafwSourceMetadataResultCb cb,
						gpointer user_data)
{
	if (cb != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		cb(self, object_id, NULL, user_data, error);
		g_error_free(error);
	}
}

static void get_keys_cb(gpointer key, gpointer val, GPtrArray *keylist)
{
	g_ptr_array_add(keylist, key);
}

static void mafw_source_default_set_metadata(MafwSource *self,
				 const gchar *object_id,
				 GHashTable *metadata,
				 MafwSourceMetadataSetCb cb,
				 gpointer user_data)
{
	if (cb != NULL)
	{
		GError *error = NULL;
		GPtrArray *keylist = g_ptr_array_new();
		
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		
		g_hash_table_foreach(metadata, (GHFunc)get_keys_cb, keylist);
		g_ptr_array_add(keylist, NULL);
		
		cb(self, object_id, (const gchar **)keylist->pdata, user_data,
				error);
		g_ptr_array_free(keylist, TRUE);
		g_error_free(error);
	}
}

static void mafw_source_default_create_object(MafwSource *self,
				  const gchar *parent, GHashTable *metadata,
				  MafwSourceObjectCreatedCb cb,
				  gpointer user_data)
{
	if (cb != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		cb(self, NULL, user_data, error);
		g_error_free(error);
	}
}

static void mafw_source_default_destroy_object(MafwSource *self,
				   const gchar *object_id,
				   MafwSourceObjectDestroyedCb cb,
				   gpointer user_data)
{
	if (cb != NULL)
	{
		GError *error = NULL;
		g_set_error(&error,
			    MAFW_EXTENSION_ERROR,
			    MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
			    "Not implemented");
		cb(self, NULL, user_data, error);
		g_error_free(error);
	}
}

#define get_sorting_modifier(key) ((*(key) == '+') ? 1 : ((*(key) =='-') ? -1 : 0))

static gboolean
check_sort_criteria (const gchar *criteria, GError **error)
{
	gboolean correct_criteria = TRUE;

        if (criteria && criteria[0] != '\0') {
		guint i = 0;
		gchar **sort_tokens = NULL;
		
		sort_tokens = g_strsplit(criteria, ",", 0);
		
		for (i = 0; (sort_tokens[i] != NULL) && correct_criteria;
		     i++) {
			if (get_sorting_modifier(sort_tokens[i]) == 0) {
				correct_criteria = FALSE;
			}
		}
		
		if (!correct_criteria) {
			if (error != NULL) {
                                *error = g_error_new(MAFW_SOURCE_ERROR,
						     MAFW_SOURCE_ERROR_INVALID_SORT_STRING,
						     "Wrong sorting criteria '%s'",
						     criteria);
			}
		}

		g_strfreev(sort_tokens);
	}

	return correct_criteria;
}

/*
 * This is part of the implementation of the Template Method pattern
 * applied in mafw_source_browse method.
 */
static guint
mafw_source_do_browse(MafwSource *self,
		       const gchar *object_id,
		       gboolean recursive, const MafwFilter *filter,
		       const gchar *sort_criteria,
		       const gchar *const *metadata_keys,
		       guint skip_count, guint item_count,
		       MafwSourceBrowseResultCb browse_cb,
		       gpointer user_data)
{
	return MAFW_SOURCE_GET_CLASS(self)->browse(self, object_id, recursive,
						    filter, sort_criteria,
						    metadata_keys,
						    skip_count, item_count,
						    browse_cb, user_data);
}


G_DEFINE_ABSTRACT_TYPE(MafwSource, mafw_source, MAFW_TYPE_EXTENSION);

static void mafw_source_init(MafwSource * self)
{ /* NOP */ }

static void mafw_source_class_init(MafwSourceClass * klass)
{
	klass->browse = mafw_source_default_browse;
	klass->cancel_browse = mafw_source_default_cancel_browse;
	klass->get_metadata = mafw_source_default_get_metadata;
	klass->set_metadata = mafw_source_default_set_metadata;
	klass->create_object = mafw_source_default_create_object;
	klass->destroy_object = mafw_source_default_destroy_object;
	
	/**
	 * MafwSource::metadata-changed:
	 * @self:      The emitting #MafwSource instance.
	 * @object_id: The object ID, whose metadata has changed
	 *
	 * Emitted when the metadata related to an object has changed.
	 */
	mafw_source_signals[METADATA_CHANGED] =
	    g_signal_new("metadata-changed",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 0,
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__STRING,
			 G_TYPE_NONE,
			 1,
			 G_TYPE_STRING);
	
	/**
	 * MafwSource::container-changed:
	 * @self:      The emitting #MafwSource instance.
	 * @object_id: The object ID of the changed container.
	 *
	 * Emitted when a container's contents have changed.
	 */
	mafw_source_signals[CONTAINER_CHANGED] =
	    g_signal_new("container-changed",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 0,
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__STRING,
			 G_TYPE_NONE,
			 1,
			 G_TYPE_STRING);
}

/**
 * mafw_source_browse:
 * @self:          A #MafwSource instance to browse.
 * @object_id:     The starting object id to browse from (usually a container).
 * @recursive:     %TRUE, if browsing should be recursive.
 * @filter:        Filter criteria, see #MafwFilter.
 * @sort_criteria: Sort criteria (i.e. the order in which results are expected)
 * @metadata_keys: A %NULL-terminated array of requested metadata keys.
 * @skip_count:    Number of items to skip from the beginning.
 * @item_count:    Number of items to return.
 * @browse_cb:     Function to call with browse results, or inform about the error
 * @user_data:     Optional user data pointer passed along with @browse_cb.
 *
 * Starts a browse session on the given source. The caller will be notified of
 * the results via the @browse_cb callback function.
 *
 * @sort_criteria is a comma-separated list of metadata keys, each prefixed
 * with '+' or '-', indicating ascending or descending ordering. For example:
 * "+year,+artist" sorts first by year, then artists.
 *
 * @skip_count and @item_count are similar to the `LIMIT' SQL clause:
 * specifying them means to only return at most @item_count items, skipping
 * @skip_count from the beginning of the result set. If the caller is
 * interested in all resulting elements, @item_count should be
 * #MAFW_SOURCE_BROWSE_ALL.
 *
 * @metadata_keys being equal to #MAFW_SOURCE_NO_KEYS indicates that the caller
 * is not interested in any metadata, only in the resulting object ids. On the
 * other hand, if @metadata_keys is equal to #MAFW_SOURCE_ALL_KEYS then the
 * source should try to retrieve all possible metadata with the results.
 *
 * Returns: The identifier of the browse session (which is also passed
 *          to @browse_cb). If some arguments were invalid,
 *          #MAFW_SOURCE_INVALID_BROWSE_ID is returned, but the @browse_cb will
 *	    be called.
 */
guint mafw_source_browse(MafwSource *self,
			 const gchar *object_id,
			 gboolean recursive, const MafwFilter *filter,
			 const gchar *sort_criteria,
			 const gchar *const *metadata_keys,
			 guint skip_count, guint item_count,
			 MafwSourceBrowseResultCb browse_cb,
			 gpointer user_data)
{
	GError *error = NULL;
	/* This implements the Template Method Pattern
	 * (http://en.wikipedia.org/wiki/Template_method_pattern). We'll
	 * do some stuff and then we let subclasses implement the real
	 * browsing, so subclasses only will have to reimplement the
	 * virtual method browse, which in this superclass is invoked
	 * with do_browse. So we perform the tests that cannot be
	 * overriden and then we let the subclasses do the other
	 * stuff. */

	if (!check_sort_criteria (sort_criteria, &error)) {
		browse_cb(self, MAFW_SOURCE_INVALID_BROWSE_ID, 0, 0, NULL, NULL,
					user_data, error);
		g_error_free(error);
		return MAFW_SOURCE_INVALID_BROWSE_ID;
	}

	return mafw_source_do_browse(self, object_id, recursive,
				      filter, sort_criteria,
				      metadata_keys,
				      skip_count, item_count,
				      browse_cb, user_data);
}

/**
 * mafw_source_all_keys:
 * @keys: A %NULL-terminated array of strings.
 *
 * Checks whether a source processing the requested @keys should return all
 * possible keys.
 *
 * Returns: %TRUE if the first element of the array is a single asterisk.
 */
gboolean mafw_source_all_keys(const gchar *const *keys)
{
	return keys && keys[0] && keys[0][0] == '*' && keys[0][1] == 0;
}

/**
 * mafw_source_cancel_browse:
 * @self:      A #MafwSource instance.
 * @browse_id: The browse session to cancel.
 * @error:     Location for a #GError, or %NULL
 *
 * Informs a #MafwSource that browse results associated with
 * @browse_id are no longer of any interest. Sources stop the
 * operation right away, but it is not guaranteed that the callback of
 * mafw_source_browse() would not be invoked anymore. It is not an
 * error if the request has already come to end.
 *
 * Returns: %FALSE if the operation failed (in which case @error is also set).
 */
gboolean mafw_source_cancel_browse(MafwSource *self, guint browse_id,
				   GError **error)
{
	return MAFW_SOURCE_GET_CLASS(self)->cancel_browse(self,
							  browse_id,
							  error);
}

/**
 * mafw_source_get_metadata:
 * @self:          A #MafwSource instance.
 * @object_id:     The object ID, whose metadata is being requested.
 * @metadata_keys: A %NULL-terminated array of requested metadata keys.
 * @metadata_cb:   The function to call with results.
 * @user_data:     Optional user data pointer passed along with @metadata_cb.
 *
 * Queries the metadata for the given @object_id. The caller is informed of
 * results via the @metadata_cb callback.
 *
 * If @metadata_keys is #MAFW_SOURCE_ALL_KEYS then the source should
 * try to retrieve all possible (see <link
 * linkend="mafw-MafwMetadata">MafwMetadata</link>) metadata it can
 * related to the given object.
 */
void mafw_source_get_metadata(MafwSource *self,
				  const gchar *object_id,
				  const gchar *const *metadata_keys,
				  MafwSourceMetadataResultCb metadata_cb,
				  gpointer user_data)
{
	MAFW_SOURCE_GET_CLASS(self)->get_metadata(self,
							 object_id,
							 metadata_keys,
							 metadata_cb,
							 user_data);
}

struct metadatas_data
{
	MafwSource *self;
	GHashTable *metadatas;	/*< collected metadatas */
	guint remaining_count;	/*< number or missing metadata-requests */
	guint result_id;	/*< source-id of the _emit_result idle cb */
	MafwSourceMetadataResultsCb cb;
	GError *err;
	gpointer udata;
};

/* Calls the get_metadatas_cb */
static gboolean _emit_result(struct metadatas_data *mdatas_data)
{
	if (mdatas_data->remaining_count)
	{
		mdatas_data->result_id = 0;
		return FALSE;
	}
	mdatas_data->cb(mdatas_data->self, mdatas_data->metadatas, mdatas_data->udata,
				mdatas_data->err);
	g_hash_table_unref(mdatas_data->metadatas);
	if (mdatas_data->err)
		g_error_free(mdatas_data->err);
	g_object_unref(mdatas_data->self);
	g_free(mdatas_data);
	return FALSE;
}

/* Collect the requested metadatas */
static void _metadata_collector(MafwSource *self,
					   const gchar *object_id,
					   GHashTable *metadata,
					   struct metadatas_data *mdatas_data,
					   const GError *error)
{
	mdatas_data->remaining_count--;
	if (metadata)
		g_hash_table_insert(mdatas_data->metadatas, g_strdup(object_id),
				g_hash_table_ref(metadata));
	
	if (error && !mdatas_data->err)
	{
		mdatas_data->err = g_error_copy(error);
	}
	
	if (!mdatas_data->remaining_count && !mdatas_data->result_id)
	{/* Call the cb on idle, so it should work with sync get_metadata too*/
		mdatas_data->result_id = g_idle_add((GSourceFunc)_emit_result,
					(gpointer)mdatas_data);
	}
}

/**
 * mafw_source_get_metadatas:
 * @self:          A #MafwSource instance.
 * @object_ids:     %NULL terminated list of object IDs, whose metadata is being requested.
 * @metadata_keys: A %NULL-terminated array of requested metadata keys.
 * @metadatas_cb:   The function to call with results.
 * @user_data:     Optional user data pointer passed along with @metadata_cb.
 *
 * Queries the metadatas for the given @object_id. The caller is informed of
 * results via the @metadatas_cb callback.
 *
 * If @metadata_keys is #MAFW_SOURCE_ALL_KEYS then the source should
 * try to retrieve all possible (see <link
 * linkend="mafw-MafwMetadata">MafwMetadata</link>) metadata it can
 * related to the given object.
 */
void mafw_source_get_metadatas(MafwSource *self,
				  const gchar **object_ids,
				  const gchar *const *metadata_keys,
				  MafwSourceMetadataResultsCb metadatas_cb,
				  gpointer user_data)
{
	if (MAFW_SOURCE_GET_CLASS(self)->get_metadatas)
	{
		MAFW_SOURCE_GET_CLASS(self)->get_metadatas(self,
							 object_ids,
							 metadata_keys,
							 metadatas_cb,
							 user_data);
	}
	else
	{
		gint i;
		struct metadatas_data *mdatas_data = g_new0(struct metadatas_data, 1);
	
		g_assert(object_ids && object_ids[0]);
	
		mdatas_data->cb = metadatas_cb;
		mdatas_data->udata = user_data;
		mdatas_data->self = g_object_ref(self);
		mdatas_data->metadatas = g_hash_table_new_full(g_str_hash,
						g_str_equal,
						(GDestroyNotify)g_free,
						(GDestroyNotify)mafw_metadata_release);

		for (i = 0; object_ids[i]; i++)
		{
			mdatas_data->remaining_count++;
			mafw_source_get_metadata(self, object_ids[i],
							metadata_keys,
							(MafwSourceMetadataResultCb)_metadata_collector,
							mdatas_data);
		}
	}
}

/**
 * mafw_source_set_metadata:
 * @self:      A #MafwSource instance.
 * @object_id: The object ID, whose metadata is being edited.
 * @metadata: A hash table containing the metadata to set for
 * @object_id as #GValues. The tag names and the concrete type of
 * their values go as defined in <link
 * linkend="mafw-MafwMetadata">MafwMetadata</link>
 * @cb:        The function to call when the operation is finished.
 * @user_data: Optional user data pointer passed along with @cb.
 *
 * Updates the given metadata properties and their values to the given
 * @object_id. The caller is notified of operation success via @cb. The source
 * may set only a subset of all the defined keys, in which case, it can report
 * any failed keys with the callback along with an error describing the details.
 *
 */
void mafw_source_set_metadata(MafwSource *self,
				  const gchar *object_id,
				  GHashTable *metadata,
				  MafwSourceMetadataSetCb cb,
				  gpointer user_data)
{
	MAFW_SOURCE_GET_CLASS(self)->set_metadata(self,
							 object_id,
							 metadata,
							 cb,
							 user_data);
}

/**
 * mafw_source_create_object:
 * @self:      A #MafwSource instance.
 * @parent:    Create the object inside this container (object ID).
 * @metadata:  The metadata keys and values that the new object should have
 *             initially; Can be %NULL.
 * @cb:        The function to call when the operation is finished.
 * @user_data: Optional user data pointer passed along with @cb.
 *
 * The intended semantics of this function is to create a new object in
 * @parent initially having at least @metadata.  What it actually does
 * is quite up to the source, which may interpret @parent and @metadata
 * freely. Returns %FALSE on failure and leaves the object unchanged.
 * Otherwise returns %TRUE and @cb will be activated with @user_data later.
 */
void mafw_source_create_object(MafwSource *self,
                                   const gchar *parent,
                                   GHashTable *metadata,
                                   MafwSourceObjectCreatedCb cb,
                                   gpointer user_data)
{
        MAFW_SOURCE_GET_CLASS(self)->create_object(self,
                                                          parent,
                                                          metadata,
                                                          cb,
                                                          user_data);
}

/**
 * mafw_source_destroy_object:
 * @self:      A #MafwSource instance.
 * @object_id:  The object ID of the object to destroy.
 * @cb:        The function to call when the operation is finished.
 * @user_data: Optional user data pointer passed along with @cb.
 *
 * The intended semantics of this function is to delete an object from the
 * given source so that it won't be encountered in browsing the parent
 * container of @objectid and any further operation on it will result in a
 * #MAFW_SOURCE_ERROR_OBJECT_ID_NOT_AVAILABLE error. However, the actual
 * outcome is almost entirely up to the source except for the pieces mentioned
 * here. On failure this function returns %FALSE and leaves the object
 * unchanged. Otherwise returns %TRUE and @cb will be activated with @user_data
 * later.
 */
void mafw_source_destroy_object(MafwSource *self,
                                    const gchar *object_id,
                                    MafwSourceObjectDestroyedCb cb,
                                    gpointer user_data)
{
        MAFW_SOURCE_GET_CLASS(self)->destroy_object(self,
                                                           object_id,
                                                           cb,
                                                           user_data);
}

/**
 * mafw_source_split_objectid:
 * @objectid: An object id to split.
 * @extensionid:   If not %NULL, where to place the source UUID part of @objetid
 *            in a newly allocated string.
 * @itemid:   Similarly, but for the item ID part.
 *
 * Splits an object ID to source UUID and item ID.  Use this function to
 * determine to which sources does an objectid belong to.  Source developers
 * may also use this function to figure out which item does it refer to.
 *
 * Returns: %FALSE if @objectid is invalid.
 */
gboolean mafw_source_split_objectid(gchar const *objectid,
				    gchar **extensionid, gchar **itemid)
{
	const gchar *sep;

	g_return_val_if_fail(objectid != NULL, FALSE);

	if (!(sep = strstr(objectid, "::")))
		return FALSE;
	if (extensionid)
		*extensionid = g_strndup(objectid, sep - objectid);
	if (itemid)
		*itemid = g_strdup( sep+2);

	return TRUE;
}

/**
 * mafw_source_create_objectid:
 * @uri: A URI to create the object ID for or a UNIX file system path.
 *
 * Creates an object ID pointing to @uri that can be used as if it
 * were obtained from a source.  If @uri is not an URI, it's treated
 * as a local file system path.  In that case if it's relative it's
 * assumed to be relative of the program's current working directory.
 * Finally, if @uri is empty the returned objectid will point to the
 * current working directory.
 *
 * Returns: A newly-created object ID (must be freed).
 */
gchar *mafw_source_create_objectid(const gchar *uri)
{
	static regex_t re;
	static gboolean re_compiled = FALSE;

	if (!re_compiled) {
		/* Can't do it in mafw_source_class_init() because we may not
		 * have any source at all.  This RE is slightly altered compared
		 * the one found in RFC3986, modified for validation, rather
		 * than breakdown. */
		if (regcomp(&re,
		    "^(([^:/?#]+):)(//([^/?#]*))([^?#]*)(\\?([^#]*))?(#(.*))?$",
		    REG_EXTENDED | REG_NOSUB) == 0)
			re_compiled = TRUE;
		else
		{
			g_critical("Unable to compile regexp");
			return NULL;
		}
	}

	if (regexec(&re, uri, 0, NULL, 0) == 0) {
		/* $uri is an URI. */
		return g_strconcat(MAFW_URI_SOURCE_UUID"::", uri, NULL);
	} else if (uri[0] == '/') {
		/* $uri is an absolute path */
		return g_strconcat(MAFW_URI_SOURCE_UUID"::"
				  "file://", uri, NULL);
	} else if (uri[0] != '\0') {
		gchar *cwd, *oid;

		/* $uri is a relative path */
		cwd = g_get_current_dir();
		oid = g_strconcat(MAFW_URI_SOURCE_UUID"::"
				  "file://", cwd, "/",
				  uri, NULL);
		g_free(cwd);
		return oid;
	} else {
		gchar *cwd, *oid;

		/* $uri is empty */
		cwd = g_get_current_dir();
		oid = g_strconcat(MAFW_URI_SOURCE_UUID"::"
				  "file://", cwd, NULL);
		g_free(cwd);
		return oid;
	} /* if */
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
