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
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <libmafw/mafw.h>

/* Local variables */

#define CHAR_BEG 'a'
#define CHAR_END 'k'
#define NREQ (CHAR_END - CHAR_BEG + 1)

static struct termios TSoriginal;
static GIOChannel *Input;
static GMainLoop *Loop;
static MafwPlaylist *Pls;
static gpointer Reqs[NREQ];

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
	MAFW_URI_SOURCE_UUID "::" "file:///2alpha",
	MAFW_URI_SOURCE_UUID "::" "file:///2beta",
	MAFW_URI_SOURCE_UUID "::" "file:///2gamma",
	MAFW_URI_SOURCE_UUID "::" "file:///2delta",
	MAFW_URI_SOURCE_UUID "::" "file:///2epsilon",
	MAFW_URI_SOURCE_UUID "::" "file:///2whiskey",
	MAFW_URI_SOURCE_UUID "::" "file:///2tango",
	MAFW_URI_SOURCE_UUID "::" "file:///2foxtrot",
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

struct info_t {
	MafwSource *self;
	const gchar *object_id;
	const gchar *const *mdkeys;
	MafwSourceMetadataResultCb cb;
	gpointer user_data;
	GError **error;
};

static gboolean runoldmd(gpointer arg)
{
	struct info_t *i;

	i = arg;
	Old_md(i->self, i->object_id, i->mdkeys, i->cb, i->user_data);
	/* we leak $i, but i dont care */
	return FALSE;
}

static void mymd(MafwSource *self,
		     const gchar *object_id,
		     const gchar *const *mdkeys,
		     MafwSourceMetadataResultCb cb,
		     gpointer user_data)
{
	struct info_t *info;

	info = g_new0(struct info_t, 1);
	info->self = self;
	info->object_id = g_strdup(object_id);
	info->mdkeys = mdkeys;
	info->cb = cb;
	info->user_data = user_data;
	g_timeout_add(g_random_int_range(1, 5) * 1000, runoldmd, info);
}

/* The stuff. */
static void gotitem(MafwPlaylist *pls, guint index, const gchar *object_id,
		    GHashTable *metadata, gpointer n)
{
	g_print("%d: got md %u. %s\n", (guint)n, index, object_id);
}

static void reqdone(gpointer n)
{
	g_print("%d: finished\n", (guint)n);
	Reqs[(guint)n] = NULL;
}

static G_GNUC_UNUSED gboolean cancelidle(gpointer req)
{
	g_print("canceling from idle\n");
	mafw_playlist_cancel_get_items_md(req);
	return FALSE;
}

static gboolean process_cmd(GIOChannel *ioc, GIOCondition cond, gpointer _)
{
	enum {
		START, CANCEL
	} op;
	guint n;
	gpointer req;
	char c;
	gsize br;
	GError *err;

	err = NULL;
	g_io_channel_read_chars(ioc, &c, 1, &br, &err);
	if (c == 'q') {
		g_main_loop_quit(Loop);
		return FALSE;
	}else if (c >= CHAR_BEG && c <= CHAR_END) {
		n = c - CHAR_BEG;
		op = START;
	} else if (c >= CHAR_BEG-('a'-'A') && c <= CHAR_END-('a'-'A')) {
		n = c - CHAR_BEG+('a'-'A');
		op = CANCEL;
	} else
		return TRUE;
	req = Reqs[n];
	if (op == CANCEL) {
		if (!req) {
			g_print("%d: not started, cannot cancel.\n", n);
			return TRUE;
		}
		mafw_playlist_cancel_get_items_md(req);
		g_print("%d: cancelling.\n", n);
	}
	if (op == START) {
		guint f, t;

		if (req) {
			g_print("%d: already started.\n", n);
			return TRUE;
		}
		f = g_random_int_range(0, PLS_SIZE);
		t = g_random_int_range(0, PLS_SIZE);
		if (f > t) {
			guint _;
			_ = t; t = f; f = _;
		}
		g_assert(f <= t);
		Reqs[n] = mafw_playlist_get_items_md(Pls, f, t,
#if 1
						      MAFW_SOURCE_LIST("uri"),
#else
						      NULL,
#endif
						      gotitem, (gpointer)n,
						      reqdone);
#if FMPBUG
		g_idle_add(cancelidle, Reqs[n]);
#endif
		g_print("%d: started %u..%u\n", n, f, t);
	}
	return TRUE;
}

int main(void)
{
	struct termios tcs;

	if (!isatty(STDIN_FILENO))
		return 1;
	tcgetattr(STDIN_FILENO, &TSoriginal);
	tcgetattr(STDIN_FILENO, &tcs);
	tcs.c_lflag &= ~(ICANON|ECHO);
	tcs.c_cc[VMIN] = 1;
	tcs.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &tcs);

	Input = g_io_channel_unix_new(STDIN_FILENO);
	g_io_add_watch(Input, G_IO_IN, process_cmd, NULL);

	g_type_init();
	Old_md = MAFW_SOURCE_GET_CLASS(mafw_get_uri_source())->get_metadata;
	MAFW_SOURCE_GET_CLASS(mafw_get_uri_source())->get_metadata = mymd;
	Pls = g_object_new(dumbpls_get_type(), NULL);

	g_print("Manual stress testing of miwmd (multiple items with metadata)\n"
		"operations.  Keys 'a'..'k' start random miwmd operations,\n"
		"pressing one of 'A'..'K' cancels the corresponding request.\n"
		"'q' quits.\n");
	Loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(Loop);

	g_io_channel_unref(Input);

	tcsetattr(STDIN_FILENO, TCSANOW, &TSoriginal);
	return 0;
}
