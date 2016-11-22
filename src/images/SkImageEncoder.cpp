/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"

bool SkEncodeImage(SkWStream* dst, const SkPixmap& src,
                   SkEncodedImageFormat format, int quality) {
    SkBitmap bm;
    if (!bm.installPixels(src)) {
        return false;
    }
    bm.setImmutable();
    std::unique_ptr<SkImageEncoder> enc(SkImageEncoder::Create((SkImageEncoder::Type)format));
    return enc && enc->encodeStream(dst, bm, quality);
}
