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
    ArithmeticFPInputs(float k0, float k1, float k2, float k3, bool enforcePMColor) {
        // We copy instances of this struct as the input data blob for the  SkSL FP. The FP
        // may try to access all of our bytes (for comparison purposes), so be sure to zero out
        // any padding after the dangling bool.
        memset(this, 0, sizeof(*this));
        fK[0] = k0;
        fK[1] = k1;
        fK[2] = k2;
        fK[3] = k3;
        fEnforcePMColor = enforcePMColor;
    }

    float fK[4];
    bool  fEnforcePMColor;
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
