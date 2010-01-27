/* Small helper element for format conversion
 * (c) 2004 Ronald Bultje <rbultje@ronald.bitfreak.net>
 * Portion Copyright © 2009 Nokia Corporation and/or its
 * subsidiary(-ies).* All rights reserved. *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <string.h>


#include "gstscreenshot.h"
#include "mafw-gst-renderer-worker.h"

static GstElement *src;
static GstElement *sink;
static GstElement *pipeline;
static GstBuffer *result;
static MafwGstRendererWorker *current_worker;
static const gchar *metadata_key;

/* GST_DEBUG_CATEGORY_EXTERN (_totem_gst_debug_cat); */
/* #define GST_CAT_DEFAULT _totem_gst_debug_cat */

static void feed_fakesrc(GstElement *src, GstBuffer *buf, GstPad *pad,
			 gpointer data)
{
	GstBuffer *in_buf = GST_BUFFER(data);

	g_assert(GST_BUFFER_SIZE(buf) >= GST_BUFFER_SIZE(in_buf));
	g_assert(!GST_BUFFER_FLAG_IS_SET(buf, GST_BUFFER_FLAG_READONLY));

	gst_buffer_set_caps(buf, GST_BUFFER_CAPS(in_buf));

	memcpy(GST_BUFFER_DATA(buf), GST_BUFFER_DATA(in_buf),
	       GST_BUFFER_SIZE(in_buf));

	GST_BUFFER_SIZE(buf) = GST_BUFFER_SIZE(in_buf);

	GST_DEBUG("feeding buffer %p, size %u, caps %" GST_PTR_FORMAT,
		  buf, GST_BUFFER_SIZE(buf), GST_BUFFER_CAPS(buf));

	gst_buffer_unref(in_buf);
}

static void save_result(GstElement *sink, GstBuffer *buf, GstPad *pad,
			gpointer data)
{

	result = gst_buffer_ref(buf);

	GST_DEBUG("received converted buffer %p with caps %" GST_PTR_FORMAT,
		  result, GST_BUFFER_CAPS(result));
}

static gboolean create_element(const gchar *factory_name, GstElement **element,
			       GError **err)
{
	*element = gst_element_factory_make(factory_name, NULL);
	if (*element)
		return TRUE;

	if (err && *err == NULL) {
		*err = g_error_new(
			GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN,
			"cannot create element '%s' - please check your "
			"GStreamer installation", factory_name);
	}

	return FALSE;
}

static gboolean finalize_process(void)
{
	g_signal_handlers_disconnect_matched(sink, (GSignalMatchType)
					     G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
					     save_result, NULL);
	g_signal_handlers_disconnect_matched(src, (GSignalMatchType)
					     G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
					     feed_fakesrc, NULL);
	gst_element_set_state(pipeline, GST_STATE_NULL);

	return FALSE;
}

static void _destroy_pixbuf (guchar *pixbuf, gpointer data)
{
	gst_buffer_unref(GST_BUFFER(data));
}

static GdkPixbuf *_convert_gstbuf_to_pixbuf(GstBuffer *new_buffer)
{
	gint width, height;
	GstStructure *structure;
	GdkPixbuf *pixbuf = NULL;

	structure =
		gst_caps_get_structure(GST_BUFFER_CAPS(new_buffer), 0);

	gst_structure_get_int(structure, "width", &width);
	gst_structure_get_int(structure, "height", &height);

	pixbuf = gdk_pixbuf_new_from_data(
		GST_BUFFER_DATA(new_buffer), GDK_COLORSPACE_RGB,
		FALSE, 8, width, height,
		GST_ROUND_UP_4(3 * width), _destroy_pixbuf,
		new_buffer);
	return pixbuf;
}

static gboolean async_bus_handler(GstBus *bus, GstMessage *msg,
				  gpointer data)
{
	gboolean keep_watch = TRUE;

	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_EOS: {
		GdkPixbuf *pbuf = NULL;
		if (result != NULL) {
			GST_DEBUG("conversion successful: result = %p",
				  result);
			pbuf = _convert_gstbuf_to_pixbuf(result);
		} else {
			GST_WARNING("EOS but no result frame?!");
		}
		mafw_gst_renderer_worker_pbuf_handler(current_worker, pbuf, metadata_key);
		keep_watch = finalize_process();
		break;
	}
	case GST_MESSAGE_ERROR: {
		gchar *dbg = NULL;
		GError *error = NULL;

		gst_message_parse_error(msg, &error, &dbg);
		if (error != NULL) {
			g_warning("Could not take screenshot: %s",
				  error->message);
			GST_DEBUG("%s [debug: %s]", error->message,
				  GST_STR_NULL(dbg));
			g_error_free(error);
		} else {
			g_warning("Could not take screenshot(and "
				  "NULL error!)");
		}
		g_free(dbg);
		result = NULL;
		mafw_gst_renderer_worker_pbuf_handler(current_worker, NULL,
							metadata_key);
		keep_watch = finalize_process();
		break;
	}
	default:
		break;
	}

	return keep_watch;
}

/* takes ownership of the input buffer */
gboolean bvw_frame_conv_convert(MafwGstRendererWorker *worker,
				GstBuffer *buf, GstCaps *to_caps,
				const gchar *mdata_key)
{
	static GstElement *filter1 = NULL, *filter2 = NULL;
	static GstBus *bus;
	GError *error = NULL;
	GstCaps *to_caps_no_par;

	g_return_val_if_fail(GST_BUFFER_CAPS(buf) != NULL, FALSE);

	current_worker = worker;
	if (pipeline == NULL) {
		GstElement *csp, *vscale;

		pipeline = gst_pipeline_new("screenshot-pipeline");
		if(pipeline == NULL) {
			g_warning("Could not take screenshot: "
				  "no pipeline (unknown error)");
			return FALSE;
		}

		/* videoscale is here to correct for the
		 * pixel-aspect-ratio for us */
		GST_DEBUG("creating elements");
		if(!create_element("fakesrc", &src, &error) ||
		   !create_element("ffmpegcolorspace", &csp, &error) ||
		   !create_element("videoscale", &vscale, &error) ||
		   !create_element("capsfilter", &filter1, &error) ||
		   !create_element("capsfilter", &filter2, &error) ||
		   !create_element("fakesink", &sink, &error)) {
			g_warning("Could not take screenshot: %s",
				  error->message);
			g_error_free(error);
			return FALSE;
		}

		GST_DEBUG("adding elements");
		gst_bin_add_many(GST_BIN(pipeline), src, csp, filter1, vscale,
				 filter2, sink, NULL);

		g_object_set(sink, "preroll-queue-len", 1,
			     "signal-handoffs", TRUE, NULL);

		/* set to 'fixed' sizetype */
		g_object_set(src, "sizetype", 2, "num-buffers", 1,
			     "signal-handoffs", TRUE, NULL);

		/* FIXME: linking is still way too expensive, profile
		 * this properly */
		GST_DEBUG("linking src->csp");
		if(!gst_element_link_pads(src, "src", csp, "sink"))
			return FALSE;

		GST_DEBUG("linking csp->filter1");
		if(!gst_element_link_pads(csp, "src", filter1, "sink"))
			return FALSE;

		GST_DEBUG("linking filter1->vscale");
		if(!gst_element_link_pads(filter1, "src", vscale, "sink"))
			return FALSE;

		GST_DEBUG("linking vscale->capsfilter");
		if(!gst_element_link_pads(vscale, "src", filter2, "sink"))
			return FALSE;

		GST_DEBUG("linking capsfilter->sink");
		if(!gst_element_link_pads(filter2, "src", sink, "sink"))
			return FALSE;

		bus = gst_element_get_bus(pipeline);
	}

	/* adding this superfluous capsfilter makes linking cheaper */
	to_caps_no_par = gst_caps_copy(to_caps);
	gst_structure_remove_field(gst_caps_get_structure(to_caps_no_par, 0),
				   "pixel-aspect-ratio");
	g_object_set(filter1, "caps", to_caps_no_par, NULL);
	gst_caps_unref(to_caps_no_par);

	g_object_set(filter2, "caps", to_caps, NULL);
	gst_caps_unref(to_caps);

	metadata_key = mdata_key;

	g_signal_connect(sink, "handoff", G_CALLBACK(save_result), NULL);

	g_signal_connect(src, "handoff", G_CALLBACK(feed_fakesrc), buf);

	gst_bus_add_watch(bus, async_bus_handler, NULL);

	/* set to 'fixed' sizetype */
	g_object_set(src, "sizemax", GST_BUFFER_SIZE(buf), NULL);

	GST_DEBUG("running conversion pipeline");
	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	return TRUE;
}
