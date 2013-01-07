
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvas.h"
#include "SkColorPriv.h"

/**
 * Converts pixels from one Config8888 to another Config8888
 */
void SkConvertConfig8888Pixels(uint32_t* dstPixels,
                               size_t dstRowBytes,
                               SkCanvas::Config8888 dstConfig,
                               const uint32_t* srcPixels,
                               size_t srcRowBytes,
                               SkCanvas::Config8888 srcConfig,
                               int width,
                               int height);

/**
 * Packs a, r, g, b, values into byte order specified by config.
 */
uint32_t SkPackConfig8888(SkCanvas::Config8888 config,
                          uint32_t a,
                          uint32_t r,
                          uint32_t g,
                          uint32_t b);

///////////////////////////////////////////////////////////////////////////////
// Implementation

namespace {

/**
  Copies all pixels from a bitmap to a dst ptr with a given rowBytes and
  Config8888. The bitmap must have kARGB_8888_Config.
 */

static inline void SkCopyBitmapToConfig8888(uint32_t* dstPixels,
                                     size_t dstRowBytes,
                                     SkCanvas::Config8888 dstConfig8888,
                                     const SkBitmap& srcBmp) {
    SkASSERT(SkBitmap::kARGB_8888_Config == srcBmp.config());
    SkAutoLockPixels alp(srcBmp);
    int w = srcBmp.width();
    int h = srcBmp.height();
    size_t srcRowBytes = srcBmp.rowBytes();
    const uint32_t* srcPixels = reinterpret_cast<uint32_t*>(srcBmp.getPixels());

    SkConvertConfig8888Pixels(dstPixels, dstRowBytes, dstConfig8888, srcPixels, srcRowBytes, SkCanvas::kNative_Premul_Config8888, w, h);
}

/**
  Copies over all pixels in a bitmap from a src ptr with a given rowBytes and
  Config8888. The bitmap must have pixels and be kARGB_8888_Config.
 */
static inline void SkCopyConfig8888ToBitmap(const SkBitmap& dstBmp,
                                     const uint32_t* srcPixels,
                                     size_t srcRowBytes,
                                     SkCanvas::Config8888 srcConfig8888) {
    SkASSERT(SkBitmap::kARGB_8888_Config == dstBmp.config());
    SkAutoLockPixels alp(dstBmp);
    int w = dstBmp.width();
    int h = dstBmp.height();
    size_t dstRowBytes = dstBmp.rowBytes();
    uint32_t* dstPixels = reinterpret_cast<uint32_t*>(dstBmp.getPixels());

    SkConvertConfig8888Pixels(dstPixels, dstRowBytes, SkCanvas::kNative_Premul_Config8888, srcPixels, srcRowBytes, srcConfig8888, w, h);
}

}
