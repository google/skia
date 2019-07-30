/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureImageFilter_DEFINED
#define SkPictureImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

class SkPicture;

class SK_API SkPictureImageFilter {
public:
    /**
     *  Refs the passed-in picture.
     */
    static sk_sp<SkImageFilter> Make(sk_sp<SkPicture> picture);

    /**
     *  Refs the passed-in picture. cropRect can be used to crop or expand the destination rect when
     *  the picture is drawn. (No scaling is implied by the dest rect; only the CTM is applied.)
     */
    static sk_sp<SkImageFilter> Make(sk_sp<SkPicture> picture, const SkRect& cropRect);

    static void RegisterFlattenables();

private:
    SkPictureImageFilter() = delete;
};

#endif
