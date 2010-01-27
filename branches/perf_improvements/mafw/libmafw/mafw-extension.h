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

#ifndef __MAFW_EXTENSION_H__
#define __MAFW_EXTENSION_H__

/* Include files */
#include <glib.h>
#include <glib-object.h>

/* Macros */
#define MAFW_TYPE_EXTENSION mafw_extension_get_type()
#define MAFW_EXTENSION(obj) \
	G_TYPE_CHECK_INSTANCE_CAST((obj), MAFW_TYPE_EXTENSION, MafwExtension)
#define MAFW_EXTENSION_CLASS(klass) \
	G_TYPE_CHECK_CLASS_CAST((klass), MAFW_TYPE_EXTENSION, MafwExtensionClass)
#define MAFW_EXTENSION_GET_CLASS(obj) \
	G_TYPE_INSTANCE_GET_CLASS((obj), MAFW_TYPE_EXTENSION, MafwExtensionClass)
#define MAFW_IS_EXTENSION(obj)					\
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAFW_TYPE_EXTENSION))

/* Type definitions */
typedef struct _MafwExtension MafwExtension;
typedef struct _MafwExtensionClass MafwExtensionClass;
typedef struct _MafwExtensionPrivate MafwExtensionPrivate;

/**
 * MafwExtensionProperty:
 * @name: name of the property.
 * @type: the #GType of the property.
 *
 * Information about a run-time property.
 */
typedef struct {
        gchar *name;
        GType type;
} MafwExtensionProperty;

/**
 * MafwExtension:
 *
 * Mafw SiSo object structure
 */
struct _MafwExtension {
	GInitiallyUnowned parent;

	/*< private >*/
	MafwExtensionPrivate *priv;
};

/**
 * MafwExtensionPropertyCallback:
 * @self: a #MafwExtension instance.
 * @name: the name of the returned property.
 * @value: the value of the property, owned by the callee.
 * @udata: additional data.
 * @error: non-%NULL if an error occurred.  Freed by the caller.
 *
 * Callback for asynchronous run-time property retrieval.  Ownership
 * of @value is passed to the callee.  If @error is set, @value is
 * %NULL.
 */
typedef void (*MafwExtensionPropertyCallback)(MafwExtension *self,
					 const gchar *name,
					 GValue *value,
					 gpointer udata,
					 const GError *error);

/**
 * MafwExtensionClass:
 * @parent_class: parent structure
 * @list_extension_properties: virtual function for returning supported
 * properties.  Subclasses must chain up to parent after finishing
 * their job.
 * @set_extension_property: virtual function that gets called via
 * mafw_extension_set_property().
 * @get_extension_property: virtual function for mafw_extension_get_property().
 *
 * Mafw SiSo class structure.
 */
struct _MafwExtensionClass {
	GInitiallyUnownedClass parent_class;
	/* Virtual functions. */
	const GPtrArray *(*list_extension_properties)(MafwExtension *self);
	void (*set_extension_property)(MafwExtension *self, const gchar *name,
				  const GValue *value);
	void (*get_extension_property)(MafwExtension *self, const gchar *name,
				  MafwExtensionPropertyCallback cb, gpointer udata);
};

/* Function prototypes */
G_BEGIN_DECLS
extern GType mafw_extension_get_type(void);

extern void mafw_extension_set_name(MafwExtension *self, const gchar *name);
extern const gchar *mafw_extension_get_name(MafwExtension *self);
extern const gchar *mafw_extension_get_uuid(MafwExtension *self);
extern const gchar *mafw_extension_get_plugin(MafwExtension *self);

extern void mafw_extension_add_property(MafwExtension *self, const gchar *name, GType type);
extern const GPtrArray *mafw_extension_list_properties(MafwExtension *self);
extern gboolean mafw_extension_set_property(MafwExtension *self, const gchar *name,
				       const GValue *value);
extern void mafw_extension_get_property(MafwExtension *self, const gchar *name,
				   MafwExtensionPropertyCallback cb,
				   gpointer udata);
extern void mafw_extension_emit_property_changed(MafwExtension *self,
					    const gchar *property,
					    const GValue *value);

/* Convenience functions. */

extern gboolean mafw_extension_set_property_string(MafwExtension *self,
					      const gchar *name,
					      const gchar *value);
extern gboolean mafw_extension_set_property_boolean(MafwExtension *self,
					       const gchar *name,
					       gboolean value);
extern gboolean mafw_extension_set_property_char(MafwExtension *self,
					    const gchar *name,
					    gchar value);
extern gboolean mafw_extension_set_property_uchar(MafwExtension *self,
					     const gchar *name,
					     guchar value);
extern gboolean mafw_extension_set_property_int(MafwExtension *self,
					   const gchar *name,
					   gint value);
extern gboolean mafw_extension_set_property_uint(MafwExtension *self,
					    const gchar *name,
					    guint value);
extern gboolean mafw_extension_set_property_long(MafwExtension *self,
					    const gchar *name,
					    glong value);
extern gboolean mafw_extension_set_property_ulong(MafwExtension *self,
					     const gchar *name,
					     gulong value);
extern gboolean mafw_extension_set_property_int64(MafwExtension *self,
					     const gchar *name,
					     gint64 value);
extern gboolean mafw_extension_set_property_uint64(MafwExtension *self,
					      const gchar *name,
					      guint64 value);
extern gboolean mafw_extension_set_property_float(MafwExtension *self,
					     const gchar *name,
					     gfloat value);
extern gboolean mafw_extension_set_property_double(MafwExtension *self,
					      const gchar *name,
					      gdouble value);

G_END_DECLS
#endif /* ! __MAFW_EXTENSION_H__ */
/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
