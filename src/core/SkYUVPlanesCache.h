/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVPlanesCache_DEFINED
#define SkYUVPlanesCache_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "src/core/SkCachedData.h"

class SkResourceCache;

class SkYUVPlanesCache {
public:
    /**
     * The Info struct contains data about the 4 Y, U, V, and A planes of memory stored
     * contiguously, in that order, as a single block of memory within SkYUVPlanesCache.
     *
     * fSizeInfo: fWidth, fHeight, and fWidthBytes of each of the Y, U, V, and A planes.
     * fColorSpace: color space that will be used for the YUV -> RGB conversion.
     */
    struct Info {
        SkYUVASizeInfo  fSizeInfo;
        SkYUVAIndex     fYUVAIndices[SkYUVAIndex::kIndexCount];
        SkYUVColorSpace fColorSpace;
    };
    /**
     * On success, return a ref to the SkCachedData that holds the pixels.
     *
     * On failure, return nullptr.
     */
    static SkCachedData* FindAndRef(uint32_t genID, Info* info,
                                    SkResourceCache* localCache = nullptr);

    /**
     * Add a pixelRef ID and its YUV planes data to the cache.
     */
    static void Add(uint32_t genID, SkCachedData* data, Info* info,
                    SkResourceCache* localCache = nullptr);
};

#endif
