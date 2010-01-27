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

/* Include files */
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>

#include "mafw-uri-source.h"
#include "mafw-metadata.h"
#include "mafw-callbas.h"
#include "mafw-marshal.h"
#include "mafw-errors.h"

/**
 * SECTION:mafwuri
 * @short_description: built-in MafwSource capable of telling the
 * metadata of random URLs
 *
 * This is a simple built-in MafwSource capable of telling the
 * metadata of random URLs.  This source is not browseable, to obtain
 * an object ID you need to mafw_source_create_objectid().  You can
 * for example add the returned object ID to a playlist and inquire
 * about its metadata the usual way (determining the source by
 * splitting it and finding the source with
 * mafw_registry_find_by_uuid()) later.  Currently only the
 * #MAFW_METADATA_KEY_URI is supported.
 */

/* GObject prerequisites */
typedef MafwSourceClass MafwURISourceClass;

static GType uri_source_get_type(void);
static void uri_source_class_init(MafwURISourceClass *me);
static void uri_source_init(MafwURISource *self);

G_DEFINE_TYPE(MafwURISource, uri_source, MAFW_TYPE_SOURCE);

/* Program code */
static void get_metadata(MafwSource *self,
			     const gchar *object_id,
			     const gchar *const *mdkeys,
			     MafwSourceMetadataResultCb cb,
			     gpointer cbarg)
{
	guint i;
	GHashTable *metadata = NULL;
	const gchar *uri;
	gchar *title, *temp;

	/* Check the sanity of the arguments. */
	g_return_if_fail(object_id != NULL);
	g_return_if_fail(g_str_has_prefix(object_id,
					      MAFW_URI_SOURCE_UUID "::"));
	g_return_if_fail(mdkeys != NULL);
	g_return_if_fail(cb     != NULL);

	/* Do we need to return the URI? */
	metadata = NULL;
	for (i = 0; mdkeys[i]; i++) {
		if (!strcmp(mdkeys[i], "*")) {
			/* Create the MAFW metadata hash table
			 * to return the URL and the TITLE. */
			uri = object_id + strlen(MAFW_URI_SOURCE_UUID"::");
			if (!metadata)
				metadata = mafw_metadata_new();
			mafw_metadata_add_str(metadata,
					       MAFW_METADATA_KEY_URI, uri);
			title = g_path_get_basename(uri);
			temp = g_uri_unescape_string(title, NULL);
			g_free(title);
			mafw_metadata_add_str(metadata,
					       MAFW_METADATA_KEY_TITLE,
					       temp);
			g_free(temp);
			break;
		} else if (!strcmp(mdkeys[i], MAFW_METADATA_KEY_URI)) {
			/* Create the MAFW metadata hash table
			 * to return the URL. */
			uri = object_id + strlen(MAFW_URI_SOURCE_UUID"::");
			if (!metadata)
				metadata = mafw_metadata_new();
			mafw_metadata_add_str(metadata,
					       MAFW_METADATA_KEY_URI, uri);
		} else if (!strcmp(mdkeys[i], MAFW_METADATA_KEY_TITLE)) {
			uri = object_id + strlen(MAFW_URI_SOURCE_UUID"::");	
			if (!metadata)
				metadata = mafw_metadata_new();
			title = g_path_get_basename(uri);
			temp = g_uri_unescape_string(title, NULL);
			g_free(title);
			mafw_metadata_add_str(metadata,
					       MAFW_METADATA_KEY_TITLE,
					       g_basename(temp));
			g_free(temp);
		}
	}

	/* Give our results to $cb later as required by
	 * the MafwSource interface. */
	mafw_callbas_defer(mafw_callbas_new(G_CALLBACK(cb),
		mafw_marshal_VOID__STRING_BOXED_POINTER_POINTER,
		self, MAFW_CBAS_STRING(object_id), MAFW_CBAS_HASH(metadata),
		MAFW_CBAS_POINTER(cbarg), MAFW_CBAS_NULL, MAFW_CBAS_END));

}

static void destroy_object(MafwSource *self,
			   const gchar *object_id,
			   MafwSourceObjectDestroyedCb cb,
			   gpointer user_data)
{
	GError *error = NULL;
	gchar *path;
	gchar *uri;

        g_return_if_fail(MAFW_IS_SOURCE(self));
        g_return_if_fail(object_id != NULL);
	
	/* Get URI from objectid, skip source uuid */
	uri = (gchar *) object_id + strlen(MAFW_URI_SOURCE_UUID"::");
	
	/* Only local URIs are supported */
	if (!g_str_has_prefix(uri, "file://")) {
		error = g_error_new(MAFW_SOURCE_ERROR,
				    MAFW_SOURCE_ERROR_DESTROY_OBJECT_FAILED,
				    "Only local resources can be destroyed");
		
	} else {
		/* Skip "file://" prefix */
		path = uri + 7;
	}

	if (!error) {
		if (g_unlink(path) == -1) {
			error = g_error_new(
				MAFW_SOURCE_ERROR,
				MAFW_SOURCE_ERROR_DESTROY_OBJECT_FAILED,
				"Failed to unlink path %s", path);
		}
	}

	if (cb) {
		cb(self, object_id, user_data, error);
	}

	g_error_free(error);
}

/* GObject infrastructure */
/**
 * mafw_get_uri_source:
 *
 * Gets the framework-wide instance of the uri-source.
 * 
 * Returns: the #URISource. Use this function to get hold on it.
 * Don't unref the returned source.
 */
MafwURISource *mafw_get_uri_source(void)
{
	static MafwURISource *self = NULL;

	if (!self)
		self = g_object_new(uri_source_get_type(), NULL);
	return self;
}

void uri_source_class_init(MafwURISourceClass *me)
{
	me->get_metadata = get_metadata;
	me->destroy_object = destroy_object;
}

void uri_source_init(MafwURISource *self)
{
	/* NOP */
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
