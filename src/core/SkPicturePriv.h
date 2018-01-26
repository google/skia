/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePriv_DEFINED
#define SkPicturePriv_DEFINED

#include "SkPicture.h"

class SkWriteBuffer;

class SkPicturePriv {
public:
    static size_t ApproxBytesUsed(const SkPicture* pic) {
        return pic->approximateBytesUsed();
    }
    static const SkBigPicture* AsBigPicture(const SkPicture* pic) {
        return pic->asSkBigPicture();
    }
    static void Flatten(SkWriteBuffer& buffer, const SkPicture* pic) {
        pic->flatten(buffer);
    }
};

#endif
