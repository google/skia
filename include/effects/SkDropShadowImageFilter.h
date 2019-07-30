/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDropShadowImageFilter_DEFINED
#define SkDropShadowImageFilter_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkScalar.h"

// DEPRECATED: Use include/effects/SkImageFilters::DropShadow and DropShadowOnly
class SK_API SkDropShadowImageFilter {
public:
    enum ShadowMode {
        kDrawShadowAndForeground_ShadowMode,
        kDrawShadowOnly_ShadowMode,

        kLast_ShadowMode = kDrawShadowOnly_ShadowMode
    };

    static const int kShadowModeCount = kLast_ShadowMode+1;

    static sk_sp<SkImageFilter> Make(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY,
                                     SkColor color, ShadowMode shadowMode,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    static void RegisterFlattenables();

private:
    SkDropShadowImageFilter() = delete;
};

#endif
