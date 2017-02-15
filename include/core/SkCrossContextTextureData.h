/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkCrossContextTextureData_DEFINED
#define SkCrossContextTextureData_DEFINED

#include "SkImage.h"

struct SK_API SkCrossContextTextureData : SkNoncopyable {
    SkCrossContextTextureData(sk_sp<SkImage> image) : fImage(std::move(image)) {
        // Texture backed images need to be stripped down and constructed from their backend data
        SkASSERT(!fImage->isTextureBacked());
    }

    // For non-GPU backed images
    sk_sp<SkImage> fImage;
};

#endif
