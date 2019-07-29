/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMergeImageFilter_DEFINED
#define SkMergeImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

class SK_API SkMergeImageFilter {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter>* const filters, int count,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> first, sk_sp<SkImageFilter> second,
                                     const SkImageFilter::CropRect* cropRect = nullptr) {
        sk_sp<SkImageFilter> array[] = {
            std::move(first),
            std::move(second),
        };
        return Make(array, 2, cropRect);
    }

    static void RegisterFlattenables();

private:
    SkMergeImageFilter() = delete;
};

#endif
