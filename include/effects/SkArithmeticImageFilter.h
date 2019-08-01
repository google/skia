/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArithmeticImageFilter_DEFINED
#define SkArithmeticImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

struct ArithmeticFPInputs {
    ArithmeticFPInputs() {
        memset(this, 0, sizeof(*this));
    }

    float k[4];
    bool enforcePMColor;
};

// DEPRECATED: Use include/effects/SkImageFilters::Arithmetic
class SK_API SkArithmeticImageFilter {
public:
    static sk_sp<SkImageFilter> Make(float k1, float k2, float k3, float k4, bool enforcePMColor,
                                     sk_sp<SkImageFilter> background,
                                     sk_sp<SkImageFilter> foreground,
                                     const SkImageFilter::CropRect* cropRect);

    static void RegisterFlattenables();

private:
    SkArithmeticImageFilter();  // can't instantiate
};

#endif
