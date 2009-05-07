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

/**
 * SECTION:mafwcallbas
 * @short_description: put together a function call and invoke it later
 *
 * Primarily these functions help you in the construction of deferred
 * function calls, ie. when you at some point know how and what you
 * want to invoke (such as a user-supplied callback), but you need to
 * do it later in execution time.  Then you need to create a
 * `callbas', an object binding a C function (callback) to some C
 * arguments you want it to be called with.  This is done with
 * mafw_callbas_new().  You can invoke a callbas with
 * mafw_callbas_invoke() any number of times before releasing it with
 * mafw_callbas_free().  This happens automatically if you
 * mafw_callbas_defer() the activisation until idle time.
 *
 * mafw_callbas_argv2gval() is also useful as a standalone function
 * when you need to create a #GValue out of some arbitrary C values.
 */

/* Include files */
#include <stdarg.h>
#include <string.h>

#include "mafw-callbas.h"

/* Function prototypes */
static gboolean invoke(MafwCallbas *cbas);

/* Interface functions */
/**
 * mafw_callbas_argv2gval:
 * @value: a #GValue to place the result
 * @type: the #GType of the value
 * @args: arguments list
 *
 * Decodes the next argument from @args having @type and encodes it
 * into @value, a reset #GValue.  Since @args is passed as a pointer
 * it is modified, so subsequent calls to this function will decode
 * subsequent varadic arguments.
 *
 * In general the value is copied, except for hash tables.
 */
void mafw_callbas_argv2gval(GValue *value, GType type, va_list *args)
{
	g_value_init(value, type);
	switch (type) {
	case G_TYPE_BOOLEAN:
		g_value_set_boolean(value, va_arg(*args, gboolean));
		break;
	case G_TYPE_INT:
		g_value_set_int(value, va_arg(*args, gint));
		break;
	case G_TYPE_UINT:
		g_value_set_uint(value, va_arg(*args, guint));
		break;
	case G_TYPE_LONG:
		g_value_set_long(value, va_arg(*args, glong));
		break;
	case G_TYPE_ULONG:
		g_value_set_ulong(value, va_arg(*args, gulong));
		break;
	case G_TYPE_INT64:
		g_value_set_int64(value, va_arg(*args, gint64));
		break;
	case G_TYPE_UINT64:
		g_value_set_uint64(value, va_arg(*args, guint64));
		break;
	case G_TYPE_DOUBLE:
		g_value_set_double(value, va_arg(*args, gdouble));
		break;
	case G_TYPE_STRING:
		g_value_set_string(value, va_arg(*args, gchar *));
		break;
	case G_TYPE_POINTER:
		g_value_set_pointer(value, va_arg(*args, gchar *));
		break;
	default:
		if (type == G_TYPE_HASH_TABLE) {
			g_value_take_boxed(value, va_arg(*args, GHashTable *));
			break;
		}
		g_assert_not_reached();
	} /* switch */
}

/**
 * mafw_callbas_new:
 * @cb: a callback
 * @mars: the closure
 * @self: self
 * @Varargs: parameters
 *
 * Creates a callbas with which you can eventually invoke @cb.  @self
 * is passed as the first argument to @cb.  The rest of the arguments
 * are given in the varadic list as GType0--value pairs, closed with
 * MAFW_CBAS_END.  You are advised to use the MAFW_CBAS_*() macros
 * for that.  @mars is the marshaller function created by
 * `glib-marshal'; it should NOT account for the first @self
 * parameter.
 *
 * Returns: a new instance of #MafwCallbas with the constructed with
 * the given parameters.
 */
MafwCallbas *mafw_callbas_new(GCallback cb, GClosureMarshal mars,
			      gpointer self, ...)
{
	GType type;
	va_list cargs;
	GArray *gvargs;
	MafwCallbas *cbas;
	GValue empty, *arg;

	/* Construct the callbas and the GClosure. */
	cbas = g_new(MafwCallbas, 1);
	cbas->closure = g_cclosure_new(cb, NULL, NULL);
	g_closure_set_marshal(cbas->closure, mars);

	/* Start constructing the GArray of GValue:s used to
	 * hold the to-be arguments of $cb. */
	memset(&empty, 0, sizeof(empty));
	cbas->gvargs = gvargs = g_array_sized_new(FALSE, FALSE,
						  sizeof(GValue), 1);

	/* First add $self. */
	g_array_append_val(gvargs, empty);
	arg = &g_array_index(gvargs, GValue, 0);
	g_value_init(arg, G_TYPE_POINTER);
	g_value_set_pointer(arg, self);

	/* Append the rest of arguments to $gvargs. */
	va_start(cargs, self);
	while ((type = va_arg(cargs, GType)) != MAFW_CBAS_END) {
		g_array_append_val(gvargs, empty);
		arg = &g_array_index(gvargs, GValue, gvargs->len-1);
		mafw_callbas_argv2gval(arg, type, &cargs);
	}
	va_end(cargs);

	return cbas;
}

/** 
 * mafw_callbas_free:
 * @cbas: a #MafwCallbas instance
 *
 * Releases a callbas and everything therein.
 */
void mafw_callbas_free(MafwCallbas *cbas)
{
	guint i;

	g_closure_unref(cbas->closure);
	for (i = 0; i < cbas->gvargs->len; i++)
		g_value_unset(&g_array_index(cbas->gvargs, GValue, i));
	g_array_free(cbas->gvargs, TRUE);
	g_free(cbas);
}

/**
 * mafw_callbas_defer:
 * @cbas: a #MafwCallbas instance
 *
 * Arranges for the invocation of @cbas when you enter (return to)
 * the main loop and returns the GLib source ID with which you can
 * cancel its activation.
 *
 * Returns: the event id
 */
gint mafw_callbas_defer(MafwCallbas *cbas)
{
	return g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
			       (GSourceFunc)invoke, cbas,
			       (GDestroyNotify)mafw_callbas_free);
}

/**
 * mafw_callbas_invoke:
 * @cbas: a #MafwCallbas instance
 *
 * Invokes the C function represented by @cbas, ignoring its return
 * value.
 */
void mafw_callbas_invoke(MafwCallbas *cbas)
{
	g_closure_invoke(cbas->closure, NULL,
			 cbas->gvargs->len,
			 &g_array_index(cbas->gvargs, GValue, 0),
			 NULL);
}

/* Private functions */
/* g_idle_add() handler to invoke $cbas. */
gboolean invoke(MafwCallbas *cbas)
{
	mafw_callbas_invoke(cbas);
	return FALSE;
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
