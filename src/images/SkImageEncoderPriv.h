/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderPriv_DEFINED
#define SkImageEncoderPriv_DEFINED

#include "include/core/SkImageEncoder.h"
#include "include/private/SkImageInfoPriv.h"

static inline bool SkPixmapIsValid(const SkPixmap& src) {
    if (!SkImageInfoIsValid(src.info())) {
        return false;
    }

    if (!src.addr() || src.rowBytes() < src.info().minRowBytes()) {
        return false;
    }

    return true;
}

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    bool SkEncodeImageWithCG(SkWStream*, const SkPixmap&, SkEncodedImageFormat);
#else
    #define SkEncodeImageWithCG(...) false
#endif

#ifdef SK_BUILD_FOR_WIN
    bool SkEncodeImageWithWIC(SkWStream*, const SkPixmap&, SkEncodedImageFormat, int quality);
#else
    #define SkEncodeImageWithWIC(...) false
#endif

#endif // SkImageEncoderPriv_DEFINED
