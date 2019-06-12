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
#include "gstinferencedebug.h"

void
gst_inference_print_embedding (GstVideoInference * vi,
    GstDebugCategory * category, GstClassificationMeta * class_meta,
    const gpointer prediction, GstDebugLevel gstlevel)
{
  GstDebugLevel level;

  g_return_if_fail (vi != NULL);
  g_return_if_fail (category != NULL);
  g_return_if_fail (class_meta != NULL);
  g_return_if_fail (prediction != NULL);

  /* Only display vector if debug level >= 6 */
  level = gst_debug_category_get_threshold (category);
  if (level >= gstlevel) {
    for (gint i = 0; i < class_meta->num_labels; ++i) {
      gfloat current = ((gfloat *) prediction)[i];
      GST_CAT_LEVEL_LOG (category, gstlevel, vi,
          "Output vector element %i : (%f)", i, current);
    }
  }
}
