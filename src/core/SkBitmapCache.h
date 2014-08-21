/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapCache_DEFINED
#define SkBitmapCache_DEFINED

#include "SkScaledImageCache.h"

class SkBitmapCache {
public:
    typedef SkScaledImageCache::ID ID;

    static void Unlock(ID* id) {
        SkScaledImageCache::Unlock(id);
    }

    /* Input: bitmap+inverse_scale */
    static ID* FindAndLock(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY,
                           SkBitmap* result);
    static ID* AddAndLock(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY,
                          const SkBitmap& result);

    /* Input: bitmap_genID+width+height */
    static ID* FindAndLock(uint32_t genID, int width, int height, SkBitmap* result);

    static ID* AddAndLock(uint32_t genID, int width, int height, const SkBitmap& result);
};

class SkMipMapCache {
public:
    typedef SkScaledImageCache::ID ID;

    static void Unlock(ID* id) {
        SkScaledImageCache::Unlock(id);
    }

    static ID* FindAndLock(const SkBitmap& src, const SkMipMap** result);
    static ID* AddAndLock(const SkBitmap& src, const SkMipMap* result);
};

#endif
