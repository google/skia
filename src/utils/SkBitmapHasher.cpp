/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapHasher.h"
#include "SkEndian.h"
#include "SkImageEncoder.h"

#ifdef BITMAPHASHER_USES_TRUNCATED_MD5
#include "SkMD5.h"
#else
#include "SkCityHash.h"
#include "SkStream.h"
#endif

/**
 * Write an int32 value to a stream in little-endian order.
 */
static void write_int32_to_buffer(uint32_t val, SkWStream* out) {
    val = SkEndian_SwapLE32(val);
    for (size_t byte = 0; byte < 4; ++byte) {
        out->write8((uint8_t)(val & 0xff));
        val = val >> 8;
    }
}

/**
 * Return the first 8 bytes of a bytearray, encoded as a little-endian uint64.
 */
static inline uint64_t first_8_bytes_as_uint64(const uint8_t *bytearray) {
    return SkEndian_SwapLE64(*(reinterpret_cast<const uint64_t *>(bytearray)));
}

/*static*/ bool SkBitmapHasher::ComputeDigestInternal(const SkBitmap& bitmap,
                                                      SkHashDigest *result) {
#ifdef BITMAPHASHER_USES_TRUNCATED_MD5
    SkMD5 out;
#else
    size_t pixelBufferSize = bitmap.width() * bitmap.height() * 4;
    size_t totalBufferSize = pixelBufferSize + 2 * sizeof(uint32_t);

    SkAutoMalloc bufferManager(totalBufferSize);
    char *bufferStart = static_cast<char *>(bufferManager.get());
    SkMemoryWStream out(bufferStart, totalBufferSize);
#endif

    // start with the x/y dimensions
    write_int32_to_buffer(SkToU32(bitmap.width()), &out);
    write_int32_to_buffer(SkToU32(bitmap.height()), &out);

    // add all the pixel data
    SkAutoTDelete<SkImageEncoder> enc(CreateARGBImageEncoder());
    if (!enc->encodeStream(&out, bitmap, SkImageEncoder::kDefaultQuality)) {
        return false;
    }

#ifdef BITMAPHASHER_USES_TRUNCATED_MD5
    SkMD5::Digest digest;
    out.finish(digest);
    *result = first_8_bytes_as_uint64(digest.data);
#else
    *result = SkCityHash::Compute64(bufferStart, totalBufferSize);
#endif
    return true;
}

/*static*/ bool SkBitmapHasher::ComputeDigest(const SkBitmap& bitmap, SkHashDigest *result) {
    if (ComputeDigestInternal(bitmap, result)) {
        return true;
    }

    // Hmm, that didn't work. Maybe if we create a new
    // kARGB_8888_Config version of the bitmap it will work better?
    SkBitmap copyBitmap;
    if (!bitmap.copyTo(&copyBitmap, SkBitmap::kARGB_8888_Config)) {
        return false;
    }
    return ComputeDigestInternal(copyBitmap, result);
}
