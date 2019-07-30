/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMagnifierImageFilter_DEFINED
#define SkMagnifierImageFilter_DEFINED

#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"

class SK_API SkMagnifierImageFilter {
public:
    static sk_sp<SkImageFilter> Make(const SkRect& srcRect, SkScalar inset,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    static void RegisterFlattenables();

private:
    SkMagnifierImageFilter() = delete;
};

#endif
