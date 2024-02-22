/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVPlanesCache_DEFINED
#define SkYUVPlanesCache_DEFINED

#include "include/core/SkTypes.h"

#include <cstdint>

class SkCachedData;
class SkResourceCache;
class SkYUVAPixmaps;

class SkYUVPlanesCache {
public:
    /**
     * On success, return a ref to the SkCachedData that holds the pixel data. The SkYUVAPixmaps
     * contains a description of the YUVA data and has a SkPixmap for each plane that points
     * into the SkCachedData.
     *
     * On failure, return nullptr.
     */
    static SkCachedData* FindAndRef(uint32_t genID,
                                    SkYUVAPixmaps* pixmaps,
                                    SkResourceCache* localCache = nullptr);

    /**
     * Add a pixelRef ID and its YUV planes data to the cache. The SkYUVAPixmaps should contain
     * SkPixmaps that store their pixel data in the SkCachedData.
     */
    static void Add(uint32_t genID, SkCachedData* data, const SkYUVAPixmaps& pixmaps,
                    SkResourceCache* localCache = nullptr);
};

#endif
