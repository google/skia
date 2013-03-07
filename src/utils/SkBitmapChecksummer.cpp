
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapChecksummer.h"
#include "SkBitmapTransformer.h"
#include "SkCityHash.h"
#include "SkEndian.h"

/**
 * Write an integer value into a bytebuffer in little-endian order.
 */
static void write_int_to_buffer(int val, char* buf) {
    val = SkEndian_SwapLE32(val);
    for (int byte=0; byte<4; byte++) {
        *buf++ = (char)(val & 0xff);
        val = val >> 8;
    }
}

/*static*/ uint64_t SkBitmapChecksummer::Compute64Internal(
        const SkBitmap& bitmap, const SkBitmapTransformer& transformer) {
    size_t pixelBufferSize = transformer.bytesNeededTotal();
    size_t totalBufferSize = pixelBufferSize + 8; // leave room for x/y dimensions

    SkAutoMalloc bufferManager(totalBufferSize);
    char *bufferStart = static_cast<char *>(bufferManager.get());
    char *bufPtr = bufferStart;
    // start with the x/y dimensions
    write_int_to_buffer(bitmap.width(), bufPtr);
    bufPtr += 4;
    write_int_to_buffer(bitmap.height(), bufPtr);
    bufPtr += 4;

    // add all the pixel data
    if (!transformer.copyBitmapToPixelBuffer(bufPtr, pixelBufferSize)) {
        return 0;
    }
    return SkCityHash::Compute64(bufferStart, totalBufferSize);
}

/*static*/ uint64_t SkBitmapChecksummer::Compute64(const SkBitmap& bitmap) {
    const SkBitmapTransformer::PixelFormat kPixelFormat =
        SkBitmapTransformer::kARGB_8888_Premul_PixelFormat;

    // First, try to transform the existing bitmap.
    const SkBitmapTransformer transformer =
        SkBitmapTransformer(bitmap, kPixelFormat);
    if (transformer.isValid(false)) {
        return Compute64Internal(bitmap, transformer);
    }

    // Hmm, that didn't work. Maybe if we create a new
    // kARGB_8888_Config version of the bitmap it will work better?
    SkBitmap copyBitmap;
    bitmap.copyTo(&copyBitmap, SkBitmap::kARGB_8888_Config);
    const SkBitmapTransformer copyTransformer =
        SkBitmapTransformer(copyBitmap, kPixelFormat);
    if (copyTransformer.isValid(true)) {
        return Compute64Internal(copyBitmap, copyTransformer);
    } else {
        return 0;
    }
}
