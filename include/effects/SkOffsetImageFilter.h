/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOffsetImageFilter_DEFINED
#define SkOffsetImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

class SK_API SkOffsetImageFilter {
public:
    static sk_sp<SkImageFilter> Make(SkScalar dx, SkScalar dy,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    static void RegisterFlattenables();

private:
    SkOffsetImageFilter() = delete;
};

#endif
