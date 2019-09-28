/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageSource_DEFINED
#define SkImageSource_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"

// DEPRECATED: Use include/effects/SkImageFilters::Image
class SK_API SkImageSource {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImage> image);
    static sk_sp<SkImageFilter> Make(sk_sp<SkImage> image,
                                     const SkRect& srcRect,
                                     const SkRect& dstRect,
                                     SkFilterQuality filterQuality);

    static void RegisterFlattenables();

private:
    SkImageSource() = delete;
};

#endif
