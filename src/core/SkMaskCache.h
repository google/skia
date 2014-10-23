/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskCache_DEFINED
#define SkMaskCache_DEFINED

#include "SkBlurTypes.h"
#include "SkCachedData.h"
#include "SkMask.h"
#include "SkRect.h"
#include "SkResourceCache.h"
#include "SkRRect.h"

class SkMaskCache {
public:
    /**
     * On success, return a ref to the SkCachedData that holds the pixels, and have mask
     * already point to that memory.
     *
     * On failure, return NULL.
     */
    static SkCachedData* FindAndRef(SkScalar sigma, const SkRRect& rrect, SkBlurStyle style,
                                    SkBlurQuality quality, SkMask* mask,
                                    SkResourceCache* localCache = NULL);
    static SkCachedData* FindAndRef(SkScalar sigma, const SkRect rects[], int count,
                                    SkBlurStyle style,SkMask* mask,
                                    SkResourceCache* localCache = NULL);

    /**
     * Add a mask and its pixel-data to the cache.
     */
    static void Add(SkScalar sigma, const SkRRect& rrect, SkBlurStyle style, SkBlurQuality quality,
                    const SkMask& mask, SkCachedData* data, SkResourceCache* localCache = NULL);
    static void Add(SkScalar sigma, const SkRect rects[], int count, SkBlurStyle style,
                    const SkMask& mask, SkCachedData* data, SkResourceCache* localCache = NULL);
};

#endif
