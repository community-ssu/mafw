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

#ifndef MOCKSOURCE_H
#define MOCKSOURCE_H

#include <glib-object.h>
#include <libmafw/mafw.h>
#include <libmafw/mafw-source.h>
#include <dbus/dbus.h>


typedef struct {
	MafwSourceClass parent;
} MockedSourceClass;

typedef struct {
	MafwSource parent;

	int browse_called, cancel_browse_called, get_metadata_called,
		get_metadatas_called, set_metadata_called, create_object_called,
		destroy_object_called;

	GMainLoop *mainloop;
	gboolean dont_quit;
	guint repeat_browse;
	gboolean dont_send_last;
	gboolean activate_state;
} MockedSource;

extern GType mocked_source_get_type(void);
#define MOCKED_SOURCE(o)						\
	(G_TYPE_CHECK_INSTANCE_CAST((o),				\
				    mocked_source_get_type(),		\
				    MockedSource))

extern gpointer mocked_source_new(const gchar *name, const gchar *uuid,
				  GMainLoop *mainloop);

DBusMessage *append_browse_res(DBusMessage *replmsg,
				DBusMessageIter *iter_msg,
				DBusMessageIter *iter_array,
				guint browse_id,
				gint remaining_count, guint index,
				const gchar *object_id,
				GHashTable *metadata,
				const gchar *domain_str,
				guint errcode,
				const gchar *err_msg);

#define METADATAS_ERROR_MSG "TESTmsg"

DBusMessage *mdatas_repl(DBusMessage *req, const gchar **objlist,
					GHashTable *metadata,
					gboolean add_error);
#endif
