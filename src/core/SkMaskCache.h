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
     * On failure, return nullptr.
     */
    static SkCachedData* FindAndRef(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                                    const SkRRect& rrect, SkMask* mask,
                                    SkResourceCache* localCache = nullptr);
    static SkCachedData* FindAndRef(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                                    const SkRect rects[], int count, SkMask* mask,
                                    SkResourceCache* localCache = nullptr);

    /**
     * Add a mask and its pixel-data to the cache.
     */
    static void Add(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                    const SkRRect& rrect, const SkMask& mask, SkCachedData* data,
                    SkResourceCache* localCache = nullptr);
    static void Add(SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                    const SkRect rects[], int count, const SkMask& mask, SkCachedData* data,
                    SkResourceCache* localCache = nullptr);
};

#endif
