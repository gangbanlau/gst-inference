/*
 * GStreamer
 * Copyright (C) 2019 RidgeRun
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
 *
 */

/**
 * SECTION:element-gstmobilenetv2
 *
 * The mobilenetv2 element allows the user to infer/execute a pretrained model
 * based on the mobilenetv2 architectures on incoming image frames.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v videotestsrc ! mobilenetv2 ! xvimagesink
 * ]|
 * Process video frames from the camera using a mobilenetv2 model.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstmobilenetv2.h"
#include "gst/r2inference/gstinferencemeta.h"
#include <string.h>
#include "gst/r2inference/gstinferencepreprocess.h"
#include "gst/r2inference/gstinferencepostprocess.h"

GST_DEBUG_CATEGORY_STATIC (gst_mobilenetv2_debug_category);
#define GST_CAT_DEFAULT gst_mobilenetv2_debug_category

#define MEAN 128.0
#define STD 1/128.0
#define MODEL_CHANNELS 3

static gboolean gst_mobilenetv2_preprocess (GstVideoInference * vi,
    GstVideoFrame * inframe, GstVideoFrame * outframe);
static gboolean gst_mobilenetv2_postprocess (GstVideoInference * vi,
    const gpointer prediction, gsize predsize, GstMeta * meta_model,
    GstVideoInfo * info_model, gboolean * valid_prediction);
static gboolean gst_mobilenetv2_start (GstVideoInference * vi);
static gboolean gst_mobilenetv2_stop (GstVideoInference * vi);

enum
{
  PROP_0
};

/* pad templates */
#define CAPS								\
  "video/x-raw, "							\
  "width=224, "							\
  "height=224, "							\
  "format={RGB, RGBx, RGBA, BGR, BGRx, BGRA, xRGB, ARGB, xBGR, ABGR}"

static GstStaticPadTemplate sink_model_factory =
GST_STATIC_PAD_TEMPLATE ("sink_model",
    GST_PAD_SINK,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS (CAPS)
    );

static GstStaticPadTemplate src_model_factory =
GST_STATIC_PAD_TEMPLATE ("src_model",
    GST_PAD_SRC,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS (CAPS)
    );

struct _GstMobilenetv2
{
  GstVideoInference parent;
};

struct _GstMobilenetv2Class
{
  GstVideoInferenceClass parent;
};

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstMobilenetv2, gst_mobilenetv2,
    GST_TYPE_VIDEO_INFERENCE,
    GST_DEBUG_CATEGORY_INIT (gst_mobilenetv2_debug_category, "mobilenetv2", 0,
        "debug category for mobilenetv2 element"));

static void
gst_mobilenetv2_class_init (GstMobilenetv2Class * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoInferenceClass *vi_class = GST_VIDEO_INFERENCE_CLASS (klass);

  gst_element_class_add_static_pad_template (element_class,
      &sink_model_factory);
  gst_element_class_add_static_pad_template (element_class, &src_model_factory);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
      "mobilenetv2", "Filter",
      "Infers incoming image frames using a pretrained GoogLeNet mobilenetv2 model",
      "Carlos Rodriguez <carlos.rodriguez@ridgerun.com> \n\t\t\t"
      "   Jose Jimenez <jose.jimenez@ridgerun.com> \n\t\t\t"
      "   Michael Gruner <michael.gruner@ridgerun.com>  \n\t\t\t"
      "   Mauricio Montero <mauricio.montero@ridgerun.com>");

  vi_class->start = GST_DEBUG_FUNCPTR (gst_mobilenetv2_start);
  vi_class->stop = GST_DEBUG_FUNCPTR (gst_mobilenetv2_stop);
  vi_class->preprocess = GST_DEBUG_FUNCPTR (gst_mobilenetv2_preprocess);
  vi_class->postprocess = GST_DEBUG_FUNCPTR (gst_mobilenetv2_postprocess);
  vi_class->inference_meta_info = gst_classification_meta_get_info ();
}

static void
gst_mobilenetv2_init (GstMobilenetv2 * mobilenetv2)
{
}

static gboolean
gst_mobilenetv2_preprocess (GstVideoInference * vi,
    GstVideoFrame * inframe, GstVideoFrame * outframe)
{
  GST_LOG_OBJECT (vi, "Preprocess");
  return gst_normalize (inframe, outframe, MEAN, STD, MODEL_CHANNELS);
}

static gboolean
gst_mobilenetv2_postprocess (GstVideoInference * vi, const gpointer prediction,
    gsize predsize, GstMeta * meta_model, GstVideoInfo * info_model,
    gboolean * valid_prediction)
{
  GstClassificationMeta *class_meta = (GstClassificationMeta *) meta_model;
  gint index;
  gdouble max;
  GstDebugLevel level;
  GST_LOG_OBJECT (vi, "Postprocess");

  gst_fill_classification_meta (class_meta, prediction, predsize);

  /* Only compute the highest probability is label when debug >= 6 */
  level = gst_debug_category_get_threshold (gst_mobilenetv2_debug_category);
  if (level >= GST_LEVEL_LOG) {
    index = 0;
    max = -1;
    for (gint i = 0; i < class_meta->num_labels; ++i) {
      gfloat current = ((gfloat *) prediction)[i];
      if (current > max) {
        max = current;
        index = i;
      }
    }
    GST_LOG_OBJECT (vi, "Highest probability is label %i : (%f)", index, max);
  }

  *valid_prediction = TRUE;
  return TRUE;
}

static gboolean
gst_mobilenetv2_start (GstVideoInference * vi)
{
  GST_INFO_OBJECT (vi, "Starting Mobilenet v2");

  return TRUE;
}

static gboolean
gst_mobilenetv2_stop (GstVideoInference * vi)
{
  GST_INFO_OBJECT (vi, "Stopping Mobilenet v2");

  return TRUE;
}