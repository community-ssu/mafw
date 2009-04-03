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

#ifndef __MAFW_FILTER_H__
#define __MAFW_FILTER_H__

#include <glib.h>

/**
 * MafwFilterType:
 * @MAFW_F_INVALID: zero value, not used.
 * @mafw_f_and:    a conjunction.
 * @mafw_f_or:     a disjunction.
 * @mafw_f_not:    a negation.
 * @MAFW_F_COMPLEX: separator, not used.
 * @mafw_f_exists: existence.
 * @mafw_f_eq:     equality.
 * @mafw_f_lt:     less-than.
 * @mafw_f_gt:     greater-than.
 * @mafw_f_approx: approximate matching.
 * @MAFW_F_LAST: separator, not used.
 *
 * Represents the different kinds of filters used in MAFW.
 */
typedef enum {
	MAFW_F_INVALID = 0,
	/* Aggregates. */
	mafw_f_and,
	mafw_f_or,
	mafw_f_not,
	/* Divider between aggregate and simple filters. */
	MAFW_F_COMPLEX,
	/* Simple ones. */
	mafw_f_exists,
	mafw_f_eq,
	mafw_f_lt,
	mafw_f_gt,
	mafw_f_approx,
	/* last element */
	MAFW_F_LAST,
} MafwFilterType;

/**
 * MAFW_FILTER_IS_COMPLEX:
 * @f: a filter instance.
 *
 * Checks if a filter is complex
 *
 * Returns: %TRUE if @f is an aggregate filter.
 */
#define MAFW_FILTER_IS_COMPLEX(f) \
	((f)->type > MAFW_F_INVALID && (f)->type < MAFW_F_COMPLEX)
/**
 * MAFW_FILTER_IS_SIMPLE:
 * @f: a filter instance.
 *
 * Checks if a filter is simple
 *
 * Returns: %TRUE if @f is a simple filter.
 */
#define MAFW_FILTER_IS_SIMPLE(f) \
	((f)->type > MAFW_F_COMPLEX && (f)->type < MAFW_F_LAST)
/**
 * MAFW_FILTER_IS_VALID:
 * @f: a filter instance.
 *
 * Checks if a filter is valid
 *
 * Returns: %TRUE if @f has a valid filter type.
 */
#define MAFW_FILTER_IS_VALID(f) \
	((f)->type > MAFW_F_INVALID && (f)->type < MAFW_F_LAST && \
	 (f)->type != MAFW_F_COMPLEX)

/**
 * MafwFilter:
 * @type:  the type of the filter (enum #filter_type).
 * @parts: if the filter is aggregate;  it's an array of pointers to
 *         child filter structures, terminated by %NULL.
 * @key:   if the filter is simple; the attribute to match.
 * @value: if the filter is simple; the value to match with.  If the
 *         type is #mafw_f_exists, it is an empty string.
 *
 * Programmatic representation of a filter.
 */
typedef struct MafwFilter {
	MafwFilterType type;
	union {
		struct MafwFilter **parts;
		struct {
			char *key;
			char *value;
		};
	};
} MafwFilter;

/* Convenience macros for constructing filters. */

/**
 * MAFW_FILTER_AND:
 * @varargs: list of elements
 *
 * Constructs a filter representing the conjunction of its arguments.
 */
#define MAFW_FILTER_AND(...) \
	mafw_filter_new(mafw_f_and, ##__VA_ARGS__, NULL)
/**
 * MAFW_FILTER_OR:
 * @...: list of elements
 *
 * Constructs a filter representing disjunction of its arguments.
 */
#define MAFW_FILTER_OR(...)  mafw_filter_new(mafw_f_or, ##__VA_ARGS__, NULL)
/**
 * MAFW_FILTER_NOT:
 * @...: list of elements
 *
 * Constructs a filter representing negation of its argument.
 */
#define MAFW_FILTER_NOT(...) mafw_filter_new(mafw_f_not, ##__VA_ARGS__, NULL)

/**
 * MAFW_FILTER_EQ:
 * @k: metadata key.
 * @v: the value.
 *
 * Constructs a filter representing `value of @k equals to @v'.
 */
#define MAFW_FILTER_EQ(k, v)     mafw_filter_new(mafw_f_eq, k, v)
/**
 * MAFW_FILTER_LT:
 * @k: metadata key.
 * @v: the value.
 *
 * Constructs a filter representing `value of @k is less than @v'.
 */
#define MAFW_FILTER_LT(k, v)     mafw_filter_new(mafw_f_lt, k, v)
/**
 * MAFW_FILTER_GT:
 * @k: metadata key.
 * @v: the value.
 *
 * Constructs a filter representing `value of @k is greater than @v'.
 */
#define MAFW_FILTER_GT(k, v)     mafw_filter_new(mafw_f_gt, k, v)
/**
 * MAFW_FILTER_APPROX:
 * @k: metadata key.
 * @v: the value.
 *
 * Constructs a filter representing `value of @k approximately matches @v'.
 */
#define MAFW_FILTER_APPROX(k, v) mafw_filter_new(mafw_f_approx, k, v)
/**
 * MAFW_FILTER_EXISTS:
 * @k: metadata key.
 *
 * Constructs a filter representing `@k is not empty'.
 */
#define MAFW_FILTER_EXISTS(k)    mafw_filter_new(mafw_f_exists, k, NULL)

/* API. */
G_BEGIN_DECLS

extern char *mafw_filter_quote(char const *str);
extern char const *mafw_filter_unquote_char(char const *str, char *cp);
extern char *mafw_filter_unquote(char const *str);

extern MafwFilter *mafw_filter_new(MafwFilterType type, ...);
extern void mafw_filter_free(MafwFilter *filter);

extern MafwFilter *mafw_filter_add_children_n(MafwFilter *filter, ...);

/**
 * mafw_filter_add_children:
 * @f: the filter to add to.
 * @...: list of children
 *
 * Adds children nodes to the given filter.  Does nothing if the
 * filter is not an aggregate filter (AND, OR).  Use the
 * mafw_filter_add_children() convenience macro that adds the
 * terminating %NULL automatically.
 *
 * Returns: the modified filter.
 */
#define mafw_filter_add_children(f, ...) \
	mafw_filter_add_children_n(f, ##__VA_ARGS__, NULL);

extern MafwFilter *mafw_filter_parse(char const *filter);
extern gchar *mafw_filter_to_string(const MafwFilter *filter);
extern MafwFilter *mafw_filter_copy(const MafwFilter *filter);

G_END_DECLS
#endif
