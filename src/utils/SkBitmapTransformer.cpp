
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapTransformer.h"
#include "SkColorPriv.h"
#include "SkTypes.h"

bool SkBitmapTransformer::isValid(bool logReason) const {
    bool retval = true;

    switch(fPixelFormat) {
    case kARGB_8888_Premul_PixelFormat:
        break;
    default:
        if (logReason) {
            SkDEBUGF(("PixelFormat %d not supported\n", fPixelFormat));
        }
        retval = false;
    }

    SkBitmap::Config bitmapConfig = fBitmap.config();
    switch(bitmapConfig) {
    case SkBitmap::kARGB_8888_Config:
        break;
    default:
        if (logReason) {
            SkDEBUGF(("SkBitmap::Config %d not supported\n", bitmapConfig));
        }
        retval = false;
    }

    return retval;
}

/**
 * Transform from kARGB_8888_Config to kARGB_8888_Premul_PixelFormat.
 *
 * Similar to the various scanline transformers in
 * src/images/transform_scanline.h .
 */
static void transform_scanline(const char* SK_RESTRICT src, int width,
                               char* SK_RESTRICT dst) {
    const SkPMColor* SK_RESTRICT srcP = reinterpret_cast<const SkPMColor*>(src);
    for (int i = 0; i < width; i++) {
        SkPMColor c = *srcP++;
        unsigned a = SkGetPackedA32(c);
        unsigned r = SkGetPackedR32(c);
        unsigned g = SkGetPackedG32(c);
        unsigned b = SkGetPackedB32(c);
        *dst++ = a;
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
    }
}

bool SkBitmapTransformer::copyBitmapToPixelBuffer(void *dstBuffer,
                                                  size_t dstBufferSize) const {
    if (!this->isValid(true)) {
        return false;
    }
    size_t bytesNeeded = this->bytesNeededTotal();
    if (dstBufferSize < bytesNeeded) {
        SkDEBUGF(("dstBufferSize %d must be >= %d\n", dstBufferSize, bytesNeeded));
        return false;
    }

    fBitmap.lockPixels();
    int width = fBitmap.width();
    size_t srcRowBytes = fBitmap.rowBytes();
    size_t dstRowBytes = this->bytesNeededPerRow();
    const char *srcBytes = const_cast<const char *>(static_cast<char*>(fBitmap.getPixels()));
    char *dstBytes = static_cast<char *>(dstBuffer);
    for (int y = 0; y < fBitmap.height(); y++) {
        transform_scanline(srcBytes, width, dstBytes);
        srcBytes += srcRowBytes;
        dstBytes += dstRowBytes;
    }
    fBitmap.unlockPixels();
    return true;
}
