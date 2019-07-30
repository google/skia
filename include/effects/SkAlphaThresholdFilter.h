/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAlphaThresholdFilter_DEFINED
#define SkAlphaThresholdFilter_DEFINED

#include "include/core/SkImageFilter.h"

class SkRegion;

class SK_API SkAlphaThresholdFilter {
public:
    /**
     * Creates an image filter that samples a region. If the sample is inside the
     * region the alpha of the image is boosted up to a threshold value. If it is
     * outside the region then the alpha is decreased to the threshold value.
     * The 0,0 point of the region corresponds to the upper left corner of the
     * source image.
     */
    static sk_sp<SkImageFilter> Make(const SkRegion& region, SkScalar innerMin,
                                     SkScalar outerMax, sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);


    static void RegisterFlattenables();

private:
    SkAlphaThresholdFilter() = delete;
};

#endif
