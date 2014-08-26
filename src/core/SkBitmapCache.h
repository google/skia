/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapCache_DEFINED
#define SkBitmapCache_DEFINED

#include "SkScalar.h"

class SkBitmap;
class SkMipMap;

class SkBitmapCache {
public:
    /**
     *  Search based on the src bitmap and inverse scales in X and Y. If found, returns true and
     *  result will be set to the matching bitmap with its pixels already locked.
     */
    static bool Find(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY, SkBitmap* result);
    static void Add(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY,
                    const SkBitmap& result);

    /**
     *  Search based on the bitmap's genID, width, height. If found, returns true and
     *  result will be set to the matching bitmap with its pixels already locked.
     */
    static bool Find(uint32_t genID, int width, int height, SkBitmap* result);
    static void Add(uint32_t genID, int width, int height, const SkBitmap& result);
};

class SkMipMapCache {
public:
    static const SkMipMap* FindAndRef(const SkBitmap& src);
    static void Add(const SkBitmap& src, const SkMipMap* result);
};

#endif
