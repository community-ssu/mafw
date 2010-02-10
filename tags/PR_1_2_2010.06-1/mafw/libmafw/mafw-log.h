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

#ifndef __MAFW_LOG_H__
#define __MAFW_LOG_H__

#include <glib.h>

/**
 * g_info():
 * @...: List of parameters.
 *
 * Convenience macro to log at INFO level. It receives a format
 * string, followed by parameters to insert into the format string (as
 * with printf())
 */
#ifdef G_HAVE_ISO_VARARGS
#define g_info(...) g_log(G_LOG_DOMAIN, \
			  G_LOG_LEVEL_INFO, \
			  __VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define g_info(format...) g_log(G_LOG_DOMAIN, \
				G_LOG_LEVEL_INFO, \
				format)
#else
#define g_info(...)
#endif

/* Prototypes. */
G_BEGIN_DECLS
extern void mafw_log_init(gchar const *doms);
G_END_DECLS

#endif
