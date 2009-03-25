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

#ifndef __MAFW_PROPERTY_H__
#define __MAFW_PROPERTY_H__

#include <libmafw/mafw-extension.h>

G_BEGIN_DECLS

/**
 * SECTION: mafwextension
 * @short_description: Extension runtime properties.
 *
 * Common definitions for runtime properties of MAFW renderer and sources.  See
 * functions mafw_extension_add_property(), mafw_extension_get_property(),
 * mafw_extension_set_property() etc. for details.  Use MAFW_EXTENSION_SUPPORTS_* to
 * declare support for the given property.
 */

/**
 * MAFW_PROPERTY_RENDERER_VOLUME:
 *
 * Property for adjusting renderer volume.
 * Type: #G_TYPE_UINT
 * Valid values: 0 .. 99
 */
#define MAFW_PROPERTY_RENDERER_VOLUME "volume"
/**
 * MAFW_EXTENSION_SUPPORTS_VOLUME:
 * @self: #MafwExtension instance to add the property on
 *
 * Adds property for selecting volume
 */
#define MAFW_EXTENSION_SUPPORTS_VOLUME(self)					\
	mafw_extension_add_property(MAFW_EXTENSION(self),				\
			       MAFW_PROPERTY_RENDERER_VOLUME, G_TYPE_UINT)

/**
 * MAFW_PROPERTY_RENDERER_MUTE:
 *
 * Property to (un)mute renderer.
 * Type: #G_TYPE_BOOLEAN
 * Valid values: %TRUE - audio muted, %FALSE - audio not muted.
 */
#define MAFW_PROPERTY_RENDERER_MUTE "mute"
/**
 * MAFW_EXTENSION_SUPPORTS_MUTE:
 * @self: #MafwExtension instance to add the property on
 *
 * Adds property for selecting mute
 */
#define MAFW_EXTENSION_SUPPORTS_MUTE(self)					\
	mafw_extension_add_property(MAFW_EXTENSION(self),				\
			       MAFW_PROPERTY_RENDERER_MUTE, G_TYPE_BOOLEAN)

/**
 * MAFW_PROPERTY_RENDERER_XID:
 *
 * Property to give a renderer an X window to draw video/image content on.
 * Type: #G_TYPE_ULONG
 * Valid values: See XID definition in X11/Xdefs.h
 */
#define MAFW_PROPERTY_RENDERER_XID "xid"
/**
 * MAFW_EXTENSION_SUPPORTS_XID:
 * @self: #MafwExtension instance to add the property on
 *
 * Adds property for selecting a xid
 */
#define MAFW_EXTENSION_SUPPORTS_XID(self)					\
	mafw_extension_add_property(MAFW_EXTENSION(self),				\
			       MAFW_PROPERTY_RENDERER_XID, G_TYPE_ULONG)

/**
 * MAFW_PROPERTY_RENDERER_ERROR_POLICY:
 *
 * Property for selecting an error policy.
 * Type: #G_TYPE_UINT
 * Valid values: Any defined in #MafwRendererErrorPolicy
 */
#define MAFW_PROPERTY_RENDERER_ERROR_POLICY "error-policy"
/**
 * MAFW_EXTENSION_SUPPORTS_ERROR_POLICY:
 * @self: #MafwExtension instance to add the property on
 *
 * Adds property for selecting an error policy.
 */
#define MAFW_EXTENSION_SUPPORTS_ERROR_POLICY(self)				\
	mafw_extension_add_property(MAFW_EXTENSION(self),				\
			       MAFW_PROPERTY_RENDERER_ERROR_POLICY, G_TYPE_UINT)

/**
 * MAFW_PROPERTY_RENDERER_COLORKEY:
 *
 * Read-only property corresponding to the Xv color key.  If its value is not
 * yet known, -1 is returned.  Interested parties are suggested to connect to
 * the MafwExtension::property-changed signal to get notified when it's
 * available.
 * Type: #G_TYPE_INT
 */
#define MAFW_PROPERTY_RENDERER_COLORKEY "colorkey"
#define MAFW_EXTENSION_SUPPORTS_COLORKEY(self)				\
	mafw_extension_add_property(MAFW_EXTENSION(self),		\
				     MAFW_PROPERTY_RENDERER_COLORKEY,	\
				     G_TYPE_INT)

/**
 * MAFW_PROPERTY_RENDERER_AUTOPAINT:
 *
 * If %TRUE, the renderer automatically paints the window passed for video
 * playback with the Xv color key.
 * Type: #G_TYPE_BOOLEAN
 */
#define MAFW_PROPERTY_RENDERER_AUTOPAINT "autopaint"
#define MAFW_EXTENSION_SUPPORTS_AUTOPAINT(self)			\
	mafw_extension_add_property(MAFW_EXTENSION(self),		\
				     MAFW_PROPERTY_RENDERER_AUTOPAINT,	\
				     G_TYPE_BOOLEAN)

G_END_DECLS

#endif
