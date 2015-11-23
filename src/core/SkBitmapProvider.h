/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProvider_DEFINED
#define SkBitmapProvider_DEFINED

#include "SkBitmap.h"
#include "SkImage.h"
#include "SkBitmapCache.h"

class SkBitmapProvider {
public:
    explicit SkBitmapProvider(const SkBitmap& bm) : fBitmap(bm) {}
    explicit SkBitmapProvider(const SkImage* img) : fImage(SkSafeRef(img)) {}
    SkBitmapProvider(const SkBitmapProvider& other)
        : fBitmap(other.fBitmap)
        , fImage(SkSafeRef(other.fImage.get()))
    {}

    int width() const;
    int height() const;
    uint32_t getID() const;

    bool validForDrawing() const;
    SkImageInfo info() const;
    bool isVolatile() const;

    SkBitmapCacheDesc makeCacheDesc(int w, int h) const;
    SkBitmapCacheDesc makeCacheDesc() const;
    void notifyAddedToCache() const;

    // Only call this if you're sure you need the bits, since it maybe expensive
    // ... cause a decode and cache, or gpu-readback
    bool asBitmap(SkBitmap*) const;

private:
    SkBitmap fBitmap;
    SkAutoTUnref<const SkImage> fImage;
};

#endif
