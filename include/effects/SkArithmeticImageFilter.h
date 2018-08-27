/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArithmeticImageFilter_DEFINED
#define SkArithmeticImageFilter_DEFINED

#include "SkImageFilter.h"

extern const char* SKSL_ARITHMETIC_SRC;

struct ArithmeticFPInputs {
    float k[4];
    bool enforcePMColor;
};

class SK_API SkArithmeticImageFilter {
public:
    static sk_sp<SkImageFilter> Make(float k1, float k2, float k3, float k4, bool enforcePMColor,
                                     sk_sp<SkImageFilter> background,
                                     sk_sp<SkImageFilter> foreground,
                                     const SkImageFilter::CropRect* cropRect);

    static void InitializeFlattenables();

private:
    SkArithmeticImageFilter();  // can't instantiate
};

#endif
