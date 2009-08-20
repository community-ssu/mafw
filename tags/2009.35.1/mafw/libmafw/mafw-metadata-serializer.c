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

/* Include files */
#include <string.h>
#include <glib-object.h>

#include <libmafw/mafw-metadata.h>
#include <libmafw/mafw-metadata-serializer.h>

/* Program code */
/* Private functions */
/* Serialization */

/* Encodes an integer.  $val can either be signed or unsigned
 * (from the respective domains). */
#define X2BARY(type)							\
	static void type##2bary(GByteArray *bary, g##type val)		\
	{								\
		g_byte_array_append(bary, (guint8 *)&val, sizeof(val));	\
	}

X2BARY(boolean);
X2BARY(int);
X2BARY(long);
X2BARY(int64);
X2BARY(float);
X2BARY(double);

/* Encodes a string together with its termination. */
static void str2bary(GByteArray *bary, const gchar *val)
{
	/* NOTE strlen() is UTF-8 ignorant. */
	g_byte_array_append(bary, (guchar *)val, strlen(val) + 1);
}

/* Encodes a GValue.  You'll need to enhance this function (and bary2gval as
 * well) when adding support for new types. */
static void gval2bary(GByteArray *bary, GValue *value)
{
	guint type;

	type = G_VALUE_TYPE(value);
	int2bary(bary, type);

	if (type == G_TYPE_BOOLEAN)
		boolean2bary(bary, g_value_get_boolean(value));
	else if (type == G_TYPE_INT)
		int2bary(bary, g_value_get_int(value));
	else if (type == G_TYPE_UINT)
		int2bary(bary, (gint)g_value_get_uint(value));
	else if (type == G_TYPE_LONG)
		long2bary(bary, g_value_get_long(value));
	else if (type == G_TYPE_ULONG)
		long2bary(bary, (glong)g_value_get_ulong(value));
	else if (type == G_TYPE_INT64)
		int642bary(bary, g_value_get_int64(value));
	else if (type == G_TYPE_UINT64)
		int642bary(bary, (gint64)g_value_get_uint64(value));
        else if (type == G_TYPE_FLOAT)
                float2bary(bary, g_value_get_float(value));
	else if (type == G_TYPE_DOUBLE)
		double2bary(bary, g_value_get_double(value));
	else if (type == G_TYPE_STRING)
		str2bary(bary, g_value_get_string(value));
	else
		g_assert_not_reached();
}

/**
 * mafw_metadata_val_freeze_bary:
 * @bary: the #GByteArray
 * @val: the pointer
 *
 *  Encodes a mafw metadata hash table value, handling both single-
 * and multiple-valued tags.
 */
void mafw_metadata_val_freeze_bary(GByteArray *bary, gpointer val)
{
	if (G_IS_VALUE(val)) {
		int2bary(bary, 1);
		gval2bary(bary, val);
	} else {
		guint nvalues, i;

		nvalues = ((GValueArray *)val)->n_values;
		int2bary(bary, nvalues);
		for (i = 0; i < nvalues; i++)
			gval2bary(bary, g_value_array_get_nth(val, i));
	}
}

/* Encodes a mafw metadata hash table entry. */
static void mdkv2bary(const gchar *key, gpointer val, GByteArray *bary)
{
	str2bary(bary, key);
	mafw_metadata_val_freeze_bary(bary, val);
}

/* Deserialization */
/* Returns the string starting at *$index in the stream,
 * and updates the pointer to point to the next datum. */
static const gchar *bary2str(GByteArray *bary, gsize *index)
{
	const gchar *str;

	/* End of stream? */
	if (*index >= bary->len)
		return NULL;
	str = (gchar *)&bary->data[*index];
	*index += strlen(str) + 1;
	return str;
}

/* Decodes the integer in the stream at *$index,
 * and advances the pointer. */
#define BARY2X(type)							\
	static g##type bary2##type(GByteArray *bary, gsize *index)	\
	{								\
		g##type val;						\
									\
		g_assert(*index < bary->len);				\
		memcpy(&val, &bary->data[*index], sizeof(val));		\
		*index += sizeof(val);					\
		return val;						\
	}

BARY2X(boolean);
BARY2X(int);
BARY2X(long);
BARY2X(int64);
BARY2X(float);
BARY2X(double);

/* Decodes the serialized mafw metadata hash table value in the stream
 * at *$index, and advances the pointer appropriately. */
static void bary2gval(GValue *value, GByteArray *bary, gsize *index)
{
	guint type;

	type = bary2int(bary, index);
	g_value_init(value, type);
	switch (type) {
	case G_TYPE_BOOLEAN:
		g_value_set_boolean(value, bary2boolean(bary, index));
		break;
	case G_TYPE_INT:
		g_value_set_int(value, bary2int(bary, index));
		break;
	case G_TYPE_UINT:
		g_value_set_uint(value, (guint)bary2int(bary, index));
		break;
	case G_TYPE_LONG:
		g_value_set_long(value, bary2long(bary, index));
		break;
	case G_TYPE_ULONG:
		g_value_set_ulong(value, (gulong)bary2long(bary, index));
		break;
	case G_TYPE_INT64:
		g_value_set_int64(value, bary2int64(bary, index));
		break;
	case G_TYPE_UINT64:
		g_value_set_uint64(value, (guint64)bary2int64(bary, index));
		break;
        case G_TYPE_FLOAT:
                g_value_set_float(value, bary2float(bary, index));
                break;
	case G_TYPE_DOUBLE:
		g_value_set_double(value, bary2double(bary, index));
		break;
	case G_TYPE_STRING:
		g_value_set_string(value, bary2str(bary, index));
		break;
	default:
		g_assert_not_reached();
	}
}

/* Interface functions */
/**
 * mafw_metadata_freeze_bary:
 * @md: hash table.
 *
 * Serializes a mafw metadata hash table.  The returned stream is
 * suitable for sending to another process or storing on the disk,
 * but is not architecture-independent. @md can be %NULL.
 *
 * <itemizedlist>
 * <listitem><code>stream	:= &lt;entry&gt; *</code></listitem>
 * <listitem><code>entry	:= &lt;key&gt; &lt;nvalues&gt; &lt;value&gt; 1*</code></listitem>
 * <listitem><code>key		:= &lt;C-string&gt;</code></listitem>
 * <listitem><code>nvalues	:= &lt;uint32&gt;</code></listitem>
 * <listitem><code>value	:= &lt;GType&gt; &lt;data&gt;</code></listitem>
 * <listitem><code>GType	:= &lt;uint32&gt;</code></listitem>
 * <listitem><code>data		:= &lt;uint32&gt; | &lt;C-string&gt;</code></listitem>
 * </itemizedlist>
 *
 * Returns: a #GByteArray..
 */
GByteArray *mafw_metadata_freeze_bary(GHashTable *md)
{
	GByteArray *bary;

	bary = g_byte_array_new();
	if (md != NULL)
		g_hash_table_foreach(md, (GHFunc)mdkv2bary, bary);
	return bary;
}

/**
 * mafw_metadata_val_thaw_bary:
 * @bary: the #GByteArray
 * @i: the pointer to store the size
 * 
 * Recreates the mafw metadata value, or value-array from its serialized from.
 * If the input stream is found syntactically incorrect the program is aborted.
 *
 * Returns: the pointer
 */
gpointer mafw_metadata_val_thaw_bary(GByteArray *bary, gsize *i)
{
	guint nvalues;
	gpointer val;

	nvalues = bary2int(bary, i);
	g_assert(nvalues > 0);

	if (nvalues > 1) {
		GValue value;
		memset(&value, 0, sizeof(value));
		val = g_value_array_new(nvalues);
		do {
			bary2gval(&value, bary, i);
			g_value_array_append(val, &value);
			g_value_unset(&value);
		} while (--nvalues > 0);
	} else {
		val = g_new0(GValue, 1);
		bary2gval(val, bary, i);
	}

	return val;

}

/**
 * mafw_metadata_thaw_bary:
 * @bary: the byte array
 *
 * Recreates the mafw metadata hash table from its serialized from.
 * The serialized and deserialized hash tables contain the same
 * information, but are not byte-equivalent.  Returns %NULL if @bary
 * does not contain any keys after all.  If the input stream is found
 * syntactically incorrect the program is aborted.
 *
 * Returns: a #GHashTable.
 */
GHashTable *mafw_metadata_thaw_bary(GByteArray *bary)
{
	gsize i;
	GHashTable *md;
	const char *key;

	i = 0;
	md = NULL;
	while ((key = bary2str(bary, &i)) != NULL) {
		if (md == NULL)
			/* Now we can be sure we have at least one key. */
			md = mafw_metadata_new();

		g_hash_table_insert(md, g_strdup(key),
				mafw_metadata_val_thaw_bary(bary, &i));
	}

	return md;
}

/**
 * mafw_metadata_freeze:
 * @md: hash table
 * @sstreamp: pointer to return the stream size
 *
 * Like mafw_metadata_freeze_bary(), but returns a conventional
 * C character array instead of a #GByteArray.
 *
 * Returns: the a conventional gchar*.
 */
gchar *mafw_metadata_freeze(GHashTable *md, gsize *sstreamp)
{
	GByteArray *bary;

	bary = mafw_metadata_freeze_bary(md);
	*sstreamp = bary->len;
	return (gchar *)g_byte_array_free(bary, FALSE);
}

/**
 * mafw_metadata_thaw:
 * @stream: a gchar* with the stream
 * @sstream: the stream size
 *
 * Like mafw_metadata_thaw_bary(), but the input stream is taken
 * from a conventional C character array instead of a #GByteArray.
 *
 * Returns: a #GHashTable.
 */
GHashTable *mafw_metadata_thaw(const gchar *stream, gsize sstream)
{
	GByteArray bary;

	bary.data = (guchar *)stream;
	bary.len = sstream;
	return mafw_metadata_thaw_bary(&bary);
}

/**
 * mafw_metadata_val_freeze:
 * @val: a pointer
 * @sstreamp: the pointer to store the stream size
 *
 * Like mafw_metadata_val_freeze_bary(), but returns a conventional
 * C character array instead of a #GByteArray.
 *
 * Returns: the streams as a gchar*
 */
gchar *mafw_metadata_val_freeze(gpointer val, gsize *sstreamp)
{
	GByteArray *bary;
	
	bary = g_byte_array_new();

	mafw_metadata_val_freeze_bary(bary, val);
	*sstreamp = bary->len;
	return (gchar *)g_byte_array_free(bary, FALSE);
}


/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
