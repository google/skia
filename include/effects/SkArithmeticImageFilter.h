/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArithmeticImageFilter_DEFINED
#define SkArithmeticImageFilter_DEFINED

#include "SkImageFilter.h"

class SK_API SkArithmeticImageFilter {
public:
    static sk_sp<SkImageFilter> Make(float k1, float k2, float k3, float k4, bool enforcePMColor,
                                     sk_sp<SkImageFilter> background,
                                     sk_sp<SkImageFilter> foreground,
                                     const SkImageFilter::CropRect* cropRect);
    static sk_sp<SkImageFilter> Make(float k1, float k2, float k3, float k4, bool enforcePMColor,
                                     sk_sp<SkImageFilter> background) {
        return Make(k1, k2, k3, k4, enforcePMColor, std::move(background), nullptr, nullptr);
    }

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP();

private:
    SkArithmeticImageFilter();  // can't instantiate
};

#endif
