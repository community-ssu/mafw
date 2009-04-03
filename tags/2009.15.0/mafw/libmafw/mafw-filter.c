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

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include <glib.h>

#include "mafw-filter.h"

/**
 * SECTION: mafwfilter
 * @short_description: filters for browsing
 * @see_also: #MafwSource
 *
 * #MafwFilter is a tree of filter expressions, used by sources to
 * parse the @filter argument in mafw_source_browse().  It can be
 * either built by parsing a string by mafw_filter_parse(), or
 * constructed programmatically with mafw_filter_new() or the
 * appropriate convenience macros.
 *
 * Filter strings
 *
 * The string representation of a filter is a slightly modified
 * variant of the LDAP search string described in RFC 4515.
 * Modifications are:
 *
 * <orderedlist>
 * <listitem><para>
 * `&gt;=' and `&lt;=' are replaced with `&gt;' and `&lt;' (only one
 * character), and they mean GREATER THAN and LESS THAN.
 * </para></listitem>
 * <listitem><para>
 * `~=' is replaced with `~' for approximate matching.
 * </para></listitem>
 * <listitem><para>
 * the `exists' operator (`=*') is represented by `?'.
 * </para></listitem>
 * <listitem><para>
 * extensible matching rules are not supported.
 * </para></listitem>
 * </orderedlist>
 *
 * LDAPv3 escaping of `(', `)', `*' is used: they are substituted \XX,
 * where XX is a 2-digit hexadecimal number.  See the
 * mafw_filter_quote() and mafw_filter_unquote() functions.
 * Attributes may consist of characters from [_-a-zA-Z0-9].
 *
 * Example
 *
 * `(&(artist=belga)(year&gt;2000))' -- matches artist named `belga' with
 * `year' after 2000.
 *
 * Programmatic construction of the same filter looks like this:
 *
 * <informalexample><programlisting>
 * #MAFW_FILTER_AND(#MAFW_FILTER_EQ("artist", "belga"),
 * 	            #MAFW_FILTER_GT("year", "2000"))
 * </programlisting></informalexample>
 */

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "mafw"

/* Finds out the length of a NULL terminated variable argument list
 * starting after _last_arg, and puts it into _len. */
#define va_len(_len, _last_arg)						\
	do {								\
		va_list _ap;						\
		va_start(_ap, _last_arg);				\
		for ((_len) = 0; (va_arg(_ap, void *)); (_len)++);	\
		va_end(_ap);						\
	} while (0)

/**
 * mafw_filter_quote:
 * @str: the string to quote.
 *
 * Quotes @str according to LDAPv3 rules.  The following characters
 * are quoted: '*', '(', ')', '\'.
 *
 * Returns: a newly allocated quoted string.
 */
char *mafw_filter_quote(char const *str)
{
	static const char _hexchars[16] = "0123456789ABCDEF";
	int nsize;
	char const *t;
	char *q, *ret = NULL;

	if (str != NULL) {
		/* Figure out how much space we need: count the number of
		 * characters to quote. */
		for (t = str, nsize = 0; *t; nsize++, t++) {
			switch (*t) {
			case '*': case '(': case ')': case '\\':
				nsize += 2;
			}
		}
		ret = q = malloc(sizeof(char) * nsize + 1);
                if (!ret) {
                        return NULL;
                }
		for (t = str; *t; q++, t++) {
			switch (*t) {
			case '*': case '(':  case ')': case '\\':
				*q++ = '\\';
				*q++ = _hexchars[*t >> 4];
				*q = _hexchars[*t & 0xf];
				break;
			default:
				*q = *t;
				break;
			}
		}
		*q = '\0';
	}
	return ret;
}

/*
 * Parses @inc as a hex digit and ORs its value with *@outc.
 * Returns 0 if @inc is not a hex digit, otherwise returns nonzero.
 */
static gboolean hex2char(char *outc, char inc)
{
	if ('0' <= inc && inc <= '9')
		*outc |= inc - '0';
	else if ('A' <= inc && inc <= 'F')
		*outc |= inc - 'A' + 10;
	else
		return FALSE;
	return TRUE;
}

/**
 * mafw_filter_unquote_char:
 * @str: points to the token to unquote
 * @cp:  points to the char to store the unquoted value
 *
 * Unquotes one token (a character or an escape sequence) of @str.
 * The token must not be an unquoted NIL, so you need to check the
 * end-of-string condition prior to calling this function.  This is
 * because the return value would point out of bounds or be ambiguous
 * otherwise.  This function is intended for sources that need to
 * unescape then reescape in a domain-specific format property values.
 *
 * Returns: the starting position of the next token or %NULL
 * if the current token was found to be ununquotable.
 */
char const *mafw_filter_unquote_char(char const *str, char *cp)
{
	if (*str == '\\') {
		*cp = 0;
		if (!hex2char(cp, *++str))
			return NULL;
		*cp <<= 4;
		if (!hex2char(cp, *++str))
			return NULL;
	} else {
		g_assert(*str != '\0');
		*cp = *str;
	}
	return ++str;
}

/**
 * mafw_filter_unquote:
 * @str: a LDAPv3 quoted string.
 *
 * Unquotes the LDAPv3 quoted string @str.
 *
 * Returns: a newly allocated string containing the unquoted value of @str
 * or %NULL if @str is quoted improperly
 */
char *mafw_filter_unquote(char const *str)
{
	int nsize;
	char *t, *ret = NULL;

	if (str != NULL) {
		/* Calculate the length of the unquoted string. */
		for (nsize = 0, t = (char *)str; *t; t++, nsize++)
			if (*t == '\\')
				t += 2;

		/* Unexpand escape sequences. */
		t = ret = malloc(nsize + 1);
                if (!ret) {
                        return NULL;
                }
		while (*str) {
			if (!(str = mafw_filter_unquote_char(str, t++))) {
				free(ret);
				return NULL;
			}
		}
		*t = '\0';
	}

	return ret;
}

/*
 * Returns the integer representation of a filter encoded by @filterc, or
 * #MAFW_F_INVALID, if unknown.
 */
static inline int char_to_simple(unsigned char filterc)
{
	switch (filterc) {
	case '=': return mafw_f_eq;
	case '<': return mafw_f_lt;
	case '>': return mafw_f_gt;
	case '~': return mafw_f_approx;
	case '?': return mafw_f_exists;
	default:  return MAFW_F_INVALID;
	}
}

/*
 * Similar to char_to_simple(), but only for aggregate (and, or, not) operators.
 */
static inline int char_to_complex(unsigned int filterc)
{
	switch (filterc) {
	case '&': return mafw_f_and;
	case '|': return mafw_f_or;
	case '!': return mafw_f_not;
	default:  return MAFW_F_INVALID;
	}
}

/*
 * Returns the #guchar representation of the a #MafwFilterType
 */
static inline guchar type_to_char(MafwFilterType filter_type)
{
	switch (filter_type) {
	case mafw_f_eq: return '=';
	case mafw_f_lt: return '<';
	case mafw_f_gt: return '>';
	case mafw_f_approx: return '~';
	case mafw_f_exists: return '?';
	case mafw_f_and: return '&';
	case mafw_f_or: return '|';
	case mafw_f_not: return '!';
	default:  return '\0';
	}
}

/*
 * Parse a simple filter expression (non-aggregate).
 */
static char const *parse_simple(char const *filt, MafwFilter **result)
{
	char const *t;
	char *attr;
        char *value;
	int ftype;

	/* NOTE that the initial `(' is already eaten by parse_sexp()
	 * when we get here! */

	/* Extract attribute.
	 * Let's allow [-_a-zA-Z0-9] */
	t = filt;
	while (*t && (isalnum(*t) || *t == '-' || *t == '_')) t++;
	/* If string ended before the key, or would be emtpy, treat it
	 * as an error. */
	if (!*t || t == filt) return NULL;

	attr = strndup(filt, t - filt);
	filt = t;

	/* ... then the operator ... */
	ftype = char_to_simple(*filt);
	if (ftype == MAFW_F_INVALID) {
		free(attr);
		return NULL;
	}
	filt++;

	/* ... and finally the value to match with. */
	t = filt;
	while (*t && *t != ')') t++;
	if (!*t) {
		/* Let's not leak attr if we have syntax error. */
		free(attr);
		return NULL;
	}
        value = strndup(filt, t - filt);

	*result = calloc(sizeof(MafwFilter), 1);
        if (*result == NULL) {
                free(attr);
                free(value);
                return NULL;
        }

	(*result)->type = ftype;
	(*result)->key = mafw_filter_unquote(attr);
	(*result)->value = mafw_filter_unquote(value);

        free(attr);
        free(value);

	/* Skip ending `)'. */
	filt = t + 1;

	return filt;
}

/*
 * Tries to parse a parenthesized expression given in @filt.  Builds a
 * #filter structure of it, for which the location is given in @result.
 *
 * Returns: a pointer after parsed part of input, or %NULL if it contains
 * syntax errors.
 */
static char const *parse_sexp(char const *filt, MafwFilter **result)
{
	int ftype, pnum;

	/* Assure it starts with a `('. */
	if (*filt != '(') return NULL;
	filt++;

	/* Invoke the simple parser if it's not {AND, OR, NOT}. */
	ftype = char_to_complex(*filt);
	if (ftype == MAFW_F_INVALID)
		return parse_simple(filt, result);

	/* Aggregate. */
	filt++;
	*result = calloc(sizeof(MafwFilter), 1);
        if (*result == NULL) {
                return NULL;
        }
	(*result)->type = ftype;

	pnum = 0;
	while (1) {
		MafwFilter *part;

		part = NULL;
		filt = parse_sexp(filt, &part);
		/* NOTE if we return here, ->parts will be NULL! */
		if (!filt) return NULL;
		pnum++;

		(*result)->parts = realloc((*result)->parts,
					   sizeof(MafwFilter *) * (pnum + 1));
                if ((*result)->parts != NULL) {
                        (*result)->parts[pnum - 1] = part;
                        (*result)->parts[pnum] = NULL;
                }

		/* End of string. */
		if (!*filt) break;
		/* Ending `)'. */
		if (*filt == ')') {
			filt++;
			break;
		}
		/* Accept only one sub-expression for NOT.  So, if we
		 * would loop around, it is an error. */
		if (ftype == mafw_f_not) return NULL;
	}
	return filt;
}

static gboolean filter_to_string_rec(GString *string, const MafwFilter *filter)
{
	gboolean valid = FALSE;
	guchar symbol;

	if (!MAFW_FILTER_IS_VALID(filter)) {
		return FALSE;
	}

	g_string_append_c(string, '(');
	symbol = type_to_char(filter->type);

	if (MAFW_FILTER_IS_COMPLEX(filter)) {
		guint i = 0;

		g_string_append_c(string, symbol);

		do {
			valid = filter_to_string_rec(string,
						     filter->parts[i++]);
		} while (valid && filter->parts[i] != NULL &&
			 filter->type != mafw_f_not);

		if (filter->type == mafw_f_not && filter->parts[i] != NULL) {
			valid = FALSE;
		}
	} else if (MAFW_FILTER_IS_SIMPLE(filter) && filter->key != NULL &&
		   filter->key[0] != '\0' &&
		   (filter->type != mafw_f_exists ||
		    filter->value == NULL ||
		    filter->value[0] == '\0')) {
		gchar *quoted_key;

		quoted_key = mafw_filter_quote(filter->key);
                if (!quoted_key) {
                        return FALSE;
                }
		g_string_append(string, quoted_key);
		g_free(quoted_key);

		g_string_append_c(string, symbol);

		if (filter->value != NULL && filter->value[0] != '\0') {
			gchar *quoted_value;
			quoted_value = mafw_filter_quote(filter->value);
                        if (!quoted_value) {
                                return FALSE;
                        }
			g_string_append(string, quoted_value);
			g_free(quoted_value);
		}
		valid = TRUE;
	}

	if (valid) {
		g_string_append_c(string, ')');
	}

	return valid;
}


/**
 * mafw_filter_parse:
 * @filter: the string representation of a MAFW filter.
 *
 * Builds a filter tree from the given @filter expression.
 *
 * Returns: a newly allocated filter tree, or %NULL if the string contains
 *          syntax errors.
 */
MafwFilter *mafw_filter_parse(char const *filter)
{
	char const *fend;
	MafwFilter *filt = NULL;

	if (filter != NULL) {
		fend = parse_sexp(filter, &filt);
		if (!fend || *fend) {
			mafw_filter_free(filt);
			filt = NULL;
		}
	}

	return filt;
}

/**
 * mafw_filter_to_string:
 * @filter: a #MafwFilter
 *
 * Transforms the #MafwFilter into a string representation. If you
 * created that #MafwFilter from a string representation, the result
 * of calling this function doesn't have to return the same string,
 * just the an equivalent one.
 *
 * Returns: a newly allocated string, or %NULL if the filter was %NULL
 * or incorrect
 */
gchar *mafw_filter_to_string(const MafwFilter *filter)
{
	gchar *converted_string = NULL;

	if (filter != NULL) {
		GString *string;
		gboolean valid;

		string = g_string_new("");
		valid = filter_to_string_rec(string, filter);
		converted_string = g_string_free(string, !valid);
	}

	return converted_string;
}

/**
 * mafw_filter_copy:
 * @filter: a #MafwFilter
 *
 * Copies a #MafwFilter
 *
 * Returns: a newly allocated #MafwFilter, or %NULL if any error
 * occurs
 */
MafwFilter *mafw_filter_copy(const MafwFilter *original)
{
	MafwFilter *copy = NULL;

	if (original == NULL || !MAFW_FILTER_IS_VALID(original)) {
		return NULL;
	}

	if (MAFW_FILTER_IS_COMPLEX(original)) {
		guint size;

		for (size = 0; original->parts[size] != NULL; size++);

		if ((original->type == mafw_f_not && size != 1) || size < 1) {
			return NULL;
		}

		if (original->type == mafw_f_not) {
			MafwFilter *sub = NULL;
			sub = mafw_filter_copy(original->parts[0]);
			copy = sub != NULL ? MAFW_FILTER_NOT(sub) : NULL;
		} else {
			guint i;
			gboolean error = FALSE;
			copy = g_new(MafwFilter, 1);
			copy->type = original->type;
			copy->parts = g_new0(MafwFilter *, size + 1);
			for (i = 0; i < size && !error; i++) {
				MafwFilter *sub = NULL;
				sub = mafw_filter_copy(original->parts[i]);
				if (sub != NULL) {
					copy->parts[i] = sub;
				} else {
					error = TRUE;
				}
			}

			if (error) {
				mafw_filter_free(copy);
				copy = NULL;
			}
		}
	} else if (MAFW_FILTER_IS_SIMPLE(original) && original->key != 0 &&
		   original->key[0] != '\0' &&
		   (original->type != mafw_f_exists ||
		    original->value == NULL ||
		    original->value[0] == '\0')) {
			copy = g_new(MafwFilter, 1);
			copy->type = original->type;
			copy->key = g_strdup(original->key);
			copy->value = g_strdup(original->value);
	}

	return copy;
}

/**
 * mafw_filter_new:
 * @type: the type of the filter (#MafwFilterType).
 * @...:  depending on the type, either a %NULL terminated list of
 *        #MafwFilter:s, or if it's a simple type, a key and a value.
 *        In the second case, the key and value pointers are duplicated.
 *
 * Creates a new filter tree node.
 *
 * Returns: the newly allocated filter node.
 */
MafwFilter *mafw_filter_new(MafwFilterType type, ...)
{
	va_list args;
	MafwFilter *f;

	f = malloc(sizeof(MafwFilter));
        if (!f) {
                return NULL;
        }
	f->type = type;
	if (MAFW_FILTER_IS_COMPLEX(f)) {
		int n;

		/* See how many arguments we have and allocate a
		 * pointer array to store them. */
		va_len(n, type);
		f->parts = malloc(sizeof(MafwFilter *) * (n + 1));
                if (!f->parts) {
                        free(f);
                        return NULL;
                }
		/* Store the passed pointers and terminate with
		 * %NULL. */
		va_start(args, type);
		for (n = 0; (f->parts[n] = va_arg(args, MafwFilter *)); n++);
		f->parts[n] = NULL;
	} else {
		char *v;

		va_start(args, type);
		f->key = strdup(va_arg(args, char *));
		f->value = NULL;
		v = va_arg(args, char *);
		if (v)
			f->value = strdup(v);
	}
	va_end(args);
	return f;
}

/**
 * mafw_filter_add_children_n:
 * @filter: the filter to add to.
 * @...: a %NULL terminated list of #MafwFilter:s to add.
 *
 * Adds children nodes to the given filter.  Does nothing if the
 * filter is not an aggregate filter (AND, OR).  Use the
 * mafw_filter_add_children() convenience macro that adds the
 * terminating %NULL automatically.
 *
 * Returns: the modified filter.
 */
MafwFilter *mafw_filter_add_children_n(MafwFilter *filter, ...)
{
	int plen, clen;
	va_list args;

        g_return_val_if_fail (filter != NULL, NULL);

        if (!filter) {
                return NULL;
        }
	if (filter->type != mafw_f_and && filter->type != mafw_f_or)
		return filter;

	/* Find out how many we need to add. */
	va_len(clen, filter);

	/* And how many we have now. */
	for (plen = 0; filter->parts[plen]; plen++);

	/* Adjust the size, and add the items after the existing. */
	filter->parts = realloc(filter->parts, sizeof(MafwFilter *) * (plen + clen + 1));
        if (!filter->parts) {
                return NULL;
        }
	va_start(args, filter);
	for (; (filter->parts[plen] = va_arg(args, MafwFilter *)); plen++);
	va_end(args);

	/* Terminate with NULL. */
	filter->parts[plen] = NULL;

	return filter;
}

typedef void (*MafwFilterVisitor)(MafwFilter *filter, void *data);

/*
 * mafw_filter_traverse:
 * @filter: the filter tree to traverse.
 * @func: the visitor function to call for each node.
 * @data: the additional data to pass to the visitor.
 *
 * Traverses the given filter tree post-order.
 */
static void mafw_filter_traverse(MafwFilter *filter,
				 MafwFilterVisitor func, void *data)
{
	if (filter == NULL || !MAFW_FILTER_IS_VALID(filter)) return;
	if (MAFW_FILTER_IS_COMPLEX(filter)) {
		MafwFilter **parts;

		for (parts = filter->parts; parts && *parts; parts++)
			mafw_filter_traverse(*parts, func, data);
	}
	func(filter, data);
}

/*
 * Frees the memory associated with this filter, and sets it's type to
 * invalid.
 */
static void mafw_filter_free_1(MafwFilter *filter, void *unused)
{
	if (MAFW_FILTER_IS_COMPLEX(filter)) {
		free(filter->parts);
	} else {
		free(filter->key);
		if (filter->value) free(filter->value);
	}
	filter->type = MAFW_F_INVALID;
	free(filter);
}

/**
 * mafw_filter_free:
 * @filter: the filter tree to free.
 *
 * Frees a filter tree recursively.  Does nothing if %NULL is passed.
 */
void mafw_filter_free(MafwFilter *filter)
{
	if (!filter) return;
	mafw_filter_traverse(filter, mafw_filter_free_1, 0);
}
