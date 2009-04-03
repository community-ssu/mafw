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

/* Extends `check' to make easier to write cleaner tests. */
#ifndef CHECKMORE_H
#define CHECKMORE_H

/* Include files */
#include <glib.h>
#include <check.h>

/* Macros */
/**
 * Use this macro when expect the failure of a g_assert()
 * during the execution of $expr.  This macro makes sure
 * no error messages are printed because of this failure.
 * It only returns if `check' is configured not to fork()
 * and the expected assertion fails to fail.  @expr is
 * evaluated only once, and it can be a complete statement
 * or even a block.
 */
#define expect_assert(expr)			\
do {						\
	CheckMore_expect_assert = TRUE;		\
	expr;					\
	/* If `check' expects a test to be terminated by a signal  */ \
	/* it *will not* print fail() messages, so we need do it   */ \
	/* manually.  We can't use g_error() though, because that  */ \
	/* abort()s the process, satisfying `check', getting it to */ \
	/* think the test was successful after all, because it was */ \
	/* aborted by SIGABRT.                                     */ \
	g_log(NULL, G_LOG_LEVEL_CRITICAL,	\
		"Assertion failure expected");	\
	fail   ("Assertion failure expected");	\
} while (0)

/**
 * Similar to #expect_assert() in spirit, except that it
 * expects an error caused by g_return_if_fail() during
 * the execution of @expr.
 */
#define expect_ignore(expr)			\
do {						\
	CheckMore_expect_fallback = 1;		\
	expr;					\
	fail_if(CheckMore_expect_fallback == 1,	\
		"Fallback expected");		\
	CheckMore_expect_fallback = 0;		\
} while (0)

/**
 * Similar to #expect_ignore(), expect that the return
 * value of @expr is checked against @val for equivalence.
 * This is useful to test g_return_val_if_fail() returns
 * what is expected down in the invocation chain of @expr.
 * While @expr is evaluated only once, this time it must
 * be a single expression.
 */
#define expect_fallback(expr, val)		\
	expect_ignore(fail_if((expr) != (val)))

/* Global variables */
/**
 * @CheckMore_expect_assert:	if %TRUE the next message about an assertion
 *				failure will be suppressed (but the program
 *				is still aborted)
 * @CheckMore_expect_fallback:	if > 0 suppress GLib critical errors caused
 *				by g_return_if_fail()
 *
 * You may use these variables directly, but the #expect_*() macros are
 * provided for the most common cases.
 */
extern gboolean	CheckMore_expect_assert;
extern guint	CheckMore_expect_fallback;

/* Function prototypes */
extern void	checkmore_set_absolute_env(const gchar *key,
					   const gchar *fname);
extern void	checkmore_redirect(char const *fname);
extern void	checkmore_ignore(gchar const *pat);

extern TCase   *checkmore_add_tcase(Suite *suite, char const *name, TFun fun);
extern void	checkmore_add_aborting_test(TCase *tc, TFun fun);
extern int	checkmore_run(SRunner *runner, gboolean nofork);

extern void     checkmore_unfakeroot(void);
extern void	checkmore_wants_dbus(void);

extern void	checkmore_start(const gchar *path, gint ok_code,
				const gchar *const *args);
extern void	checkmore_stop(void);
extern gboolean	checkmore_child_died(int status, const gchar *who,
				     int ok_code);

extern void     checkmore_spin_loop(gint time);
extern void     checkmore_stop_loop(void);

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
#endif /* ! CHECKMORE_H */
