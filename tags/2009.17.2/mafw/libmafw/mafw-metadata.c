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
#include <string.h>
#include <fnmatch.h>

#include "mafw-metadata.h"
#include "mafw-callbas.h"
#include "mafw-filter.h"

/**
 * SECTION: mafwmetadata
 * @short_description: Metadata representation in MAFW
 *
 * Metadata of objects in the framework are represented in #GHashTable:s
 * called mafw metadata hash tables as tag-value pairs.  Tags (keys of
 * the hash table) are strings, while values are either #GValue:s or
 * #GValueArray:s.  The C type of the hash table values depends on the
 * multiplicity: values of tags having exactly one value are #GValue:s,
 * otherwise they are stored in #GValueArray:s.  In a mafw metadata hash
 * table Every tag has at least one one.
 *
 * Use mafw_metadata_new() to create a mafw metadata hash table.
 * You can use mafw_metadata_release() when you don't need it
 * anymore, or you can use the regular GLib function.
 *
 * You can add metadata to the hash table with the mafw_metadata_add_*()
 * functions.  If you set the value of the same tag several times the
 * values will automatically be merged, forming a multiple-valued tag.
 * String values set with mafw_metadata_add_string() are duplicated.
 * Once you set one type of value to a metadata tag you must not set
 * another one with a different type.
 *
 * One way to query the structure is using ordinary g_hash_table_*()
 * functions.  This case with mafw_metadata_nvalues() you can enable
 * your client to deal with multiple-valued tags correctly.  However,
 * if you are only prepared to handle single-valued ones you may use
 * the mafw_metadata_first() function.
 *
 * Software development is facilitated by the mafw_metadata_print*()
 * functions.  You can extend the set of recognized metadata value types
 * by adding cases to check_mdvtype() and mafw_callbas_argv2gval().
 * Make sure you update mafw_metadata_freeze() and mafw_metadata_thaw() too.
 */

/* Private functions */
/*
 * Destructs a value of a mafw metadata hash table allocated with
 * mafw_metadata_new().  In the hash table the values can either
 * be #GValue:s or #GValueArray:s.  This function deals with both
 * cases.
 */
static void value_dtor(gpointer value)
{
	if (G_IS_VALUE(value)) {
		g_value_unset(value);
		g_free(value);
	} else
		g_value_array_free(value);
}

/*
 * Checks whether $type is allowed for a metadata value.  We can't afford
 * arbitrary types because we need to be able to serialize MAFW metadata
 * hash tables always.
 * NOTE that we don't support as many kinds of GValue:s in a MAFW metadata
 * hash tables as mafw_callbas_argv2gval() do.
 */
static GType check_mdvtype(GType type)
{
	switch (type) {
	case G_TYPE_BOOLEAN:
	case G_TYPE_INT:
	case G_TYPE_UINT:
	case G_TYPE_LONG:
	case G_TYPE_ULONG:
	case G_TYPE_INT64:
	case G_TYPE_UINT64:
	case G_TYPE_DOUBLE:
	case G_TYPE_STRING:
		return type;
	default:
		g_assert_not_reached();
	}
}

/* Traverses a @filter and stores all referenced keys in @all. */
static void get_keys_from_filter(GHashTable *all, const MafwFilter *filter)
{
	guint i;

	if (filter->type < MAFW_F_COMPLEX) {
		for (i = 0; filter->parts[i]; i++)
			get_keys_from_filter(all, filter->parts[i]);
	} else
		g_hash_table_insert(all, filter->key, NULL);
}

/* Adds all keys of a hash table to an array.  The strings are not duped. */
static void hash2ary(const gchar *key, gpointer unused, GPtrArray *all)
{
	g_ptr_array_add(all, (gchar *)key);
}

/* GValue transform function to turn strings into integers.
 * While GLib duplicates and wraps everyting under the sun
 * they could not afford adding useful functionality it seems. */
static void gvstr2gvint(const GValue *src, GValue *dst)
{
	dst->data[0].v_int = atoi(src->data[0].v_pointer);
}

/*
 * Evaluates @filter as a MafwFilter (sub)expression.
 * Returns TRUE or FALSE if @md matches @filter, or -1
 * if it could not be decided.
 */
static gint eval_filter(GHashTable *md, const MafwFilter *filter,
		       	MafwMetadataComparator funcomp)
{
	if (filter->type < MAFW_F_COMPLEX) {
		guint i;
		gint ret;
		gboolean cond, action;

		/*
		 * Complex filter expression, let's process AND, OR and
		 * NOT together.  $cond is the short-circuit condition,
		 * $action is the value to be returned if $cond holds
		 * for a subexpression.
		 */
		switch (filter->type) {
		case mafw_f_and:
			/* Any FALSE ==> FALSE */
			cond = action = FALSE;
			break;
		case mafw_f_or:
			/* Any TRUE  ==> TRUE */
			cond = action = TRUE;
			break;
		case mafw_f_not:
			/*
			 * Any TRUE ==> FALSE
			 * mafw_filter_parse() does not actually allow
			 * multiple subexpressions in a negation, but
			 * for the sake of completeness handle it as
			 * NAND.
			 */
			cond   = TRUE;
			action = FALSE;
			break;
		default:
			g_assert_not_reached();
		}

		/* Evaluate the subexpressions; return -1 if all of them
		 * was undecidable. */
		ret = -1;
		for (i = 0; filter->parts[i]; i++) {
			int now;

			now = eval_filter(md, filter->parts[i], funcomp);
			if (now == cond)
				return action;
			else if (now == !cond)
				ret = !action;
		}
		return ret;
	} else if (filter->type == mafw_f_exists) {
		/* The most simple expression */
		return g_hash_table_lookup(md, filter->key) != NULL;
	} else {
		guint i;
		GType vtype;
		gboolean ret;
		gpointer lhs;
		GValue rhs_str, rhs_natural, *rhs;

		/* Simple expression */
		g_assert(filter->type < MAFW_F_LAST);

		/* Is the relation decidable? */
		if (!(lhs = g_hash_table_lookup(md, filter->key)))
			return -1;

		/* Put $filter->value into a GValue we can pass to $funcomp().
		 * Convert it to the G_VALUE_TYPE of $lhs if necessary.
		 * Back out if it's not possible. */
		memset(&rhs_str, 0, sizeof(rhs_str));
		g_value_init(&rhs_str, G_TYPE_STRING);
		g_value_set_static_string(&rhs_str, filter->value);
		vtype = G_IS_VALUE(lhs)
		       	? G_VALUE_TYPE(lhs)
			: G_VALUE_TYPE(g_value_array_get_nth(lhs, 0));
		if (vtype != G_TYPE_STRING) {
			memset(&rhs_natural, 0, sizeof(rhs_natural));
			g_value_init(&rhs_natural, vtype);
			if (!g_value_transform(&rhs_str, &rhs_natural))
				return -1;
			rhs = &rhs_natural;
		} else
			rhs = &rhs_str;

		/* Single-valued tag ==> ask $funcomp() directly. */
		if (G_IS_VALUE(lhs)) {
			ret = funcomp(filter->type, filter->key, lhs, rhs);
			goto finish;
		}

		/* Multi-valued tag, return whether at least one
		 * of the values holds against the relation. */
		ret = FALSE;
		for (i = 0; i < ((GValueArray *)lhs)->n_values; i++) {
			ret = funcomp(filter->type, filter->key,
				      g_value_array_get_nth(lhs, i), rhs);
			if (ret == TRUE)
				break;
		}

finish:
		/* Free $rhs_natural if we used it. */
		if (rhs == &rhs_natural)
			g_value_unset(&rhs_natural);
		return ret;
	}
}

/*
 * Wraps a MafwMetadataComparator to compare two values of the same key
 * from a mafw metadata hash table and returns -1, +1 or 0 if @lhs is
 * found to be less than, greater than or equal to @rhs.
 */
static gint compare_mvals(const GValue *lhs, const GValue *rhs,
			  const gchar *key, MafwMetadataComparator funcomp)
{
	/* $funcomp() can only tell us if $lhs and $rhs are in a particular
	 * relation, so lots of time we'll need to call it more than once.
	 * This can be considered a suboptimal approach. */
	if	(funcomp(mafw_f_lt, key, lhs, rhs))
		return -1;
	else if (funcomp(mafw_f_gt, key, lhs, rhs))
		return +1;
	else	/* If $lhs is neither less nor greater than $rhs
		   they must be equal. */
		return 0;
}

/* Interface functions */

/**
 * mafw_metadata_new:
 *
 * Creates a new mafw metadata hash table.  The hash table has string keys,
 * and either #GValue or #GValueArray values.
 *
 * Returns: a new hash table that can be filled with metadata and with
 * the suitable destructors for keys and values
 */
GHashTable *mafw_metadata_new(void)
{
	return g_hash_table_new_full(g_str_hash, g_str_equal,
		g_free, value_dtor);
}

/**
 * mafw_metadata_release:
 * @md: hash table
 *
 * Convenience function to release a mafw metadata hash table.
 * Does nothing if @md is %NULL.
 */
void mafw_metadata_release(GHashTable *md)
{
	if (md != NULL)
		g_hash_table_unref(md);
}

/**
 * mafw_metadata_add_something:
 * @md: hash table created with mafw_metadata_new()
 * @key: key to use
 * @argvtype: #GType of the values
 * @nvalues: number of values to add. If zero, nothing is done.
 * @...: list of values of @argvtype to add. All values need to have
 * the same type.
 *
 * Adds @nvalues number of metadata values of type @argvtype to the
 * mafw metadata hash table @md.
 */
void mafw_metadata_add_something(GHashTable *md, const gchar *key,
				 GType argvtype, guint nvalues, ...)
{
	va_list argvals;
	gpointer mdvals;

	/* Anything to do? */
	if (!nvalues)
		return;

	/* Are we dealing with multiple-valued metadata tags? */
	va_start(argvals, nvalues);
	if ((mdvals = g_hash_table_lookup(md, key)) != NULL || nvalues > 1) {
		GValue newval;
		GType mdvtype;

		/* Yes.  The value of the hash table entry will be
		 * a GValueArray. */
		if (!mdvals || G_IS_VALUE(mdvals)) {
			if (mdvals != NULL) {
				GValue *mdval;

				/* $key has exactly 1 value for the moment. */
				mdval = mdvals;
				mdvals = g_value_array_new(1+nvalues);
				mdvtype = G_VALUE_TYPE(mdval);

				/* g_value_array_append() appends a *copy*
				 * of $mdval.  $mdval is free()d by
				 * g_hash_table_insert(). */
				g_value_array_append(mdvals, mdval);
			} else {
				/* $key doesn't exist yet, create $mdvals. */
				mdvals = g_value_array_new(nvalues);
				mdvtype = G_TYPE_INVALID;
			}
			g_hash_table_insert(md, g_strdup(key), mdvals);
		} else	/* All the values must have the same $argvtype. */
			mdvtype = G_VALUE_TYPE(g_value_array_get_nth(mdvals,
								     0));

		/* Append $argvals:s one by one.  First they are temporarily
		 * placed into $newval, which g_value_array_append() will
		 * pick up and copy. */
		memset(&newval, 0, sizeof(newval));
		do {
			if (argvtype == G_TYPE_VALUE) {
				GValue *argval;

				argval = va_arg(argvals, GValue *);
				g_assert(G_IS_VALUE(argval));
				if (mdvtype != G_TYPE_INVALID)
					g_assert(G_VALUE_HOLDS(argval,
							       mdvtype));
				else
					mdvtype = check_mdvtype(
						     G_VALUE_TYPE(argval));
				g_value_array_append(mdvals, argval);
			} else {
				if (mdvtype != G_TYPE_INVALID)
					g_assert(argvtype == mdvtype);
				else
					mdvtype = argvtype;
				mafw_callbas_argv2gval(&newval,
							argvtype, &argvals);
				g_value_array_append(mdvals, &newval);
				g_value_unset(&newval);
			}
		} while (--nvalues > 0);
	} else {
		GValue *newval;

		/* This is the first occurrance of $key in $md. */
		newval = g_new0(GValue, 1);
		if (argvtype == G_TYPE_VALUE) {
			GValue *argval;

			argval = va_arg(argvals, GValue *);
			g_assert(G_IS_VALUE(argval));
			g_value_init(newval,
				     check_mdvtype(G_VALUE_TYPE(argval)));
			g_value_copy(argval, newval);
		} else
			mafw_callbas_argv2gval(newval, argvtype, &argvals);

		g_hash_table_insert(md, g_strdup(key), newval);
	}
	va_end(argvals);
}

/**
 * mafw_metadata_nvalues:
 * @value: value
 *
 * Use this function to determine how to handle @value, which shall be
 * obtained with an ordinary g_hash_table_lookup().  If @value is
 * %NULL (lookup failed) 0 is returned.  If the tag is found to have
 * one value you may assume @value is a #GValue.  Otherwise it needs to
 * be treated as a #GValueArray.
 * 
 * Returns: the number of values of a metadata tag from a mafw
 * metadata hash table.  
 */
guint mafw_metadata_nvalues(gconstpointer value)
{
	if (!value)
		return 0;
	else if (G_IS_VALUE(value))
		return 1;
	else
		return ((GValueArray *)value)->n_values;
}

/**
 * mafw_metadata_first:
 * @md: hash table
 * @key: key
 *
 * Convenience function to obtain the first #GValue of @key in the mafw
 * metadata hash table @md.  Returns %NULL if no such @key in @md.
 *
 * Returns: a #GValue containing the first value with that #key.
 */
GValue *mafw_metadata_first(GHashTable *md, const gchar *key)
{
	gpointer value;

	value = g_hash_table_lookup(md, key);
	if (!value)
		return NULL;
	else if (G_IS_VALUE(value))
		return value;
	else {
		g_assert(((GValueArray *)value)->n_values > 1);
		return g_value_array_get_nth(value, 0);
	}
}

/**
 * mafw_metadata_print_one:
 * @key: key
 * @val: value
 * @domain: domain
 *
 * Like mafw_metadata_print(), but prints only the given @key-@val pair,
 * as obtained from a mafw metadata hash table.
 */
void mafw_metadata_print_one(const gchar *key, gpointer val,
			     const gchar *domain)
{
	GValue strval;

	memset(&strval, 0, sizeof(strval));
	if (G_IS_VALUE(val)) {
		g_value_init(&strval, G_TYPE_STRING);
		g_value_transform(val, &strval);

		if (domain) {
			g_log(domain, G_LOG_LEVEL_DEBUG, "\t%s: `%s'",
			      key, g_value_get_string(&strval));
		} else
			g_print("\t%s: `%s'\n",
			       	key, g_value_get_string(&strval));

		g_value_unset(&strval);
	} else {
		guint i;
		GString *str;
		GValueArray *values;

		str = g_string_new(NULL);

		values = val;
		for (i = 0; i < values->n_values; i++) {
			g_value_init(&strval, G_TYPE_STRING);
			g_value_transform(g_value_array_get_nth(values, i),
					  &strval);

			if (i > 0)
				g_string_append(str, ", ");
			g_string_append_c(str, '`');
			g_string_append(str, g_value_get_string(&strval));
			g_string_append_c(str, '\'');
			g_value_unset(&strval);
		}

		if (domain)
			g_log(domain, G_LOG_LEVEL_DEBUG, "\t%s: %s",
			      key, str->str);
		else
			g_print("\t%s: %s\n", key, str->str);
		g_string_free(str, TRUE);
	}
}

/**
 * mafw_metadata_print:
 * @md: hash table
 * @domain: domain. If @domain is NULL entries are g_print()ed,
 * otherwise they are g_log()ed from @domain at debug level.
 * 
 * Dumps the contents of a mafw metadata hash table.
 * 
 * The format of the output looks like:
 * <code>
 * """
 * &lt;tab&gt;&lt;key1&gt;: `&lt;value11&gt;'
 * &lt;tab&gt;&lt;key2&gt;: `&lt;value21&gt;', `&lt;value22&gt;', `&lt;value33&gt;'
 * """.
 * </code>
 */
void mafw_metadata_print(GHashTable *md, const gchar *domain)
{
	g_hash_table_foreach(md, (GHFunc)mafw_metadata_print_one,
			     (gpointer)domain);
}

/**
 * mafw_metadata_sorting_terms:
 * @sorting: sorting criteria
 *
 * Creates a spliced sorting term array from a mafw_source_browse()
 * sorting criteria suitable for mafw_metadata_compare() as input.
 *
 * Returns: %NULL if @sorting is %NULL or empty.  Free with g_strfreev().
 */
gchar **mafw_metadata_sorting_terms(const gchar *sorting)
{
	if (!sorting || !sorting[0])
		return NULL;

	/* Make a generous scream if $sorting is delimited wrong. */
	g_assert(strcspn(sorting, " \t\n") == strlen(sorting));

	/* Lots of talk expressed in little code. */
	return g_strsplit(sorting, ",", -1);
}

/**
 * mafw_metadata_relevant_keys:
 * @keys: keys
 * @filter: filter
 * @sorting: sorting criteria
 *
 * Helps deciding what metadata a #MafwSource implementation will need
 * while browsing the upstream in order to be able to observe the filter
 * and sorting criteria of mafw_source_browse().
 * The strings in the array are taken directly from @keys, @filter and
 * @sorting without duplication, so the returned array is valid only as
 * long as the structures are valid.  Free the array with g_free().
 *
 * <note><para> Some actions are are performed not to include
 * duplicate tag names in the returned array, but it is not
 * guaranteed.</para></note>
 *
 * Returns: all metadata tags in a %NULL-terminated string array being
 * referenced either in @keys, @filter or @sorting. Any of the
 * parameters may be %NULL. Returns %NULL if no tags are referred at
 * all.
 */
const gchar **mafw_metadata_relevant_keys(const gchar *const *keys,
					  const MafwFilter *filter,
					  const gchar *const *sorting)
{
	guint i;
	GPtrArray *vall;
	GHashTable *hall;

	/* Shortcut if possible. */
	if (!filter && !sorting) {
		size_t skeys;

		if (!keys || !keys[0])
			/* User wants no metadata at all. */
			return NULL;

		/* Duplicate $keys without duping its strings.
		 * Good morning glib, ever heard of the great invention
		 * of the last century, const correctness? */
		skeys = sizeof(*keys) * (g_strv_length((gchar **)keys)+1);
		return g_memdup(keys, skeys);
	}

	/* Collect all tag names in $hall so that we ignore duplicates.
	 * (Tags referred in both, say, $keys and $filter.) */
	hall = g_hash_table_new(g_str_hash, g_str_equal);

	if (keys != NULL)
		for (i = 0; keys[i]; i++)
			g_hash_table_insert(hall, (gchar *)keys[i], NULL);

	if (filter != NULL)
		get_keys_from_filter(hall, filter);

	if (sorting != NULL) {
		for (i = 0; sorting[i]; i++) {
			const gchar *key;

			key = sorting[i];
			if (key[0] == '+' || key[0] == '-')
				key++;
			g_hash_table_insert(hall, (gchar *)key, NULL);
		}
	}

	/* Get the (uniqe) keys of $hall and put them into an array. */
	vall = g_ptr_array_new();
	g_hash_table_foreach(hall, (GHFunc)hash2ary, vall);
	g_hash_table_unref(hall);

	/* Free the array and return what we have. */
	if (vall->len > 0) {
		g_ptr_array_add(vall, NULL);
		return (const gchar **)g_ptr_array_free(vall, FALSE);
	} else {
		g_ptr_array_free(vall, TRUE);
		return NULL;
	}
}

/**
 * mafw_metadata_ordered:
 * @rel: filter type
 * @key: key
 * @lhsgv: left comparison element
 * @rhsgv: right comparison element
 *
 * Generic mafw_metadata_filter() comparator capable of dealing with
 * strings and integers.  It is used by default, and you are advised
 * to call it as a fallback from your custom comparator function.
 * Strings are compared ignoring their case.  The approximate matching
 * of strings is executed in terms of globbing. @key is ignored.
 *
 * Returns: a positive integer if left argument is bigger than right,
 * a negative if left is smaller than right and 0 if equal.
 */
gboolean mafw_metadata_ordered(MafwFilterType rel, const gchar *key,
			       const GValue *lhsgv, const GValue *rhsgv)
{
	g_assert(G_VALUE_TYPE(lhsgv) == G_VALUE_TYPE(rhsgv));

	// One could define special ordering eg. for content type
	// order video/* above audio/*.
	switch (G_VALUE_TYPE(lhsgv)) {
	case G_TYPE_STRING: {
		const gchar *lhs, *rhs;

		/* Wrap strcasecmp().  A sophisticated data model
		 * would do it for us in the box (GValue). */
		lhs = g_value_get_string(lhsgv);
		rhs = g_value_get_string(rhsgv);

		/* TODO We're totally UTF-8-unaware. */
		switch (rel) {
		case mafw_f_eq:
			return !strcasecmp(lhs, rhs);
		case mafw_f_approx:
			/*
			 * Wwwwarninghhh, FNM_CASEFOLD is a glibc extension!
			 * shhhhh!  This is because g_pattern_match() cannot
			 * ignore case.
			 *
			 * TODO fnmatch()'s quoting is different from LDAP,
			 * which we don't handle at the moment.
			 */
			return fnmatch(rhs, lhs, FNM_CASEFOLD) == 0;
		case mafw_f_lt:
			return strcasecmp(lhs, rhs) < 0;
		case mafw_f_gt:
			return strcasecmp(lhs, rhs) > 0;
		default:
			g_assert_not_reached();
		}
	}
	case G_TYPE_INT: {
		gint lhs, rhs;

		/* The same junk for G_TYPE_INT. */
		lhs = g_value_get_int(lhsgv);
		rhs = g_value_get_int(rhsgv);

		switch (rel) {
		case mafw_f_eq:
		case mafw_f_approx:
			/* mafw_f_approx and mafw_f_eq are the same for
			 * integers. */
			return lhs == rhs;
		case mafw_f_lt:
			/* This case it matters that $lhs is interpreted
			 * as a signed integer. */
			return lhs  < rhs;
		case mafw_f_gt:
			return lhs  > rhs;
		default:
			g_assert_not_reached();
		}
	}
	default:
		g_assert_not_reached();
	}
}

/**
 * mafw_metadata_filter:
 * @md: hash table
 * @filter: filter
 * @funcomp: comparison function
 *
 * Utility function to help filtering browse results in
 * mafw_source_browse() implementations.
 *
 * Returns whether mafw metadata hash table @md matches @filter,
 * a preparsed filter expression.  For the purpose of evaluating
 * the filter metadata tags (keys of @md) are considered attributes.
 * @funcomp is a function used to compare metadata.  If it's %NULL
 * mafw_metadata_ordered() is assumed.
 *
 * The semantics of @filter is complemented as follows:
 * <itemizedlist>
 * <listitem><para>Simple expressions referring to tags not existing
 * in @md are ignored.</para></listitem>
 * <listitem><para>Relations over multiple-valued tags are asserted if
 *    the relation holds for at least one value.</para></listitem>
 * <listitem><para>For the purpose of comparison metadata of @filter
 *    are converted to the type of the value of the appropriate tag of
 *    @md.</para></listitem>
 * <listitem><para>Filters not referring to existing keys in @md are
 * considered matching.</para></listitem>
 * <listitem><para>Empty metadata (@md is %NULL) or empty filter
 * (@filter is %NULL) match with everything.</para></listitem>
 * </itemizedlist>
 *
 * Returns: %TRUE if match, %FALSE otherwise.
 */
gboolean mafw_metadata_filter(GHashTable *md, const MafwFilter *filter,
			      MafwMetadataComparator funcomp)
{
	static gboolean hacked = FALSE;

	if (!filter || !md)
		return TRUE;

	/*
	 * We'll need to convert strings of $filter into types accepted
	 * in the hash table to perform comparison.  Note that it will
	 * (probably) take over any previous conversions between these
	 * types if the user happened to define one.
	 */
	if (!hacked) {
		g_value_register_transform_func(G_TYPE_STRING, G_TYPE_INT,
						gvstr2gvint);
		hacked = TRUE;
	}
	
	if (!funcomp)
		funcomp = mafw_metadata_ordered;
	return eval_filter(md, filter, funcomp) != FALSE;
}

/**
 * mafw_metadata_compare: 
 * @md1: first hash table
 * @md2: second hash table
 * @terms: comparison terms
 * @funcomp: comparison function
 *
 * Utility function to be called from #GCompareDataFunc:tions to help
 * sorting browse results in mafw_source_browse() implementations.
 *
 * Compares two mafw metadata hash tables, like a #GCompareDataFunc
 * (but not exactly so because it takes more parameters).  The order
 * of the hash tables is a function of the sorting @terms, a sliced
 * mafw_source_browse() sorting expression. @funcomp is used to
 * compare metadata values, and defaults to mafw_metadata_ordered().
 *
 * For each tag in @terms if one of the hash tables has value for it,
 * but the other does not, the latter is sorted downwards.  Otherwise
 * the values are compared, and if they are sorting equally the next
 * term is considered.  If either or both of the hash tables are %NULL
 * it is handled as if it did not have value for any of the @terms.
 *
 * During comparison as many values of the same key are compared one
 * by one as possible, but not more.  Since a key may have multiple
 * values this can be more than one.  If difference is found that will
 * determine the order of the hash tables, observing the direction
 * requested in current sorting term.  Otherwise the hash table having
 * less values for the current tag will be sorted upwards.
 * Conceptually, this is very similar to strcmp().
 *
 * Returns: value greater than 0 if first value is greater than
 * second, a negative value if first smaller than second, and 0 if
 * both are equal.
 */
gint mafw_metadata_compare(GHashTable *md1, GHashTable *md2,
			   const gchar *const *terms,
			   MafwMetadataComparator funcomp)
{
	guint i;

	/*
	 * This is the only thing we can tell upfront if one of the
	 * metadata structures is NULL because it is not evident that
	 * eg. if $md2 is NULL but $md1 is NOT then $md1 is "lighter";
	 * it depends on whether $md1 includes keys from $term.  If it
	 * does not, they are equal.
	 */
	if (md1 == NULL && md2 == NULL)
		return 0;

	/* Use the default $funcomp? */
	if (!funcomp)
		funcomp = mafw_metadata_ordered;

	/* Try to find a difference between the hashes in $terms[$i].
	 * If it falils, try with the next term.  If we've run out of
	 * $terms declare equality between $md1 and $md2. */
	for (i = 0; terms[i]; i++) {
		gint cmp, dir;
		const gchar *key;
		gpointer lhs, rhs;

		/* Parse the current term into a key and a direction. */
		key = terms[i];
		if (key[0] == '+') {
			dir = +1;
			key++;
		} else if (key[0] == '-') {
			dir = -1;
			key++;
		} else	/* Should not happen, but anyway. */
			dir = +1;

		/* Get the values to be compared. */
		lhs = md1 ? g_hash_table_lookup(md1, key) : NULL;
		rhs = md2 ? g_hash_table_lookup(md2, key) : NULL;

		/*
		 * If one hash table lacks a $key that the other has,
		 * sort the former one downwards.  This is the opposit
		 * of what some SQL implementations (eg. SQLite) do,
		 * which sort NULL upwards, but it seems to make more
		 * sense in our case.
		 */
		if	( lhs && !rhs)
			return -1;
		else if (!lhs &&  rhs)
			return +1;
		else if (!lhs && !rhs)
			continue;

		/* Comparison depends on whether $lhs or $rhs
		 * is multi-valued. */
		if	( G_IS_VALUE(lhs) &&  G_IS_VALUE(rhs)) {
			/* Both are single, just compare them and
			 * if they are not equal, end of story.
			 * Otherwise try with another $key. */
			cmp = compare_mvals(lhs, rhs, key, funcomp) * dir;
			if (cmp != 0)
				return cmp;
		} else if ( G_IS_VALUE(lhs) && !G_IS_VALUE(rhs)) {
			/* $lhs is single, $rhs is multi.  Compare $lhs
			 * with $rhs[0].  If they are found equal, sort
			 * the multi-valued side downwards. */
			cmp = compare_mvals(lhs, g_value_array_get_nth(rhs, i),
				     key, funcomp) * dir;
			return cmp != 0 ? cmp : -1*dir;
		} else if (!G_IS_VALUE(lhs) &&  G_IS_VALUE(rhs)) {
			/* Likewise, but this case $lhs is multi-valued. */
			cmp = compare_mvals(g_value_array_get_nth(lhs, i), rhs,
				     key, funcomp) * dir;
			return cmp != 0 ? cmp : +1*dir;
		} else {
			guint o, nl, nr;

			/*
			 * Both $lhs and $rhs are multi-valued.
			 * Compare $lhs[$i] with $rhs[$i] until
			 * we find inequality or run out of values
			 * on one of the sides.
			 */
			nl = ((GValueArray *)lhs)->n_values;
			nr = ((GValueArray *)rhs)->n_values;
			for (o = 0; o < nl && o < nr; o++) {
				cmp=compare_mvals(g_value_array_get_nth(lhs,o),
						  g_value_array_get_nth(rhs,o),
					     key, funcomp) * dir;
				if (cmp != 0)
					return cmp;
			}

			/* All examined values seems equal.  Sort the one
			 * with less keys upwards.  If both sides had the
			 * same number of values, try with another $key. */
			if	(nl < nr)
				return -1*dir;
			else if (nl > nr)
				return +1*dir;
		}
	}

	/* Can't believe, $md1 and $md2 are equal wrt. to $terms. */
	return 0;
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
