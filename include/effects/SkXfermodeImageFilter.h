/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermodeImageFilter_DEFINED
#define SkXfermodeImageFilter_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkImageFilter.h"
#include "include/effects/SkArithmeticImageFilter.h"

/**
 * This filter takes a SkBlendMode, and uses it to composite the foreground over the background.
 * If foreground or background is NULL, the input bitmap (src) is used instead.
 */
class SK_API SkXfermodeImageFilter {
public:
    static sk_sp<SkImageFilter> Make(SkBlendMode, sk_sp<SkImageFilter> background,
                                     sk_sp<SkImageFilter> foreground,
                                     const SkImageFilter::CropRect* cropRect);
    static sk_sp<SkImageFilter> Make(SkBlendMode mode, sk_sp<SkImageFilter> background) {
        return Make(mode, std::move(background), nullptr, nullptr);
    }

    static void RegisterFlattenables();

private:
    SkXfermodeImageFilter();    // can't instantiate
};

#endif
