/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskCache_DEFINED
#define SkMaskCache_DEFINED

#include "include/core/SkScalar.h"

class SkCachedData;
class SkRRect;
class SkResourceCache;
enum SkBlurStyle : int;
struct SkMask;
struct SkRect;
template <typename T> class SkTLazy;

class SkMaskCache {
public:
    /**
     * On success, return a ref to the SkCachedData that holds the pixels, and have mask
     * already point to that memory.
     *
     * On failure, return nullptr.
     */
    static SkCachedData* FindAndRef(SkScalar sigma, SkBlurStyle style,
                                    const SkRRect& rrect, SkTLazy<SkMask>* mask,
                                    SkResourceCache* localCache = nullptr);
    static SkCachedData* FindAndRef(SkScalar sigma, SkBlurStyle style,
                                    const SkRect rects[], int count, SkTLazy<SkMask>* mask,
                                    SkResourceCache* localCache = nullptr);

    /**
     * Add a mask and its pixel-data to the cache.
     */
    static void Add(SkScalar sigma, SkBlurStyle style,
                    const SkRRect& rrect, const SkMask& mask, SkCachedData* data,
                    SkResourceCache* localCache = nullptr);
    static void Add(SkScalar sigma, SkBlurStyle style,
                    const SkRect rects[], int count, const SkMask& mask, SkCachedData* data,
                    SkResourceCache* localCache = nullptr);
};

#endif
