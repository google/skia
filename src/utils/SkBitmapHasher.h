
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapHasher_DEFINED
#define SkBitmapHasher_DEFINED

#include "SkBitmap.h"

/**
 * Static class that generates a uint64 hash digest from an SkBitmap.
 */
class SkBitmapHasher {
public:
    /**
     * Fills in "result" with a hash of the pixels in this bitmap.
     *
     * If this is unable to compute the hash for some reason,
     * it returns false.
     *
     * Note: depending on the bitmap colortype, we may need to create an
     * intermediate SkBitmap and copy the pixels over to it... so in some
     * cases, performance and memory usage can suffer.
     */
    static bool ComputeDigest(const SkBitmap& bitmap, uint64_t *result);

private:
    static bool ComputeDigestInternal(const SkBitmap& bitmap, uint64_t *result);
};

#endif
