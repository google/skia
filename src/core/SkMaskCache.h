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
    static SkCachedData* FindAndRef(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                                    const SkRRect& rrect, SkMask* mask,
                                    SkResourceCache* localCache = NULL);
    static SkCachedData* FindAndRef(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                                    const SkRect rects[], int count, SkMask* mask,
                                    SkResourceCache* localCache = NULL);

    /**
     * Add a mask and its pixel-data to the cache.
     */
    static void Add(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                    const SkRRect& rrect, const SkMask& mask, SkCachedData* data,
                    SkResourceCache* localCache = NULL);
    static void Add(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                    const SkRect rects[], int count, const SkMask& mask, SkCachedData* data,
                    SkResourceCache* localCache = NULL);

    /**
     * On success, set mask with cached value, allocate memory for mask->fImage,
     * copy pixels from SkCachedData in the cache to mask->fImage, then return true.
     *
     * On failure, return false, no memory allocated for mask->fImage.
     */
    static bool FindAndCopy(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                            const SkRRect& rrect, SkMask* mask);
    static bool FindAndCopy(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                            const SkRect rects[], int count, SkMask* mask);

    /**
     * Create a new SkCachedData, copy pixels from mask.fImage to it, then add it into cache.
     */
    static void AddAndCopy(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                           const SkRRect& rrect, const SkMask& mask);
    static void AddAndCopy(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                           const SkRect rects[], int count, const SkMask& mask);
};

#endif
