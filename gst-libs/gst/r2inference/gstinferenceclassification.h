/* Copyright (C) 2019 RidgeRun, LLC (http://www.ridgerun.com)
 * All Rights Reserved.
 *
 * The contents of this software are proprietary and confidential to RidgeRun,
 * LLC.  No part of this program may be photocopied, reproduced or translated
 * into another programming language without prior written consent of
 * RidgeRun, LLC.  The user is free to modify the source code after obtaining
 * a software license from RidgeRun.  All source code changes must be provided
 * back to RidgeRun without any encumbrance.
*/

#ifndef __GST_INFERENCE_CLASSIFICATION__
#define __GST_INFERENCE_CLASSIFICATION__

#include <gst/gst.h>

G_BEGIN_DECLS

/**
 * GstInferenceClassification:
 * @classification_id: a unique id associated to this classification
 * @class_id: the numerical id associated to the assigned class
 * @class_prob: the resulting probability of the assigned
 * class. Typically between 0 and 1
 * @class_label: the label associated to this class or NULL if not
 * available
 * @num_classes: the amount of classes of the entire prediction
 * @probabilities: the entire array of probabilities of the prediction
 * @labels: the entire array of labels of the prediction or NULL if
 * not available
 */
typedef struct _GstInferenceClassification GstInferenceClassification;
struct _GstInferenceClassification
{
  /*<private>*/
  GstMiniObject base;
  GMutex mutex;

  /*<public>*/
  guint64 classification_id;
  gint class_id;
  gdouble class_prob;
  gchar *class_label;
  gint num_classes;
  gdouble *probabilities;
  gchar **labels;
};

/**
 * gst_inference_classification_new:
 * 
 * Creates a new GstInferenceClassification.
 *
 * Returns: A newly allocated and initialized GstInferenceClassification.
 */
GstInferenceClassification * gst_inference_classification_new (void);

/**
 * gst_inference_classification_new_full:
 * @class_id: the numerical id associated to the assigned class
 * @class_prob: the resulting probability of the assigned
 * class. Typically between 0 and 1
 * @class_label: the label associated to this class or NULL if not
 * available. A copy of the label is made if available.
 * @num_classes: the amount of classes of the entire prediction
 * @probabilities: the entire array of probabilities of the
 * prediction. A copy of the array is made.
 * @labels: the entire array of labels of the prediction or NULL if
 * not available. A copy is made, if available.
 * 
 * Creates a new GstInferenceClassification and assigns its members.
 *
 * Returns: A newly allocated and initialized GstInferenceClassification.
 */
GstInferenceClassification * gst_inference_classification_new_full (gint class_id, gdouble class_prob,
    const gchar * class_label, gint num_classes, const gdouble * probabilities,
    gchar ** labels);

/**
 * gst_inference_classification_reset:
 * @self: the classification to reset 
 *
 * Clears a classification, effectively freeing all associated memory.
 */
void gst_inference_classification_reset (GstInferenceClassification * self);

/**
 * gst_inference_classification_copy:
 * @self: the classification to copy
 *
 * Copies a classification into a newly allocated one. This is a deep
 * copy, meaning that all arrays are copied as well. No pointers are
 * shared.
 *
 * Returns: a newly allocated copy of the original classification
 */
GstInferenceClassification * gst_inference_classification_copy (const GstInferenceClassification * self);

/**
 * gst_inference_classification_ref:
 * @self: the classification to ref
 *
 * Increase the reference counter of the classification.
 *
 * Returns: the same classification, for convenience purposes.
 */
GstInferenceClassification * gst_inference_classification_ref (GstInferenceClassification * self);

/**
 * gst_inference_classification_unref:
 * @self: the classification to unref
 *
 * Decreases the reference counter of the classification. When the
 * reference counter hits zero, the classification is freed.
 */
void gst_inference_classification_unref (GstInferenceClassification * self);

/**
 * gst_inference_classification_to_string:
 * @self: the classification to serialize
 *
 * Serializes the classification into a JSON-like string. The full
 * arrays are not included. Free this string after usage using
 * g_free()
 *
 * Returns: a string representing the classification.
 */
gchar * gst_inference_classification_to_string (GstInferenceClassification * self, gint level);

/**
 * GST_INFERENCE_CLASSIFICATION_LOCK:
 * @c: The GstInferenceClassification to lock
 *
 * Locks the classification to avoid concurrent access from different
 * threads.
 */
#define GST_INFERENCE_CLASSIFICATION_LOCK(c) g_mutex_lock (&((c)->mutex))

/**
 * GST_INFERENCE_CLASSIFICATION_UNLOCK:
 * @c: The GstInferenceClassification to unlock
 *
 * Unlocks the prediction to yield the access to other threads.
 */
#define GST_INFERENCE_CLASSIFICATION_UNLOCK(c) g_mutex_unlock (&((c)->mutex))

G_END_DECLS

#endif // __GST_INFERENCE_CLASSIFICATION__
