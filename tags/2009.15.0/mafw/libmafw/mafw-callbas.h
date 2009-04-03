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

#ifndef __MAFW_CALLBAS_H__
#define __MAFW_CALLBAS_H__

#include <glib.h>
#include <glib-object.h>

/* Use these macros when you're specifying the callback argument list
 * in mafw_callbas_new(). */
/**
 * MAFW_CBAS_END:
 *
 * End of parameters
 */
#define MAFW_CBAS_END		G_TYPE_INVALID
/**
 * MAFW_CBAS_BOOLEAN:
 * @val: value
 *
 *  boolean parameter.
 */
#define MAFW_CBAS_BOOLEAN(val)	G_TYPE_BOOLEAN,		(val)
/**
 * MAFW_CBAS_INT:
 * @val: value
 *
 * An integer parameter.
 */
#define MAFW_CBAS_INT(val)	G_TYPE_INT,		(val)
/**
 * MAFW_CBAS_UINT:
 * @val: value
 *
 * An unsigned integer parameter.
 */
#define MAFW_CBAS_UINT(val)	G_TYPE_UINT,		(val)
/**
 * MAFW_CBAS_LONG:
 * @val: value
 *
 * A long parameter.
 */
#define MAFW_CBAS_LONG(val)	G_TYPE_LONG,		(val)
/**
 * MAFW_CBAS_ULONG:
 * @val: value
 *
 * An unsigned long parameter.
 */
#define MAFW_CBAS_ULONG(val)	G_TYPE_ULONG,		(val)
/**
 * MAFW_CBAS_INT64:
 * @val: value
 *
 * An 64bit integer parameter.
 */
#define MAFW_CBAS_INT64(val)	G_TYPE_INT64,		(val)
/**
 * MAFW_CBAS_UINT64:
 * @val: value
 *
 * An unsigned 64bits integer parameter.
 */
#define MAFW_CBAS_UINT64(val)	G_TYPE_UINT64,		(val)
/**
 * MAFW_CBAS_DOUBLE:
 * @val: value
 *
 * A double floating pointer parameter.
 */
#define MAFW_CBAS_DOUBLE(val)	G_TYPE_DOUBLE,		(val)
/**
 * MAFW_CBAS_STRING:
 * @val: value
 *
 * A string parameter.
 */
#define MAFW_CBAS_STRING(val)	G_TYPE_STRING,		(val)
/**
 * MAFW_CBAS_HASH:
 * @val: value
 *
 * A hash table parameter.
 */
#define MAFW_CBAS_HASH(val)	G_TYPE_HASH_TABLE,	(val)
/**
 * MAFW_CBAS_POINTER:
 * @val: value
 *
 * A pointer parameter.
 */
#define MAFW_CBAS_POINTER(val)	G_TYPE_POINTER, 	(val)
/**
 * MAFW_CBAS_NULL:
 * @val: value
 *
 * A %NULL pointer parameter.
 */
#define MAFW_CBAS_NULL		MAFW_CBAS_POINTER(NULL)

/**
 * MafwCallbas:
 * @closure: function's closure
 * @gvargs: is an array of #GValue:s holding the actual
 * arguments of @closure.
 *
 * This structure is meant to represent a function call
*/
typedef struct {
	GClosure *closure;
	GArray *gvargs;
} MafwCallbas;

G_BEGIN_DECLS
extern void mafw_callbas_argv2gval(GValue *value, GType type, va_list *args);
extern MafwCallbas *mafw_callbas_new(GCallback cb, GClosureMarshal mars,
				     gpointer self, ...);
extern void mafw_callbas_free(MafwCallbas *cbas);
extern void mafw_callbas_invoke(MafwCallbas *cbas);
extern gint mafw_callbas_defer(MafwCallbas *cbas);
G_END_DECLS

#endif /* ! __MAFW_CALLBAS_H__ */
