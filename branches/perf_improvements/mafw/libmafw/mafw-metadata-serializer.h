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

#ifndef __MAFW_METADATA_DBUS_H__
#define __MAFW_METADATA_DBUS_H__

#include <glib.h>

G_BEGIN_DECLS
extern GByteArray *mafw_metadata_freeze_bary(GHashTable *md);
extern GHashTable *mafw_metadata_thaw_bary(GByteArray *bary);

extern gchar *mafw_metadata_freeze(GHashTable *md, gsize *sstreamp);
extern GHashTable *mafw_metadata_thaw(const gchar *stream, gsize sstream);

extern void mafw_metadata_val_freeze_bary(GByteArray *bary, gpointer val);
extern gpointer mafw_metadata_val_thaw_bary(GByteArray *bary, gsize *i);

extern gchar *mafw_metadata_val_freeze(gpointer val, gsize *sstreamp);
G_END_DECLS

#endif
/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
