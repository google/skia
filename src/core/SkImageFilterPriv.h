/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterPriv_DEFINED
#define SkImageFilterPriv_DEFINED

#include "include/core/SkImageFilter.h"

/**
 *  Helper to unflatten the common data, and return nullptr if we fail.
 */
#define SK_IMAGEFILTER_UNFLATTEN_COMMON(localVar, expectedCount)    \
    Common localVar;                                                \
    do {                                                            \
        if (!localVar.unflatten(buffer, expectedCount)) {           \
            return nullptr;                                         \
        }                                                           \
    } while (0)

/**
 * Return an image filter representing this filter applied with the given ctm. This will modify the
 * DAG as needed if this filter does not support complex CTMs and 'ctm' is not simple. The ctm
 * matrix will be decomposed such that ctm = A*B; B will be incorporated directly into the DAG and A
 * must be the ctm set on the context passed to filterImage(). 'remainder' will be set to A.
 *
 * If this filter supports complex ctms, or 'ctm' is not complex, then A = ctm and B = I. When the
 * filter does not support complex ctms, and the ctm is complex, then A represents the extracted
 * simple portion of the ctm, and the complex portion is baked into a new DAG using a matrix filter.
 *
 * This will never return null.
 */
sk_sp<SkImageFilter> SkApplyCTMToFilter(const SkImageFilter* filter, const SkMatrix& ctm,
                                        SkMatrix* remainder);

/**
 * Similar to SkApplyCTMToFilter except this assumes the input content is an existing backdrop image
 * to be filtered. As such,  the input to this filter will also be transformed by B^-1 if the filter
 * can't support complex CTMs, since backdrop content is already in device space and must be
 * transformed back into the CTM's local space.
 */
sk_sp<SkImageFilter> SkApplyCTMToBackdropFilter(const SkImageFilter* filter, const SkMatrix& ctm,
                                                SkMatrix* remainder);

bool SkIsSameFilter(const SkImageFilter* a, const SkImageFilter* b);

// Exposes just the behavior of the protected SkImageFilter::onFilterNodeBounds()
SkIRect SkFilterNodeBounds(const SkImageFilter* filter, const SkIRect& srcRect, const SkMatrix& ctm,
                           SkImageFilter::MapDirection dir, const SkIRect* inputRect);

#endif
