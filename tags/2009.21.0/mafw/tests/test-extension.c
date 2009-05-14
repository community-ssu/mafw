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

#include <glib.h>
#include <string.h>

#include "libmafw/mafw-registry.h"
#include "nopsource.h"

static MafwRegistry *Reg = NULL;

static gboolean test_init(MafwRegistry *registry, GError **error)
{
	gpointer extension;

	Reg = registry;
	extension = g_object_new(nop_source_get_type(),
			    "plugin", "test-extension",
			    "uuid", "ext-uuid",
			    "name", "ext-name",
			    NULL);
	mafw_registry_add_extension(Reg, extension);
	return TRUE;
}

static void test_deinit(GError **error)
{
	MafwExtension *extension;

	extension = MAFW_EXTENSION(mafw_registry_get_extension_by_uuid(Reg, "ext-uuid"));
	mafw_registry_remove_extension(Reg, extension);
}

MafwPluginDescriptor test_extension_plugin_description =
{
	{ .name = "Test extension" },
	.initialize = test_init,
	.deinitialize = test_deinit,
};
