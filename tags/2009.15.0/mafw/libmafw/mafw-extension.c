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
#include "config.h"

#include <string.h>

#include "mafw-marshal.h"
#include "mafw-extension.h"
#include "mafw-errors.h"

/**
 * SECTION: mafwextension
 * @short_description: Base class for both sources and renderers
 *
 * #MafwExtension is an abstract base class common to both sources and
 * renderers.  It provides the name and uuid properties, as described in
 * #MafwRegistry.
 *
 * <emphasis>Run-time properties</emphasis>
 *
 * Sources and renderers may have class- (or even instance-) specific
 * run-time properties.  Examples are Brightness of the screen of an
 * UPnP renderer, or Volume, but subclasses are free to use this
 * facility for their own purposes.  If a subclass wants to have
 * run-time properties, it should call mafw_extension_add_property() for
 * each of its properties at some point (probably in the instance
 * initializer).  Known properties (and their types) may be listed
 * with mafw_extension_list_properties(); queried with
 * mafw_extension_get_property() and set using the
 * mafw_extension_set_property_*() family of functions.
 */

enum {
	PROP_0,
	PROP_NAME,
	PROP_UUID,
	PROP_PLUGIN,
};

struct _MafwExtensionPrivate {
	gchar *uuid;
	gchar *name;
	gchar *plugin;
	GPtrArray *rtprops;
};

enum {
	ERROR,
	PROPERTY_CHANGED,
	LAST_SIGNAL,
};

static guint Signals[LAST_SIGNAL];

/* Automatic code */
G_DEFINE_ABSTRACT_TYPE(MafwExtension, mafw_extension, G_TYPE_INITIALLY_UNOWNED);

static void get_property(MafwExtension *extension, guint prop_id,
			 GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string(value, extension->priv->name);
		break;
	case PROP_UUID:
		g_value_set_string(value, extension->priv->uuid);
		break;
	case PROP_PLUGIN:
		g_value_set_string(value, extension->priv->plugin);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(extension, prop_id, pspec);
		break;
	}
}


static void set_property(MafwExtension *extension, guint prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
        case PROP_NAME:
	{
		gchar *name;

		name = g_value_dup_string(value);
		g_return_if_fail(name);
		if (extension->priv->name)
			g_free(extension->priv->name);
		extension->priv->name = name;
		break;
	}
	case PROP_UUID:
		/* Construct-only, so extension->priv->uuid == NULL. */
		g_assert(extension->priv->uuid == NULL);
		extension->priv->uuid = g_value_dup_string(value);
		break;
	case PROP_PLUGIN:
		/* Construct-only, assert it was NULL. */
		g_assert(extension->priv->plugin == NULL);
		extension->priv->plugin = g_value_dup_string(value);
		break;
        default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(extension, prop_id, pspec);
		break;
	}
}

static void mafw_extension_finalize(MafwExtension *self)
{
	guint i;

	g_free(self->priv->uuid);
	g_free(self->priv->name);
	g_free(self->priv->plugin);
	for (i = 0; i < self->priv->rtprops->len; ++i) {
		g_free(((MafwExtensionProperty *)
		       self->priv->rtprops->pdata[i])->name);
		g_free(self->priv->rtprops->pdata[i]);
	}
	g_ptr_array_free(self->priv->rtprops, TRUE);
	G_OBJECT_CLASS(mafw_extension_parent_class)->finalize(G_OBJECT(self));
}

/* Methods */

/**
 * mafw_extension_get_uuid:
 * @self: a #MafwExtension instance.
 *
 * Queries the UUID of the extension.
 *
 * Returns: the UUID of the object.  The value is owned by the
 * #MafwExtension and should never be modified or freed.
 */
const gchar *mafw_extension_get_uuid(MafwExtension *self)
{
	g_return_val_if_fail(self != NULL, NULL);

	return self->priv->uuid;
}

/**
 * mafw_extension_set_name:
 * @self: a #MafwExtension instance.
 * @name: the new name of the extension.
 *
 * Sets the name of the extension to @name.
 */
void mafw_extension_set_name(MafwExtension *self, const gchar *name)
{
	g_object_set(self, "name", name, NULL);
}

/**
 * mafw_extension_get_name:
 * @self: a #MafwExtension instance.
 *
 * Queries the name of the extension.
 *
 * Returns: the name of the extension.  The value is owned by the #MafwExtension
 * and should never be modified or freed.
 */
const gchar *mafw_extension_get_name(MafwExtension *self)
{
	g_return_val_if_fail(self != NULL, NULL);

	return self->priv->name;
}

/**
 * mafw_extension_get_plugin:
 * @self: a #MafwExtension instance.
 *
 * Queries the name of the plugin that created @self.
 *
 * Returns: MafwExtension:name, owned by the extension, not modifiable by the user.
 */
const gchar *mafw_extension_get_plugin(MafwExtension *self)
{
	g_return_val_if_fail(self, NULL);
	return self->priv->plugin;
}

/*
 * MafwExtension's implementation returns the priv->rtprops array.
 */
static const GPtrArray *_extension_list_properties(MafwExtension *self)
{
	return self->priv->rtprops;
}

static void _extension_set_property(MafwExtension *self,
			       const gchar *name, const GValue *value)
{ /* NOP */ }

static void _extension_get_property(MafwExtension *self, const gchar *name,
			       MafwExtensionPropertyCallback cb, gpointer udata)
{
	GError *err;

	err = g_error_new_literal(MAFW_EXTENSION_ERROR,
				  MAFW_EXTENSION_ERROR_INVALID_PROPERTY,
				  "Base class has no properties at all.");
	cb(self, name, NULL, udata, err);
	g_error_free(err);
}

static MafwExtensionProperty *find_prop(MafwExtension *self, const gchar *name)
{
	guint i;
	for (i = 0; i < self->priv->rtprops->len; ++i)
		if (!strcmp(((MafwExtensionProperty *)
			     self->priv->rtprops->pdata[i])->name, name))
			return self->priv->rtprops->pdata[i];
	return NULL;
}

/**
 * mafw_extension_add_property:
 * @self: a #MafwExtension instance.
 * @name: the name of the property.
 * @type: the #GType of the property.
 *
 * Adds @name to the list of supported properties of this #MafwExtension
 * instance.  This function exists for the convenience of subclasses
 * (to call e.g. in their list_extension_properties implementation).  It
 * does nothing if a property already exists with the given name.
 */
void mafw_extension_add_property(MafwExtension *self, const gchar *name, GType type)
{
	MafwExtensionProperty *p;

	if (find_prop(self, name)) return;
	p = g_new(MafwExtensionProperty, 1);
	p->name = g_strdup(name);
	p->type = type;
	g_ptr_array_add(self->priv->rtprops, p);
}

/**
 * mafw_extension_list_properties:
 * @self: a #MafwExtension instance.
 *
 * List the SiSo properties.
 *
 * Returns: a #GPtrArray containing pointers to #MafwExtensionProperty
 * structures.  The caller should NOT modify it in any way.  It is
 * assumed that the returned list will not change after the first
 * invocation of this function (callers may cache it).
 */
const GPtrArray *mafw_extension_list_properties(MafwExtension *self)
{
	return MAFW_EXTENSION_GET_CLASS(self)->list_extension_properties(self);
}

/**
 * mafw_extension_set_property:
 * @self: a #MafwExtension instance.
 * @name: name of the property.
 * @value: the value to set.
 *
 * Sets the run-time property named @name on @self.  This may happen
 * asynchronously, so the new value may not be in effect for a while.
 * If the change is in effect #MafwExtension::property-changed will be
 * emitted. If some error happens, the interested is notified using
 * #MafwExtension::error (codes from #MafwExtensionError).
 *
 * Returns: %FALSE if the property does not exists or it's type is
 * different than of @value.
 */
gboolean mafw_extension_set_property(MafwExtension *self,
				const gchar *name, const GValue *value)
{
	MafwExtensionProperty *p;

	g_return_val_if_fail(name, FALSE);

	/* Let's check for type.  Probably there won't be too many of
	 * them, making lookup a big overhead.
	 *
	 * FIXME but if we check the type we disallow the arbitrary
	 * keys use case, e.g. set("services/foo/username", "bar"). */
	p = find_prop(self, name);
	if (!p || p->type != G_VALUE_TYPE(value)) return FALSE;
	MAFW_EXTENSION_GET_CLASS(self)->set_extension_property(self, name, value);
	return TRUE;
}

/* Convenience functions. */

/**
 * mafw_extension_set_property_string:
 * @self: a #MafwExtension object
 * @name: property name
 * @value: value
 *
 * Sets a property of value #gchar*.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 */
gboolean mafw_extension_set_property_string(MafwExtension *self,
				       const gchar *name, const gchar *value)
{
	GValue v = { 0 };
	gboolean r;

	g_value_init(&v, G_TYPE_STRING);
	g_value_set_static_string(&v, value);
	r = mafw_extension_set_property(self, name, &v);
	g_value_unset(&v);
	return r;
}

/* Let emacs do the doc comments. */
/* And let's make cpp do the boring work. */
#undef SNP
#define SNP(lctype, uctype)						\
	gboolean mafw_extension_set_property_##lctype(MafwExtension *self,	\
						 const gchar *name,	\
						 g##lctype value)	\
	{								\
		GValue v = { 0 };					\
		gboolean r;						\
		g_value_init(&v, uctype);				\
		g_value_set_##lctype(&v, value);			\
		r = mafw_extension_set_property(self, name, &v);		\
		g_value_unset(&v);					\
		return r;						\
	}

/**
 * mafw_extension_set_property_boolean:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #gboolean.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(boolean, G_TYPE_BOOLEAN)
/**
 * mafw_extension_set_property_char:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #gchar.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(char, G_TYPE_CHAR)
/**
 * mafw_extension_set_property_uchar:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #guchar.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(uchar, G_TYPE_UCHAR)
/**
 * mafw_extension_set_property_int:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #gint.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(int, G_TYPE_INT)
/**
 * mafw_extension_set_property_uint:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #guint.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(uint, G_TYPE_UINT)
/**
 * mafw_extension_set_property_long:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #glong.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(long, G_TYPE_LONG)
/**
 * mafw_extension_set_property_ulong:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #gulong.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(ulong, G_TYPE_ULONG)
/**
 * mafw_extension_set_property_int64:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #gint64.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(int64, G_TYPE_INT64)
/**
 * mafw_extension_set_property_uint64:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #guint64.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(uint64, G_TYPE_UINT64)
/**
 * mafw_extension_set_property_float:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #gfloat.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(float, G_TYPE_FLOAT)
/**
 * mafw_extension_set_property_double:
 * @self: a #MafwExtension instance
 * @name: name
 * @value: value
 *
 * Sets a value of type #gdouble.
 *
 * Returns: %TRUE if success, %FALSE otherwise.
 */
SNP(double, G_TYPE_DOUBLE)

#undef SNP


/**
 * mafw_extension_emit_property_changed:
 * @self: a #MafwExtension instance.
 * @property: property to signal
 * @value: #GValue with property value.
 *
 * Emits #MafwExtension::property-changed with @property as detail.  This is
 * a helper function for subclasses, who should call this after the
 * property change initiated with mafw_extension_set_property() is in
 * effect.
 */
void mafw_extension_emit_property_changed(MafwExtension *self, const gchar *property,
				     const GValue *value)
{
	g_signal_emit(self, Signals[PROPERTY_CHANGED],
		      g_quark_from_string(property),
		      property, value);
}

/**
 * mafw_extension_get_property:
 * @self: a #MafwExtension instance.
 * @name: name of a property.
 * @cb: a function to call with the result.
 * @udata: additional data passed to @cb.
 *
 * Queries a run-time property asynchronously.  The callback will be
 * always invoked (with a #GError if appropriate).
 */
void mafw_extension_get_property(MafwExtension *self, const gchar *name,
			    MafwExtensionPropertyCallback cb,
			    gpointer udata)
{
	g_return_if_fail(cb);
	g_return_if_fail(name);
	if (!find_prop(self, name)) {
		GError *err;

		err = g_error_new(MAFW_EXTENSION_ERROR,
				  MAFW_EXTENSION_ERROR_INVALID_PROPERTY,
				  "Unknown property: %s", name);
		cb(self, name, NULL, udata, err);
		g_error_free(err);
		return;
	}
	MAFW_EXTENSION_GET_CLASS(self)->get_extension_property(self, name, cb, udata);
}

#ifndef G_PARAM_STATIC_STRINGS
#define G_PARAM_STATIC_STRINGS \
	(G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB)
#endif

/* Class construction */
static void mafw_extension_class_init(MafwExtensionClass *cls)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(cls);

	g_type_class_add_private(cls, sizeof(MafwExtensionPrivate));

	gobject_class->finalize = (void *)mafw_extension_finalize;
	gobject_class->set_property = (gpointer)set_property;
	gobject_class->get_property = (gpointer)get_property;

	cls->list_extension_properties = _extension_list_properties;
	cls->set_extension_property = _extension_set_property;
	cls->get_extension_property = _extension_get_property;

	/**
	 * MafwExtension:name
	 *
	 * The name of the extension.
	 */
	g_object_class_install_property(
		gobject_class, PROP_NAME,
		g_param_spec_string("name", "Name",
				    "The name of the extension",
				    "",
				    G_PARAM_READWRITE |
				    G_PARAM_CONSTRUCT |
				    G_PARAM_STATIC_STRINGS));
	/**
	 * MafwExtension:uuid
	 *
	 * The read-only UUID of the extension, MUST be given at
	 * construction-time and it is not modifiable afterwards.
	 */
	g_object_class_install_property(
		gobject_class, PROP_UUID,
		g_param_spec_string("uuid", "Uuid",
				    "The UUID of the extension",
				    NULL,
				    G_PARAM_READWRITE |
				    G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS));
	/**
	 * MafwExtension:plugin
	 *
	 * Name of the plugin which created the extension.  MUST be given at
	 * construction-time and it is not modifiable afterwards.
	 */
	g_object_class_install_property(
		gobject_class, PROP_PLUGIN,
		g_param_spec_string("plugin", "Plugin",
				    "Name of the extension's parent plugin",
				    NULL,
				    G_PARAM_READWRITE |
				    G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS));
	/**
         * MafwExtension::error:
	 * @self: a #MafwExtension instance.
	 * @domain: GQuark representing the error domain string.
	 * @code: an error code.
	 * @message: Error description string
         *
	 * A signal indicating that a spontaneous error has occured.
	 */
	Signals[ERROR] =
		g_signal_new("error",
			     G_TYPE_FROM_CLASS(cls),
			     G_SIGNAL_RUN_FIRST,
			     0, NULL, NULL,
			     mafw_marshal_VOID__UINT_INT_STRING,
			     G_TYPE_NONE, 3,
			     G_TYPE_UINT, G_TYPE_INT, G_TYPE_STRING);
	/**
         * MafwExtension::property-changed:
	 * @self: a #MafwExtension instance.
	 * @name: the name of the changed property.
	 * @val:  a #GValue representing the new value.
         *
	 * A detailed signal indicating that a run-time property has
	 * changed ("property-changed::foo" is emitted if a property
	 * named "foo" changed).  Note that `changed' does not
	 * necessary mean a different value, but the fact that someone
	 * has issued mafw_extension_set_property(). The handler prototype
	 * looks like the following:
	 *
	 * <programlisting>
         * void property_changed(MafwExtension *object, const gchar *name,
	 *                       const GValue *value);
         * </programlisting>
	 */
	Signals[PROPERTY_CHANGED] =
		g_signal_new("property-changed",
			     G_TYPE_FROM_CLASS(cls),
			     G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
			     0, NULL, NULL,
			     mafw_marshal_VOID__STRING_BOXED,
			     G_TYPE_NONE, 2,
			     G_TYPE_STRING, G_TYPE_VALUE);
}

/* Object construction */
static void mafw_extension_init(MafwExtension *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
						 MAFW_TYPE_EXTENSION,
						 MafwExtensionPrivate);
	memset(self->priv, 0, sizeof(*self->priv));
	self->priv->rtprops = g_ptr_array_new();
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
