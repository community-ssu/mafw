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

#include <string.h>
#include "checkmore.h"

#include <libmafw/mafw.h>
#include <libmafw/mafw-uri-source.h>

/* The invasion of mockups. */
typedef struct {
	GObjectClass parent;
} DumbPlsClass;

typedef struct {
	GObject parent;
} DumbPls;

static const gchar *Contents[] = {
	MAFW_URI_SOURCE_UUID "::" "file:///alpha",
	MAFW_URI_SOURCE_UUID "::" "file:///beta",
	MAFW_URI_SOURCE_UUID "::" "file:///gamma",
	MAFW_URI_SOURCE_UUID "::" "file:///delta",
	MAFW_URI_SOURCE_UUID "::" "file:///epsilon",
	MAFW_URI_SOURCE_UUID "::" "file:///whiskey",
	MAFW_URI_SOURCE_UUID "::" "file:///tango",
	MAFW_URI_SOURCE_UUID "::" "file:///foxtrot",
};
#define PLS_SIZE G_N_ELEMENTS(Contents)
#define URI(oid) (&oid[sizeof(MAFW_URI_SOURCE_UUID "::") - 1])

static gchar *dumbpls_get_item(MafwPlaylist *pls, guint index, GError **errp)
{
	if (index >= PLS_SIZE)
		return NULL;
	return g_strdup(Contents[index]);
}
static guint dumbpls_get_size(MafwPlaylist *pls, GError **errp)
{
	return PLS_SIZE;
}
static GType dumbpls_get_type(void);
static void dumbpls_init(DumbPls *obj) {/* NOP */}
static void dumbpls_class_init(DumbPlsClass *cls)
{
	GObjectClass *ocls;

	ocls = G_OBJECT_CLASS(cls);
	ocls->set_property = (gpointer)0xfafafafa;
	ocls->get_property = (gpointer)0xdededede;
	g_object_class_override_property(ocls, 1, "name");
	g_object_class_override_property(ocls, 1, "repeat");
	g_object_class_override_property(ocls, 1, "is-shuffled");
}
static void dumbpls_iface_init(MafwPlaylistIface *iface)
{
	iface->get_item = dumbpls_get_item;
	iface->get_size = dumbpls_get_size;
}

G_DEFINE_TYPE_WITH_CODE(DumbPls, dumbpls, G_TYPE_OBJECT,
			G_IMPLEMENT_INTERFACE(MAFW_TYPE_PLAYLIST,
					      dumbpls_iface_init));

/* URI source hook. */
static void (*Old_md)(MafwSource *self,
			  const gchar *object_id,
			  const gchar *const *mdkeys,
			  MafwSourceMetadataResultCb cb,
			  gpointer user_data);
static guint Old_md_called;

static void mymd(MafwSource *self,
		     const gchar *object_id,
		     const gchar *const *mdkeys,
		     MafwSourceMetadataResultCb cb,
		     gpointer user_data)
{
	Old_md_called++;
	Old_md(self, object_id, mdkeys, cb, user_data);
}

/* Test cases, finally. */

/* The MIWMD request. */
static gpointer Req;
static MafwPlaylist *Pls;
static guint Itemcb_called;
static gboolean Destructed;

static void destruct(gpointer called)
{
	Destructed = TRUE;
	checkmore_stop_loop();
}

static void itemcb_valid(MafwPlaylist *pls,
			 guint idx,
			 const gchar *oid,
			 GHashTable *md,
			 gpointer _)
{
	GValue *vuri;

	/* We've requested the URI in this case. */
	Itemcb_called++;
	fail_unless(md != NULL);
	vuri = mafw_metadata_first(md, MAFW_METADATA_KEY_URI);
	fail_unless(vuri != NULL);
	fail_if(strcmp(g_value_get_string(vuri), URI(Contents[idx])));
}

START_TEST(test_valid)
{
	/* Ordinary call, [0..2], valid metadata key. */
	Req = mafw_playlist_get_items_md(Pls, 0, 2,
					 MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
					 itemcb_valid, &Destructed, destruct);
	checkmore_spin_loop(-1);
	fail_unless(Itemcb_called == 3);
	fail_unless(Old_md_called == 3);
	fail_unless(Destructed);
	/* [0..0], valid metadata key. */
	Old_md_called = 0;
	Itemcb_called = 0;
	Destructed = FALSE;
	Req = mafw_playlist_get_items_md(Pls, 0, 0,
					 MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
					 itemcb_valid, &Destructed, destruct);
	checkmore_spin_loop(-1);
	fail_unless(Itemcb_called == 1);
	fail_unless(Old_md_called == 1);
	fail_unless(Destructed);
	/* [0..inf), valid metadata key. */
	Old_md_called = 0;
	Itemcb_called = 0;
	Destructed = FALSE;
	Req = mafw_playlist_get_items_md(Pls, 0, -1,
					 MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
					 itemcb_valid, &Destructed, destruct);
	checkmore_spin_loop(-1);
	fail_unless(Itemcb_called == PLS_SIZE);
	fail_unless(Old_md_called == PLS_SIZE);
	fail_unless(Destructed);
	/* [2..inf), valid metadata key. */
	Old_md_called = 0;
	Itemcb_called = 0;
	Destructed = FALSE;
	Req = mafw_playlist_get_items_md(Pls, 2, -1,
					 MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
					 itemcb_valid, &Destructed, destruct);
	checkmore_spin_loop(-1);
	fail_unless(Itemcb_called == PLS_SIZE - 2);
	fail_unless(Old_md_called == PLS_SIZE - 2);
	fail_unless(Destructed);
}
END_TEST


static void itemcb_nomd(MafwPlaylist *pls,
			   guint idx,
			   const gchar *oid,
			   GHashTable *md,
			   gpointer testcase)
{
	/* In this case we have requested NO metadata */
	Itemcb_called++;
	fail_unless(md == NULL);
}

START_TEST(test_invalid)
{
	/* Invalid range of items [0..999]. */
	Req = mafw_playlist_get_items_md(Pls, 0, 999, NULL, itemcb_nomd,
					 &Destructed, destruct);
	checkmore_spin_loop(-1);
	fail_unless(Old_md_called == 0);
	fail_unless(Itemcb_called == PLS_SIZE);
	fail_unless(Destructed);
}
END_TEST

START_TEST(test_invalid_2)
{
	/* Invalid range of items [10..20]. */
	Req = mafw_playlist_get_items_md(Pls, 10, 20, NULL, itemcb_nomd,
					 &Destructed, destruct);
	checkmore_spin_loop(500);
	fail_unless(Old_md_called == 0);
	fail_unless(Itemcb_called == 0);
	fail_unless(Destructed);
}
END_TEST

START_TEST(test_no_md)
{
	/* Request no metadata. */
	Req = mafw_playlist_get_items_md(Pls, 0, 2, NULL, itemcb_nomd,
					 &Destructed, destruct);
	checkmore_spin_loop(-1);
	fail_unless(Itemcb_called == 3);
	fail_unless(Old_md_called == 0);
	fail_unless(Destructed);
}
END_TEST

static void itemcb_cancel(MafwPlaylist *pls,
			  guint idx,
			  const gchar *oid,
			  GHashTable *md,
			  gpointer _)
{
	Itemcb_called++;
	if (idx == 0)
		mafw_playlist_cancel_get_items_md(Req);
	/* It's a failure if we get called after canceling. */
	fail_if(idx > 0);
}

START_TEST(test_cancel_1)
{
	/* Cancelling after the first item. */
	Req = mafw_playlist_get_items_md(Pls, 0, 2, NULL, itemcb_cancel,
					 &Destructed, destruct);
	checkmore_spin_loop(-1);
	fail_unless(Destructed);
	fail_unless(Old_md_called == 0);
}
END_TEST

START_TEST(test_cancel_2)
{
	/* Cancelling before even starting the mainloop. */
	Req = mafw_playlist_get_items_md(Pls, 0, 2, NULL, itemcb_nomd,
					 &Destructed, destruct);
	mafw_playlist_cancel_get_items_md(Req);
	checkmore_spin_loop(50);
	fail_unless(Destructed);
	fail_unless(Itemcb_called == 0);
	fail_unless(Old_md_called == 0);
}
END_TEST

static void multi_dest(gpointer called)
{
	if (++*(guint *)called == 2)
		checkmore_stop_loop();
}

START_TEST(test_multi_1)
{
	guint n_dest;

	/* Multiple requests. */
	n_dest = 0;
	mafw_playlist_get_items_md(Pls, 0, 1,
				   MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
				   itemcb_valid, &n_dest, multi_dest);
	mafw_playlist_get_items_md(Pls, 4, 7,
				   MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
				   itemcb_valid, &n_dest, multi_dest);
	checkmore_spin_loop(-1);
	fail_unless(n_dest == 2);
	fail_unless(Itemcb_called == 6);
	fail_unless(Old_md_called == 6);
}
END_TEST

static void itemcb_cancel_multi(MafwPlaylist *pls,
				guint idx,
				const gchar *oid,
				GHashTable *md,
				gpointer _)
{
	/* Cancel ourselves after the second item. */
	Itemcb_called++;
	if (idx == 1)
		mafw_playlist_cancel_get_items_md(Req);
	fail_if(idx > 1);
}

START_TEST(test_multi_2)
{
	guint n_dest;

	/* Canceling one of multiple requests. */
	n_dest = 0;
	Req = mafw_playlist_get_items_md(Pls, 0, 5, NULL,
					 itemcb_cancel_multi,
					 &n_dest, multi_dest);
	mafw_playlist_get_items_md(Pls, 0, 5,
				   MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI),
				   itemcb_valid, &n_dest, multi_dest);
	checkmore_spin_loop(-1);
	fail_unless(n_dest == 2);
	fail_unless(Itemcb_called == 2 + 6);
	fail_unless(Old_md_called == 0 + 6);
}
END_TEST

/* Fixtures. */
static void setup(void)
{
	Pls = g_object_new(dumbpls_get_type(), NULL);
	Itemcb_called = 0;
	Old_md_called = 0;
	Destructed = FALSE;
}

static void teardown(void)
{
	g_object_unref(Pls);
	Pls = NULL;
}

int main(void)
{
	TCase *tc;
	Suite *suite;

	/* Freeze into the URI source's get_metadata(). */
	g_type_init();
	Old_md = MAFW_SOURCE_GET_CLASS(mafw_get_uri_source())->get_metadata;
	MAFW_SOURCE_GET_CLASS(mafw_get_uri_source())->get_metadata = mymd;

	suite = suite_create("Playlist");
	tc = tcase_create("MWIMD");
	suite_add_tcase(suite, tc);
	tcase_add_checked_fixture(tc, setup, teardown);

	if (1) tcase_add_test(tc, test_valid);
	if (1) tcase_add_test(tc, test_invalid);
	if (1) tcase_add_test(tc, test_invalid_2);
	if (1) tcase_add_test(tc, test_no_md);
	if (1) tcase_add_test(tc, test_cancel_1);
	if (1) tcase_add_test(tc, test_cancel_2);
	if (1) tcase_add_test(tc, test_multi_1);
	if (1) tcase_add_test(tc, test_multi_2);

	return checkmore_run(srunner_create(suite), FALSE);
}
