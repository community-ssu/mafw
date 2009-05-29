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
#include <glib.h>
#include "mafw-log.h"

#undef  G_LOG_DOMAIN
#define G_LOG_DOMAIN		"mafw-log"

/**
 * SECTION: mafwlog
 * @short_description: Logging in MAFW
 *
 * Users of MAFW can enable the display of log and informational messages
 * selectively in run-time.  To do so you need to invoke mafw_log_init(),
 * which takes a string describing what levels of messages to log; others
 * are all disabled.  You can override this compile-time specification by
 * setting the $MAFW_LOG environment variable.
 *
 * The specification string is like
 * "[domain]:level[,[domain]:level]*", where `domain' is the
 * @log_domain argument of #g_log, and `level' is either "ERROR",
 * "CRITICAL", "WARNING", "MESSAGE", "INFO", "DEBUG", "ALL", "PRINT"
 * or "-" (lettercase is not taken into account) telling the minimum
 * required urgency of messages to be logged (ie. don't log less
 * important messages).
 *
 * "-" means don't log anything from that domain.  If `domain' is "default"
 * `level' applies to all messages for which #G_LOG_DOMAIN wasn't defined.
 * Otherwise if the `domain' part is empty `level' will apply to domains
 * not specifically mentioned in the string.
 *
 * All possible values of `level' except "ALL" and "PRINT" makes g_print()
 * silent.  If `level' is "ALL" the domain is effectively ignored by the
 * filtering mechanism (ie. it's processing is left as it was).  "PRINT" is
 * like "-", but leaves g_print() enabled.
 *
 * Some examples:
 *
 * <informalexample><programlisting>
 * # silence everything
 * MAFW_LOG=":-" ./app
 * # silence 'foo'
 * MAFW_LOG="foo:-" ./app
 * # only mafw messages less important than message
 * MAFW_LOG="mafw:warning" ./app
 * # all from mafw-dbus
 * MAFW_LOG="mafw-dbus:debug" ./app
 * </programlisting></informalexample>
 */

/* Digital blackhole. */
static void log_devnull(void)
{
	/* NOP */
}

/* Log $message if it should be according to $logged_levels. */
static void log_worsethan(gchar const *domain, GLogLevelFlags level,
			  gchar const *message, GLogLevelFlags logged_levels)
{
	/* Print messages only worse or equal than logged_levels. */
	if (level & logged_levels)
		g_log_default_handler(domain, level, message, NULL);
}

/*
 * Returns a #GLogLevelFlags mask, representing log levels equal or
 * worse (towards critical) than the log level encoded by @lstr.
 */
static GLogLevelFlags levels_worse_than(gchar const *lstr)
{
	GLogLevelFlags levels;

	g_assert(lstr != NULL);

	levels = 0;
	if (!g_ascii_strcasecmp(lstr, "-"))
		/* `-' is worse than anything. */
		return levels;
	levels |= G_LOG_LEVEL_ERROR;
	if (!g_ascii_strcasecmp(lstr, "ERROR")) 
		return levels;
	levels |= G_LOG_LEVEL_CRITICAL;
	if (!g_ascii_strcasecmp(lstr, "CRITICAL")) 
		return levels;
	levels |= G_LOG_LEVEL_WARNING;
	if (!g_ascii_strcasecmp(lstr, "WARNING")) 
		return levels;
	levels |= G_LOG_LEVEL_MESSAGE;
	if (!g_ascii_strcasecmp(lstr, "MESSAGE")) 
		return levels;
	levels |= G_LOG_LEVEL_INFO;
	if (!g_ascii_strcasecmp(lstr, "INFO")) 
		return levels;
	levels |= G_LOG_LEVEL_DEBUG;
	if (!g_ascii_strcasecmp(lstr, "DEBUG")) 
		return levels;

	/* Unknown level. */
	g_warning("%s: unknown log level", lstr);
	return levels;
}

/**
 * mafw_log_init:
 * @doms: the doms. If the $MAFW_LOG environment variable is defined,
 * its value overrides this parameter
 *
 * Disables the display of all log messages except those matching @doms,
 * whose format is described in the section introduction.  If @doms is
 * %NULL, a sane default is used.  If it's empty no changes will be made
 * to logging.  
 *
 * <note><para> This function works as expected only for the first
 *   time invoked and only if no log handlers were set before because
 *   there's no way clearing out log handlers in general.</para></note>
 */
void mafw_log_init(gchar const *udoms)
{
	int leave_gprint;
	const gchar *doms;
	gchar **pairs, **pair;

	if (!(doms = g_getenv("MAFW_LOG")))
		/* Environment overrides. */
		doms = udoms;
	if (!doms)
		/* Use a sane default. */
		doms = ":warning";
	else if (!doms[0])
		/* Don't touch anything. */
		return;

	/* Parse $doms. */
	leave_gprint = -1;
	pairs = g_strsplit(doms, ",", 0);
	for (pair = pairs; *pair; pair++) {
		gchar *domain, *level;
		GLogLevelFlags levels;

		/* Split `domain:level' pair. */
		domain = *pair;
		if ((level = strchr(*pair, ':')) != NULL)
			*level++ = '\0';
		if (!level || !g_strcasecmp(level, "ALL")) {
			leave_gprint = 1;
			continue;
		} else if (!g_strcasecmp(level, "PRINT")) {
			leave_gprint = 1;
			level = "-";
		}
		levels = levels_worse_than(level);

		if (!domain[0]) {
			/* Empty domain means default handler. */
			g_log_set_default_handler((GLogFunc)log_worsethan,
						  GUINT_TO_POINTER(levels));
		} else {
			/* `default' means the default domain
			 * (G_LOG_DOMAIN not set by the programmer). */
			if (!g_ascii_strcasecmp(domain, "default"))
				domain = NULL;

			/* If levels == 0 we have to disable all levels
			 * in that domain.  Otherwise set the handler
			 * for all levels and let it decide what to log. */
			g_log_set_handler(domain,
					  G_LOG_LEVEL_MASK
					  | G_LOG_FLAG_RECURSION
					  | G_LOG_FLAG_FATAL,
					  levels ? (GLogFunc)log_worsethan
					  	 : (GLogFunc)log_devnull,
					  GUINT_TO_POINTER(levels));
		}

		/* Unless we're told specifically not to, disable g_print(). */
		if (leave_gprint < 0)
			leave_gprint = 0;
	}
	g_strfreev(pairs);

	if (!leave_gprint)
		g_set_print_handler((GPrintFunc)log_devnull);
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
