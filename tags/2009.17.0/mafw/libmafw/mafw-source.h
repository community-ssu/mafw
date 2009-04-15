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

#ifndef __MAFWSOURCE_H__
#define __MAFWSOURCE_H__

#include <glib.h>
#include <glib-object.h>

#include <libmafw/mafw-extension.h>
#include <libmafw/mafw-filter.h>

G_BEGIN_DECLS

#define MAFW_TYPE_SOURCE			\
        (mafw_source_get_type ())
#define MAFW_SOURCE(obj)						\
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), MAFW_TYPE_SOURCE, MafwSource))
#define MAFW_IS_SOURCE(obj)					\
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAFW_TYPE_SOURCE))
#define MAFW_SOURCE_CLASS(klass)					\
        (G_TYPE_CHECK_CLASS_CAST((klass), MAFW_TYPE_SOURCE, MafwSourceClass))
#define MAFW_IS_SOURCE_CLASS(klass)				\
        (G_TYPE_CHECK_CLASS_TYPE((klass), MAFW_TYPE_SOURCE))
#define MAFW_SOURCE_GET_CLASS(obj)					\
        (G_TYPE_INSTANCE_GET_CLASS ((obj), MAFW_TYPE_SOURCE, MafwSourceClass))


typedef struct _MafwSource MafwSource;
typedef struct _MafwSourceClass MafwSourceClass;

/**
 * MAFW_SOURCE_KEY_WILDCARD:
 *
 * Wildcard to get all available metadata.
 */
#define MAFW_SOURCE_KEY_WILDCARD "*"

/**
 * MAFW_SOURCE_INVALID_BROWSE_ID:
 *
 * Invalid browse session id.
 */
#define MAFW_SOURCE_INVALID_BROWSE_ID (~0)

/**
 * MAFW_SOURCE_BROWSE_ALL:
 *
 * If passed as @item_count to mafw_source_browse(), it means that
 * the caller is interested in all resulting elements.
 */
#define MAFW_SOURCE_BROWSE_ALL (0)

extern const gchar * const _mafw_source_no_keys[];
 /**
 * MAFW_SOURCE_NO_KEYS:
 *
 * Value usable as @metadata_keys of browse(), denoting that the
 * caller is not interested in any metadata.  Its value is an
 * zero-length, %NULL-terminated array.
 */
#define MAFW_SOURCE_NO_KEYS _mafw_source_no_keys

extern const gchar * const _mafw_source_all_keys[];
/**
 * MAFW_SOURCE_ALL_KEYS:
 *
 * Value usable as @metadata_keys of mafw_source_browse() and
 * mafw_source_get_metadata(), denoting that the caller is interested
 * in all possible metadata the source can retrieve.  Its value is a
 * %NULL-terminated array with one single "*" element.
 */
#define MAFW_SOURCE_ALL_KEYS _mafw_source_all_keys

/**
 * MAFW_SOURCE_LIST:
 * @...: list of elements
 *
 * Constructs and returns a %NULL-terminated string array from the varadic
 * arguments, suitable to be passed to mafw_source_browse() as object IDs or
 * metadata tags of interest.  The arguments are evaluated only once and the
 * list can be empty.
 *
 * <note>
 * The returned array is allocated on the stack, thus it becomes
 * invalid after exiting the current frame.  To keep it afterwards
 * wrap it in g_strdupv().
 * </note>
 */
#define MAFW_SOURCE_LIST(...) \
	(const gchar *const[]){ __VA_ARGS__+0, NULL }

/**
 * MAFW_SOURCE_PTRLIST:
 * @...: list of elements
 *
 * Like MAFW_SOURCE_LIST() but returns an unterminated void-pointer array.
 */
#define MAFW_SOURCE_PTRLIST(...) \
	(const void *const[]){ __VA_ARGS__ }

/**
 * MAFW_SOURCE_MKPTRLIST:
 * @...: list of elements
 *
 * Like MAFW_SOURCE_PTRLIST() but returns a newly allocated array of pointers
 * of its arguments.
 * The arguments are evaluated only once.
 */
#define MAFW_SOURCE_MKPTRLIST(...) \
	g_memdup(MAFW_SOURCE_PTRLIST(__VA_ARGS__), \
		 sizeof(MAFW_SOURCE_PTRLIST(__VA_ARGS__)))

/**
 * MafwSourceBrowseResultCb:
 * @self:      The emitting #MafwSource.
 * @browse_id: The browse session ID that these results are associated to.
 * @remaining_count: Number of remaining results in the current browse session.
 * @index:     The current item's index in the current browse session.
 * @object_id: The object ID of the current item.
 * @metadata:  Metadata related to @object_id as #GValue:s. The tag names and
 *             the concrete types of their values are defined in <link
 *             linkend="mafw-MafwMetadata">MafwMetadata</link>. The source
 *             is expected to make its best effort to obtain all metadata
 *             keys requested in mafw_source_browse(), but there are no
 *             guarantees of them all being present in @metadata (usually
 *             because the source might not support some particular key).
 *             On the other hand, the source may include additional keys on
 *             its own, even if not requested by the caller, however this is
 *             not recommended.
 *             The table is destroyed by the source after the callback returns.
 * @user_data: Optional user data pointer passed to mafw_source_browse().
 * @error:     Non-%NULL if an error occurred.
 *
 * Callback prototype for browse results. For a successful mafw_source_browse()
 * call, the number of callbacks depends on the number of items requested and
 * number of items available. The callback should be called at least once,
 * unless mafw_source_browse() returned with an error.
 *
 * @object_id tells the unique object ID that @metadata is related to. It
 * should not be %NULL unless there are no results available. If an error has
 * occurred, @error is set non-%NULL, the contents of @metadata can be %NULL
 * and @remaining_count and @index might be invalid. No more results are to
 * be expected after a non-%NULL error.
 *
 * @remaining_count tells the number of results that are to be expected for
 * the browse session identified by @browse_id. If 10 items were requested
 * and the currently browsed container has at least 10 items, @remaining_count
 * should go from 9 to 0. The same goes for @index, which tells the index of
 * the current item within the current browse session; in the aforementioned
 * example result set @index should go from 0 to 9. When @remaining_count
 * reaches zero, the session has ended and no more results are to be expected.
 * @browse_id is also invalidated after receiving zero @remaining_count.
 *
 * when there are no more results available (which is not considered an error
 * situation) the source must call the callback exactly once with zero
 * @remaining_count. This applies also when the browsed container is completely
 * empty or @skip_count in mafw_source_browse() is beyond the container's
 * child count (in both of these cases @object_id is %NULL).
 *
 * Cancelling an ongoing browse session with mafw_source_cancel_browse() will
 * result in exactly one callback with zero @remaining_count. If the last item
 * has already been signalled (with zero @remaining_count), cancelling will
 * have no effect. Otherwise the browse session will terminate immediately
 * with one last termination signal having zero @remaining_count.
 */
typedef void (*MafwSourceBrowseResultCb)(MafwSource *self,
					  guint browse_id,
					  gint remaining_count,
					  guint index,
					  const gchar *object_id,
					  GHashTable *metadata,
					  gpointer user_data,
					  const GError *error);

/**
 * MafwSourceMetadataResultCb:
 * @self:      The emitting #MafwSource.
 * @object_id: The object ID of the current item.
 * @metadata:  Metadata of @object_id as #GValue's, identical to
 *             #MafwSourceBrowseResultCb.
 * @user_data: Optional user data pointer passed to mafw_source_get_metadata().
 * @error:     Non-%NULL if an error occurred.
 *
 * Callback prototype for metadata results. If both @object_id and @error are
 * set, then the error affects only the actual item (e.g. permission denied,
 * etc.). If @error is set, the contents of @metadata might be set %NULL.
 *
 * Exactly one callback is expected for a mafw_source_get_metadata() call.
 */
typedef void (*MafwSourceMetadataResultCb)(MafwSource *self,
					   const gchar *object_id,
					   GHashTable *metadata,
					   gpointer user_data,
					   const GError *error);

/**
 * MafwSourceMetadataSetCb:
 * @self:        The emitting #MafwSource.
 * @object_id:   The object ID of the modified object.
 * @failed_keys: Null-terminated array of failed metadata keys or %NULL 
 *               if the operation was completely successful.
 * @user_data:   Optional user data pointer passed to mafw_source_set_metadata().
 * @error:       Non-%NULL if an error occurred.
 *
 * Callback prototype for metadata editing/setting result. If @error is set,
 * @failed_keys contains a list of metadata keys that could not be set.
 *
 * Exactly one callback is expected for a mafw_source_set_metadata() call.
 */
typedef void (*MafwSourceMetadataSetCb)(MafwSource *self,
					const gchar *object_id,
					const gchar **failed_keys,
					gpointer user_data,
					const GError *error);

/**
 * MafwSourceObjectCreatedCb:
 * @self:      The emitting #MafwSource.
 * @object_id: The object ID of the newly created object or %NULL if it could
 *             not be created due to an error.
 * @user_data: Optional user data pointer passed to mafw_source_create_object().
 * @error:     Non-%NULL if an error occurred.
 *
 * Callback prototype for object creation result. If any errors were encountered
 * during object creation, @error is set non-%NULL, while @object_id might also
 * be %NULL.
 *
 * Exactly one callback is expected for a mafw_source_create_object() call.
 */
typedef void (*MafwSourceObjectCreatedCb)(MafwSource *self,
					  const gchar *object_id,
					  gpointer user_data,
					  const GError *error);

/**
 * MafwSourceObjectDestroyedCb:
 * @self:      The emitting #MafwSource.
 * @object_id: The object ID of the destroyed object.
 * @user_data: Optional user data pointer passed to mafw_source_destroy_object().
 * @error:     Non-%NULL if an error occurred.
 *
 * Callback prototype for object destruction result. If any errors were
 * encountered during object destruction, @error is set non-%NULL.
 *
 * Exactly one callback is expected for a mafw_source_destroy_object() call.
 */
typedef void (*MafwSourceObjectDestroyedCb)(MafwSource *self,
					    const gchar *object_id,
					    gpointer user_data,
					    const GError *error);

/**
 * MafwSource:
 *
 * MafwSource object structure
 */
struct _MafwSource {
	MafwExtension parent;
};

/**
 * MafwSourceClass:
 * @parent_class: parent structure.
 * @browse:         Virtual function for mafw_source_browse().
 * @cancel_browse:  Virtual function for mafw_source_cancel_browse().
 * @get_metadata:   Virtual function for mafw_source_get_metadata().
 * @set_metadata:   Virtual function for mafw_source_set_metadata().
 * @create_object:  Virtual function for mafw_source_create_object().
 * @destroy_object: Virtual function for mafw_source_destroy_object().
 *
 * Base class for MAFW source components.
 */
struct _MafwSourceClass {

	MafwExtensionClass parent_class;

	/* vtable */

	guint (*browse)(MafwSource *self,
			const gchar *object_id, gboolean recursive,
			const MafwFilter *filter, const gchar *sort_criteria,
		       	const gchar *const *mdkeys,
			guint skip_count, guint item_count,
			MafwSourceBrowseResultCb cb, gpointer user_data);

	gboolean (*cancel_browse)(MafwSource *self, guint browse_id,
				  GError **error);

	void (*get_metadata)(MafwSource *self,
				 const gchar *object_id,
				 const gchar *const *mdkeys,
				 MafwSourceMetadataResultCb cb,
				 gpointer user_data);

	void (*set_metadata)(MafwSource *self,
				 const gchar *object_id,
				 GHashTable *metadata,
				 MafwSourceMetadataSetCb cb,
				 gpointer user_data);

	void (*create_object)(MafwSource *self,
				  const gchar *parent, GHashTable *metadata,
				  MafwSourceObjectCreatedCb cb,
				  gpointer user_data);

	void (*destroy_object)(MafwSource *self,
				   const gchar *object_id,
				   MafwSourceObjectDestroyedCb cb,
				   gpointer user_data);
};

extern GType mafw_source_get_type(void);

extern gboolean mafw_source_all_keys(const gchar *const *keys);
extern guint mafw_source_browse(MafwSource *self,
				const gchar *object_id,
				gboolean recursive, const MafwFilter *filter,
				const gchar *sort_criteria,
				const gchar *const *metadata_keys,
				guint skip_count, guint item_count,
				MafwSourceBrowseResultCb browse_cb,
				gpointer user_data);

extern gboolean mafw_source_cancel_browse(MafwSource *self, guint browse_id, 
					  GError **error);

extern void mafw_source_get_metadata(MafwSource *self,
					 const gchar *object_id,
					 const gchar *const *metadata_keys,
					 MafwSourceMetadataResultCb metadata_cb,
					 gpointer user_data);

extern void mafw_source_set_metadata(MafwSource *self,
					 const gchar *object_id,
					 GHashTable *metadata,
					 MafwSourceMetadataSetCb cb,
					 gpointer user_data);

extern void mafw_source_create_object(MafwSource *self,
					  const gchar *parent,
					  GHashTable *metadata,
					  MafwSourceObjectCreatedCb cb,
					  gpointer user_data);

extern void mafw_source_destroy_object(MafwSource *self,
					   const gchar *object_id,
					   MafwSourceObjectDestroyedCb cb,
					   gpointer user_data);

extern gboolean mafw_source_split_objectid(gchar const *objectid,
					   gchar **extensionid, gchar **itemid);
extern gchar *mafw_source_create_objectid(const gchar *uri);

G_END_DECLS

#endif
/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
