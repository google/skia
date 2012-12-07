/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * Hash functions, using the CityHash algorithm.
 *
 * Results are guaranteed to be:
 *  1. consistent across revisions of the library (for a given set
 *     of bytes, the checksum generated at one revision of the Skia
 *     library will match the one generated on any other revision of
 *     the Skia library)
 *  2. consistent across platforms (for a given
 *     set of bytes, the checksum generated on one platform will
 *     match the one generated on any other platform)
 */

#ifndef SkCityHash_DEFINED
#define SkCityHash_DEFINED

#include "SkTypes.h"

class SkCityHash : SkNoncopyable {
public:
    /**
     *  Compute a 32-bit checksum for a given data block.
     *
     *  @param data Memory address of the data block to be processed.
     *  @param size Size of the data block in bytes.
     *  @return checksum result
     */
    static uint32_t Compute32(const char *data, size_t size);

    /**
     *  Compute a 64-bit checksum for a given data block.
     *
     *  @param data Memory address of the data block to be processed.
     *  @param size Size of the data block in bytes.
     *  @return checksum result
     */
    static uint64_t Compute64(const char *data, size_t size);
};

#endif
