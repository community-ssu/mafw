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
#include <gmodule.h>

#include "mafw-registry.h"
#include "mafw-extension.h"
#include "mafw-errors.h"
#include "mafw-marshal.h"
#include "mafw-uri-source.h"

#undef  G_LOG_DOMAIN
#define G_LOG_DOMAIN "mafw-registry"

/**
 * SECTION:mafwregistry
 * @short_description: Registry for UI to find sources and renderers
 *
 * #MafwRegistry is the starting point for an application using the
 * framework.  It stores available renderers and sources, and provides signals
 * for addition and removal events.
 *
 * After getting a registry instance with mafw_registry_get_instance(), an
 * application can load plugins via mafw_registry_load_plugin(), or let
 * mafw_registry_load_plugins() load all of them residing in
 * $MAFW_PLUGIN_DIR (see #MAFW_DEFAULT_PLUGIN_DIR).
 *
 * <emphasis>UUIDs:</emphasis>
 *
 * Sources and renderers are identified by an arbitrary long unique string,
 * called `Source ID' or `Renderer ID' (generally referred to as the 'uuid' of
 * an extension).  The contents have no meaning other than identifying a
 * component.  The only restriction is that it MUST NOT contain the substring
 * "::" (that is two colons).
 *
 * Currently available renderers and sources can be queried with
 * mafw_registry_get_renderers() and mafw_registry_get_sources(), or a
 * specific instance can be retrieved with
 * mafw_registry_get_extension_by_uuid().
 *
 * Renderer and source plugins use mafw_registry_add_extension(), and
 * mafw_registry_remove_extension() to register or de-register their
 * components.
 */

/**
 * MAFW_DEFAULT_PLUGIN_DIR:
 *
 * Default plug-in search directory.  Overridable via the $MAFW_PLUGIN_DIR
 * environment variable (which MUST be an absolute path).
 */
#define MAFW_DEFAULT_PLUGIN_DIR MAFW_PREFIX "/lib/mafw-plugin/"

G_DEFINE_TYPE(MafwRegistry, mafw_registry, G_TYPE_OBJECT);

/* Type definitions */
struct _MafwRegistryPrivate {
        /* List of available renderers */
        GList *renderers;
        /* List of available sources */
        GList *sources;
	/* Loaded plugin list. List of #MafwRegistryPlugin */
	GList *plugin_list;
};

enum {
	RENDERER_ADDED = 0,
	RENDERER_REMOVED,
	SOURCE_ADDED,
	SOURCE_REMOVED,
	LAST_SIGNAL,
};

typedef struct _MafwRegistryPlugin MafwRegistryPlugin;

/**
 * Structure, what contains all the needed data for an plugin
 */
struct _MafwRegistryPlugin {
        GModule *handle; //Handle pointer of the opened module
        gchar *name; // The name of the plugin-structure in the module
        MafwPluginDescriptor *descriptor; // descriptor structure
};

/* Function prototypes */
static void mafw_registry_dispose(GObject *object);
static void unload_plugin(MafwRegistryPlugin *ext);

/* Private variables */
static guint mafw_registry_signals[LAST_SIGNAL];

/* Program code */
/* Private functions */

/*
 * find_plugin:
 * a: a #MafwRegistryPlugin
 * b: name to find (gchar*)
 *
 * This function is used by g_list_find_custom to find an plugin
 * with @name in the plugin list.
 */
static gint find_plugin(const MafwRegistryPlugin *ext, const gchar *name)
{
        return strcmp(ext->name, name);
}

/*
 * Unloads @ext and frees associated memory.
 */
static void unload_plugin(MafwRegistryPlugin *ext)
{
        GError *error = NULL;

        if (ext->descriptor->deinitialize)
                ext->descriptor->deinitialize(&error);
        if (error)
                g_warning("Plugin deinitialization failed: %s",
			  error->message);
	/* XXX: and here we crash */
        g_module_close(ext->handle);
        g_free(ext->name);
        g_free(ext);
}

/* I'm sure you don't see GList ***:s too often.  Finds out the appropriate
 * chain for $extension and returns whether it's a source or not. */
static gboolean extension_details(MafwRegistry *self,
				  MafwExtension *extension,
				  GList ***listpp)
{
	g_assert(extension != NULL);

	if (MAFW_IS_SOURCE(extension)) {
		*listpp = &self->priv->sources;
		return TRUE;
	} else if (MAFW_IS_RENDERER(extension)) {
		*listpp = &self->priv->renderers;
		return FALSE;
	} else
		g_assert_not_reached();
}

static MafwExtension *find_extension(MafwRegistry *self,
				      GList *list, const gchar *uuid)
{
	if (uuid == NULL)
		return NULL;

	for (; list; list = g_list_next(list))
		if (!strcmp(mafw_extension_get_uuid(list->data), uuid))
			return list->data;
	return NULL;
}

static void remove_extension_link(MafwRegistry *self, GList *link,
				  GList **listp, guint sig)
{
	g_assert(link != NULL);

	*listp = g_list_remove_link(*listp, link);
	g_signal_emit(self, sig, 0, link->data);
	g_object_unref(link->data);
	g_list_free_1(link);
}

/* Class construction */
static void mafw_registry_class_init(MafwRegistryClass *klass)
{
	g_type_class_add_private(klass, sizeof(MafwRegistryPrivate));

	/* Virtual methods */
	G_OBJECT_CLASS(klass)->dispose = mafw_registry_dispose;

	/* Signals */

	/**
         * MafwRegistry::renderer-added:
         * @self: a #MafwRegistry instance.
         * @renderer: the #MafwRenderer which was added.
         *
         * Emitted when a new renderer is added.
         */
	mafw_registry_signals[RENDERER_ADDED] =
	    g_signal_new("renderer-added",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 0,
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__OBJECT,
			 G_TYPE_NONE, 1, MAFW_TYPE_RENDERER);

	/**
         * MafwRegistry::renderer-removed:
         * @self: a #MafwRegistry instance.
         * @renderer: the #MafwRenderer object being removed.
         *
         * Emitted when a renderer is removed.
         */
	mafw_registry_signals[RENDERER_REMOVED] =
	    g_signal_new("renderer-removed",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 0,
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__OBJECT,
			 G_TYPE_NONE, 1, MAFW_TYPE_RENDERER);

	/**
         * MafwRegistry::source-added:
         * @self: a #MafwRegistry instance.
         * @renderer: the #MafwSource which was added.
         *
         * Emitted when a new source is added.
         */
	mafw_registry_signals[SOURCE_ADDED] =
	    g_signal_new("source-added",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 0,
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__OBJECT,
			 G_TYPE_NONE, 1, MAFW_TYPE_SOURCE);

	/**
         * MafwRegistry::source-removed:
         * @self: a #MafwRegistry instance.
         * @renderer: the #MafwSource which is being removed.
         *
         * Emitted when a source is removed.
         */
	mafw_registry_signals[SOURCE_REMOVED] =
	    g_signal_new("source-removed",
			 G_TYPE_FROM_CLASS(klass),
			 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			 0,
			 NULL,
			 NULL,
			 g_cclosure_marshal_VOID__OBJECT,
			 G_TYPE_NONE, 1, MAFW_TYPE_SOURCE);
}

/**
 * mafw_registry_get_instance:
 *
 * Unless already created, this function instantiate it from #MafwRegistry.
 * The returned object must not be unref()ed.
 *
 * Returns: the framework's Extension registry object.
 */
MafwRegistry *mafw_registry_get_instance(void)
{
        static MafwRegistry *singleton_registry;
        if (!singleton_registry)
                singleton_registry = g_object_new(MAFW_TYPE_REGISTRY, NULL);
        return singleton_registry;
}

static void mafw_registry_init(MafwRegistry *self)
{
	g_return_if_fail(MAFW_IS_REGISTRY(self));

	self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
						 MAFW_TYPE_REGISTRY,
						 MafwRegistryPrivate);
	memset(self->priv, 0, sizeof(*self->priv));
}

static void mafw_registry_dispose(GObject *object)
{
	MafwRegistry *self;

	g_return_if_fail(object != NULL);
	g_return_if_fail(MAFW_IS_REGISTRY(object));

	self = MAFW_REGISTRY(object);
	while (self->priv->sources)
		remove_extension_link(self,
				 self->priv->sources, &self->priv->sources,
				 mafw_registry_signals[SOURCE_REMOVED]);
	while (self->priv->renderers)
		remove_extension_link(self,
				 self->priv->renderers, &self->priv->renderers,
				 mafw_registry_signals[RENDERER_REMOVED]);

	g_list_foreach(self->priv->plugin_list,
                       (GFunc)unload_plugin, NULL);
        g_list_free(self->priv->plugin_list);
        self->priv->plugin_list = NULL;

	G_OBJECT_CLASS(mafw_registry_parent_class)->dispose(object);
}

/*
 * 1. basename
 * 2. eat everything after first dot
 * 3. replace - with _
 * 4. append MAFW_PLUGIN_SUFFIX
 */
static gchar *plugin_symbol_name(const gchar *fn)
{
        gchar *dot, *t, *pn;

        t = g_path_get_basename(fn);
        dot = strchr(t, '.');
        if (dot) *dot = 0;
        if (!g_str_has_suffix(t, MAFW_PLUGIN_SUFFIX))
                pn = g_strconcat(t, MAFW_PLUGIN_SUFFIX, NULL);
        else
                pn = g_strdup(t);
        pn = g_strdelimit(pn, "-", '_');
        g_free(t);
        return pn;
}

/*
 * register_plugin:
 * @self:          a #MafwRegistry instance.
 * @descriptor:    the descriptor structure.
 * @module_handle: #GModule handle of its module.
 * @name:          the name of the plugin.
 * @error:         location for a #GError
 *
 * Registers the plug-in identified by @descriptor, adding it to the
 * registry's private list and calling its initialization function.
 *
 * Returns: %TRUE if successful, %FALSE otherwise, in this case, @error will
 * be set.
 */
static gboolean register_plugin(MafwRegistry *self,
				MafwPluginDescriptor *descriptor,
				GModule *module_handle, const gchar *name,
				GError **error)
{
        MafwRegistryPlugin *new_plugin;
	gboolean init_ok;
        GError *serror;

        g_return_val_if_fail(NULL != descriptor, FALSE);
        g_assert(NULL != descriptor->pub.name);
        g_assert(NULL != descriptor->initialize);
        g_assert(NULL != module_handle);
        g_assert(NULL != name);

	serror = NULL;
	init_ok = descriptor->initialize(MAFW_REGISTRY(self), &serror);
        if (!init_ok || serror) {
                if (serror)
			g_propagate_error(error, serror);
		else
			g_set_error(error,
				    MAFW_ERROR,
				    MAFW_ERROR_PLUGIN_INIT_FAILED,
				    "plugin initialize() failed for '%s'",
				    name);
                return FALSE;
        }
        new_plugin              = g_new0(MafwRegistryPlugin, 1);
        new_plugin->handle      = module_handle;
        new_plugin->descriptor  = descriptor;
        new_plugin->name        = plugin_symbol_name(name);
        self->priv->plugin_list = g_list_append(self->priv->plugin_list,
						new_plugin);
        return TRUE;
}

/*
 * Tries to g_module_open(@fn) and sets @err if failed.  Returns the
 * handle.
 */
static GModule *modopen(const gchar *fn, GError **err)
{
        GModule *h;

        h = g_module_open(fn, G_MODULE_BIND_LAZY);
        if (!h) {
                g_set_error(err,
                            MAFW_ERROR,
                            MAFW_ERROR_PLUGIN_LOAD_FAILED,
                            "%s", g_module_error());
        }
        return h;
}

/*
 * Loads the plug-in structure @name (without the suffix) from the
 * module identified by @handle.  It fails if it's already registered
 * or the symbol cannot be found in the module.  If something fails,
 * @handle will be closed and @error will be filled.
 */
static gboolean load_from_module(MafwRegistry *self,
                                 GModule *handle, const gchar *name,
                                 GError **error)
{
        MafwPluginDescriptor *desc;
        gchar *plugin_sym;
        GError *serror = NULL;
        gboolean isok;

        g_assert(handle);
        g_assert(name);

        isok = FALSE;
        plugin_sym = plugin_symbol_name(name);
        if (g_list_find_custom(self->priv->plugin_list, plugin_sym,
			       (GCompareFunc)find_plugin)) {
                g_set_error(error,
                            MAFW_ERROR,
                            MAFW_ERROR_PLUGIN_NAME_CONFLICT,
                            "A plugin named '%s' already exists",
                            name);
                goto err;
        }

        if (!g_module_symbol(handle, plugin_sym, (gpointer)&desc)) {
                g_set_error(error,
                            MAFW_ERROR,
                            MAFW_ERROR_PLUGIN_LOAD_FAILED,
                            "Plugin '%s' does not contain the symbol '%s'",
                            name, plugin_sym);
                goto err;
        }
        isok = register_plugin(self, desc, handle, name, &serror);
        if (serror)
                g_propagate_error(error, serror);
err:
        if (!isok)
		g_module_close(handle);
        g_free(plugin_sym);
        return isok;
}

static const gchar *get_plugin_dir(void)
{
	const gchar *dir;

	dir = g_getenv("MAFW_PLUGIN_DIR");
	if (!dir)
		dir = MAFW_DEFAULT_PLUGIN_DIR;
	return dir;
}

/**
 * mafw_registry_load_plugins:
 * @self:  a #MafwRegistry instance.
 *
 * Tries to load all plugins from $MAFW_PLUGIN_DIR.
 */
void mafw_registry_load_plugins(MafwRegistry *self)
{
        GDir *dir;
        const gchar *current_file, *plugindir;
        GError *error;

	plugindir = get_plugin_dir();
        dir = g_dir_open(plugindir, 0, NULL);
        /* If opendir() failed then the directory does not exists,
         * therefore we don't have to load anything. */
        if (!dir)
		return;
        while ((current_file = g_dir_read_name(dir)) != NULL) {
                gchar *fn;

		if (!g_str_has_suffix(current_file, "." G_MODULE_SUFFIX))
			continue;
                error = NULL;
                fn = g_build_filename(plugindir, current_file, NULL);
                mafw_registry_load_plugin(self, fn, &error);
                if (error != NULL) {
			g_warning("Couldn't load plugin '%s': %s",
				  fn, error->message);
                        g_error_free(error);
                }
                g_free(fn);
        }
        g_dir_close(dir);
}

/**
 * mafw_registry_load_plugin:
 * @self:  a #MafwRegistry instance.
 * @name:  name of the plug-in.
 * @error: location for a #GError, or %NULL if the caller is not
 *         interested.
 *
 * Loads a plug-in.  It consists of two steps: finding the shared object, and
 * the plugin descriptor structure in it.
 *
 * A so-called `plugin-name' is derived from @name by replacing all
 * dashes '-' with underscores '_'.  If @name was an absolute
 * filename, the above is applied to the string produced by taking the
 * basename and removing the ".so" suffix.  For example,
 * "/tmp/my-best.so" => "my_best".  The plugin-name is used to form a
 * `plugin-symbol', by concatenating it with #MAFW_PLUGIN_SUFFIX
 * (e.g. "my_best_plugin_description").  (@name should only contain
 * alphanumeric characters, and either dashes '-' or underscores '_'.)
 *
 * <note><para>The loaded plugin will be remembered by its
 * plugin-name.</para></note>
 *
 * After we have a plugin-symbol and if @name was an absolute filename, the
 * symbol is looked up in that particular file.  If it is not found, no other
 * places are tried.  On the other hand, if it was not an absolute filename,
 * then it is searched in the following order:
 *
 * <orderedlist>
 * <listitem>in the address space of the current process</listitem>
 * <listitem> $MAFW_PLUGIN_DIR/&lt;plugin-name&gt;.so</listitem>
 * <listitem> let g_module_open() resolve it</listitem>
 * </orderedlist>
 *
 * The symbol should refer to a #MafwPluginDescriptor structure.
 *
 * Returns: %TRUE if the plug-in was loaded an initialized successfully.
 */
gboolean mafw_registry_load_plugin(MafwRegistry *self,
				    const gchar *name,
				    GError **error)

{
        GModule *handle;
        GError *serror = NULL;

        g_return_val_if_fail(name != NULL, FALSE);

        if (!g_module_supported()) {
                g_set_error(error, MAFW_ERROR,
                            MAFW_ERROR_PLUGINS_NOT_SUPPORTED,
                            "Module loading not supported: %s",
                            g_module_error());
                return FALSE;
        }

        if (name[0] == '/') {
                handle = modopen(name, &serror);
                if (serror) goto err;

                load_from_module(self, handle, name, &serror);
                if (serror) goto err;
        } else {

                handle = modopen(NULL, &serror);
                if (serror) goto err;

                load_from_module(self, handle, name, &serror);

                if (serror) {
                        gchar *full_path;

                        /* Only try ext-dir if LOAD_FAILED. */
                        if ((serror)->code != MAFW_ERROR_PLUGIN_LOAD_FAILED)
                                goto err;

			/* Superfluous separators should not hurt. */
			full_path = g_strconcat(get_plugin_dir(),
						G_DIR_SEPARATOR_S,
						name, "." G_MODULE_SUFFIX,
						NULL);
                        g_clear_error(&serror);
                        handle = modopen(full_path, &serror);
                        g_free(full_path);
                        if (serror) {
                                /* Not found in $MAFW_PLUGIN_DIR,
                                 * let dlopen() sort it out. */
                                g_clear_error(&serror);
                                handle = modopen(name, &serror);
                                if (serror) goto err;
                        }

                        load_from_module(self, handle, name, &serror);
                        if (serror) goto err;
                }
        }

        return TRUE;
err:
        g_propagate_error(error, serror);
        return FALSE;
}

/**
 * mafw_registry_unload_plugin:
 * @self:  a #MafwRegistry instance.
 * @name:  name of the plug-in to unload.
 * @error: location for a #GError, or %NULL.
 *
 * Unloads a plug-in after deinitializing it.  The plugin-name is derived from
 * @name the same way as in mafw_registry_load_plugin().
 *
 * Returns: %TRUE on sucess on request, %FALSE otherwise
 */
gboolean mafw_registry_unload_plugin(MafwRegistry *self,
				      const gchar *name,
				      GError **error)
{
        GList *item;
        gchar *plugin_sym = plugin_symbol_name(name);

        item = g_list_find_custom(self->priv->plugin_list, plugin_sym,
                                  (GCompareFunc)find_plugin);

        g_free(plugin_sym);

        if (item == NULL) {
                g_set_error(error,
                            MAFW_ERROR,
                            MAFW_ERROR_PLUGIN_NOT_LOADED,
                            "Plugin '%s' is not loaded",
                            name);
                return FALSE;
        }
        unload_plugin(item->data);
        self->priv->plugin_list =
                g_list_delete_link(self->priv->plugin_list,
                                   item);
        return TRUE;
}

/**
 * mafw_registry_list_plugins:
 * @self: a #MafwRegistry instance
 *
 * This method gives information about the plugins this registry instance has
 * loaded and initialized successfully.
 *
 * Returns: a list of #MafwPluginDescriptorPublic structures.  The
 * caller owns the list, but may not change the list contents. The
 * list is not kept up-to-data with changes in the list of loaded
 * plugins and becomes invalid when one is unloaded.  Free the list
 * with g_list_free().
 */
GList *mafw_registry_list_plugins(MafwRegistry *self)
{
        GList *li, *lo;

        /* Create the return list from .pub of .plugin_list. */
        lo = NULL;
        for (li = self->priv->plugin_list; li; li = li->next)
                lo = g_list_prepend(lo, &((MafwRegistryPlugin *)(li->data))->descriptor->pub);
        return lo;
}

/**
 * mafw_registry_unload_plugins:
 * @self:  a #MafwRegistry instance.
 *
 * Attempts to unload all plugins @self knows about, closing their containing
 * shared object.  Successful or not all plugins will be forgotten.
 */
void mafw_registry_unload_plugins(MafwRegistry *self)
{
        while (self->priv->plugin_list) {
                unload_plugin(self->priv->plugin_list->data);
                self->priv->plugin_list = g_list_delete_link(
                                                self->priv->plugin_list,
                                                self->priv->plugin_list);
        }
}

/* Methods */
/**
 * mafw_registry_get_renderers:
 * @self: a #MafwRegistry object.
 *
 * Gets the list of available renderers.
 *
 * Returns: a #GList of available renderers, which should NOT be freed.
 */
GList *mafw_registry_get_renderers(MafwRegistry *self)
{
	g_assert(self != NULL);

	return self->priv->renderers;
}

/**
 * mafw_registry_get_sources:
 * @self: a #MafwRegistry object.
 *
 * Gets the list of available sources.
 *
 * Returns: a #GList of available sources, which should NOT be freed.
 */
GList *mafw_registry_get_sources(MafwRegistry *self)
{
	g_assert(self != NULL);

	return self->priv->sources;
}

/* Registry manipulation (used by the extension, DO NOT use from UI) */

/**
 * mafw_registry_add_extension:
 * @self: a #MafwRegistry object.
 * @extension: either a #MafwRenderer or a #MafwSource to be added.
 *
 * Adds a #MafwExtension to the registry.  @extension must have a UUID.
 * There may not exist another extension of the same type with the same UUID
 * as @extension. The registry will own the reference of the added extension.
 */
void mafw_registry_add_extension(MafwRegistry *self,
				  MafwExtension *extension)
{
	guint sig;
	GList **listp;

	g_assert(self != NULL);
	sig = extension_details(self, extension, &listp)
	       	? mafw_registry_signals[SOURCE_ADDED]
		: mafw_registry_signals[RENDERER_ADDED];
	g_assert(mafw_extension_get_uuid(extension) != NULL);
	g_assert(!find_extension(self, *listp,
				 mafw_extension_get_uuid(extension)));

	g_object_ref_sink(extension);
	*listp = g_list_prepend(*listp, extension);
	g_signal_emit(self, sig, 0, extension);
}

/**
 * mafw_registry_remove_extension:
 * @self: a #MafwRegistry object.
 * @extension: either a #MafwRenderer or a #MafwSource object to remove.
 *
 * Removes a #MafwExtension from the registry.  @extension must be registered
 * beforehand.
 */
void mafw_registry_remove_extension(MafwRegistry *self,
				     MafwExtension *extension)
{
	guint sig;
	GList **listp;

	g_assert(self  != NULL);
	sig = extension_details(self, extension, &listp)
	       	? mafw_registry_signals[SOURCE_REMOVED]
		: mafw_registry_signals[RENDERER_REMOVED];
	g_assert(find_extension(self, *listp,
				mafw_extension_get_uuid(extension)));
	remove_extension_link(self,
			      g_list_find(*listp, extension),
			      listp, sig);
}

/**
 * mafw_registry_get_extension_by_uuid:
 * @self: a #MafwRegistry object.
 * @uuid: the UUID to be found.
 *
 * Gets a specific extension by its UUID.  If @uuid is %NULL returns %NULL.
 *
 * Returns: a #MafwExtension object, if found or %NULL.
 */
MafwExtension *mafw_registry_get_extension_by_uuid(MafwRegistry *self,
						     const gchar *uuid)
{
	MafwExtension *extension;

	g_return_val_if_fail(uuid != NULL, NULL);
	if (!strcmp(uuid, MAFW_URI_SOURCE_UUID))
		return MAFW_EXTENSION(mafw_get_uri_source());
	extension = find_extension(self, self->priv->sources, uuid);
	if (!extension)
		extension = find_extension(self, self->priv->renderers, uuid);
	return extension;
}
/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
