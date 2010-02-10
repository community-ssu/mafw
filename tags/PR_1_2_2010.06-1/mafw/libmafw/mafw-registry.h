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

#ifndef __MAFW_REGISTRY_H__
#define __MAFW_REGISTRY_H__

#include <glib.h>
#include <glib-object.h>

#include <libmafw/mafw-source.h>
#include <libmafw/mafw-renderer.h>

/**
 * MAFW_PLUGIN_SUFFIX:
 *
 * Every plug-in should contain a #MafwPluginDescriptor structure ending in
 * this suffix.
 */
#define MAFW_PLUGIN_SUFFIX "_plugin_description"

#define MAFW_TYPE_REGISTRY \
        (mafw_registry_get_type())
#define MAFW_REGISTRY(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), MAFW_TYPE_REGISTRY, MafwRegistry))
#define MAFW_IS_REGISTRY(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj), MAFW_TYPE_REGISTRY))
#define MAFW_REGISTRY_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass), MAFW_TYPE_REGISTRY, MafwRegistryClass))
#define MAFW_IS_REGISTRY_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass), MAFW_TYPE_REGISTRY))
#define MAFW_REGISTRY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), MAFW_TYPE_REGISTRY, MafwRegistryClass))

typedef struct _MafwRegistryPrivate MafwRegistryPrivate;

/**
 * MafwRegistry:
 *
 * Registry instance.
 */
typedef struct _MafwRegistry MafwRegistry;
struct _MafwRegistry {
	GObject parent;

	MafwRegistryPrivate *priv;
};

/**
 * MafwRegistryClass:
 * @parent_class: parent structure.
 *
 * Registry class.
 */
typedef struct _MafwRegistryClass MafwRegistryClass;
struct _MafwRegistryClass {
	GObjectClass parent_class;
};

/**
 * MafwPluginDescriptorPublic:
 * @name:        The human-readable name of the plugin.  Not used currently,
 *               except for debugging purposes.
 * @description: Plugin description.
 * @version:     Plugin version.
 *
 * Information about a plugin, returned by mafw_registry_list_plugins().
 */
typedef struct _MafwPluginDescriptorPublic {
        gchar const *name;
        gchar const *description;
        gchar const *version;
} MafwPluginDescriptorPublic;

/**
 * MafwPluginDescriptor:
 * @pub:          the public part of the plugin descriptor.
 * @initialize:   After glib has been set up the framework calls this function,
 *                so the plugin can start discovering and registering its
 *                sources and renderers.  By that time #GType is initialized,
 *                and idle callbacks or #GSources can be added to the main
 *                loop.  This function should not block.
 * @deinitialize: If not %NULL, this function should deregister all sources
 *                and renderers of the plugin and release all claimed
 *                resources.  If ever, it is called when the framework is
 *                being shut down and releasing allocated resources ensure
 *                that profiling tools (such as valgrind) won't report memory
 *                leakage after program termination.
 *
 * This structure contains all the entry points of an plugin for the
 * framework.  Every plug-in should have exactly one global instance of these,
 * whose name is the concatenation of plugin-name and
 * %MAFW_INIT_PLUGIN_SUFFIX (e.g. "bar_plugin_descriptor").
 */
typedef struct _MafwPluginDescriptor {
        MafwPluginDescriptorPublic pub;
        gboolean (*initialize)(MafwRegistry *registry, GError **error);
        void (*deinitialize)(GError **error);
} MafwPluginDescriptor;

G_BEGIN_DECLS
GType mafw_registry_get_type(void);

MafwRegistry *mafw_registry_get_instance(void);

extern GList *mafw_registry_get_renderers(MafwRegistry *self);
extern GList *mafw_registry_get_sources(MafwRegistry *self);
extern MafwExtension *mafw_registry_get_extension_by_uuid(
	MafwRegistry *self, const gchar *uuid);

extern void mafw_registry_add_extension(MafwRegistry *self,
					 MafwExtension *extension);
extern void mafw_registry_remove_extension(MafwRegistry *self,
					    MafwExtension *extension);

extern gboolean mafw_registry_load_plugin(MafwRegistry *self,
					   const gchar *name,
					   GError **error);
extern gboolean mafw_registry_unload_plugin(MafwRegistry *self,
					     const gchar *name,
					     GError **error);
extern GList *mafw_registry_list_plugins(MafwRegistry *self);
extern void mafw_registry_load_plugins(MafwRegistry *self);
extern void mafw_registry_unload_plugins(MafwRegistry *self);

G_END_DECLS
#endif				/* __MAFW_REGISTRY_H__ */
/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
