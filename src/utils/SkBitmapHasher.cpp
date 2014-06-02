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

#include "SkMD5.h"

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

/*static*/ bool SkBitmapHasher::ComputeDigestInternal(const SkBitmap& bitmap, uint64_t *result) {
    SkMD5 out;

    // start with the x/y dimensions
    write_int32_to_buffer(SkToU32(bitmap.width()), &out);
    write_int32_to_buffer(SkToU32(bitmap.height()), &out);

    // add all the pixel data
    SkAutoTDelete<SkImageEncoder> enc(CreateARGBImageEncoder());
    if (!enc->encodeStream(&out, bitmap, SkImageEncoder::kDefaultQuality)) {
        return false;
    }

    SkMD5::Digest digest;
    out.finish(digest);
    *result = first_8_bytes_as_uint64(digest.data);
    return true;
}

/*static*/ bool SkBitmapHasher::ComputeDigest(const SkBitmap& bitmap, uint64_t *result) {
    if (ComputeDigestInternal(bitmap, result)) {
        return true;
    }

    // Hmm, that didn't work. Maybe if we create a new
    // version of the bitmap it will work better?
    SkBitmap copyBitmap;
    if (!bitmap.copyTo(&copyBitmap, kN32_SkColorType)) {
        return false;
    }
    return ComputeDigestInternal(copyBitmap, result);
}
