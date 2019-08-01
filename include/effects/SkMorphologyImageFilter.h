/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMorphologyImageFilter_DEFINED
#define SkMorphologyImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

///////////////////////////////////////////////////////////////////////////////
// DEPRECATED: Use include/effects/SkImageFilters::Dilate
class SK_API SkDilateImageFilter {
public:
    static sk_sp<SkImageFilter> Make(int radiusX, int radiusY,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    // Registers all morphology filter implementations
    static void RegisterFlattenables();

private:
    SkDilateImageFilter() = delete;
};

///////////////////////////////////////////////////////////////////////////////
// DEPRECATED: Use include/effects/SkImageFilters::Erode
class SK_API SkErodeImageFilter {
public:
    static sk_sp<SkImageFilter> Make(int radiusX, int radiusY,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

private:
    SkErodeImageFilter() = delete;
};

#endif
