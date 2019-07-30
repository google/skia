/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterImageFilter_DEFINED
#define SkColorFilterImageFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"

class SK_API SkColorFilterImageFilter {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkColorFilter> cf,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    static void RegisterFlattenables();

private:
    SkColorFilterImageFilter() = delete;
};

#endif
