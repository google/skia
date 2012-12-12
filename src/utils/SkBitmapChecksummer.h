
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapChecksummer_DEFINED
#define SkBitmapChecksummer_DEFINED

#include "SkBitmap.h"
#include "SkBitmapTransformer.h"

/**
 * Static class that can generate checksums from SkBitmaps.
 */
class SkBitmapChecksummer {
public:
    /**
     * Returns a 64-bit checksum of the pixels in this bitmap.
     *
     * If this is unable to compute the checksum for some reason,
     * it returns 0.
     *
     * Note: depending on the bitmap config, we may need to create an
     * intermediate SkBitmap and copy the pixels over to it... so in some
     * cases, performance and memory usage can suffer.
     */
    static uint64_t Compute64(const SkBitmap& bitmap);

private:
    static uint64_t Compute64Internal(const SkBitmap& bitmap,
                                      const SkBitmapTransformer& transformer);
};

#endif
