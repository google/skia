/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProvider_DEFINED
#define SkBitmapProvider_DEFINED

#include "SkImage.h"
#include "SkBitmapCache.h"

class SkBitmapProvider {
public:
    explicit SkBitmapProvider(const SkImage* img)
        : fImage(img) {
        SkASSERT(img);
    }
    SkBitmapProvider(const SkBitmapProvider& other)
        : fImage(other.fImage)
    {}

    SkBitmapCacheDesc makeCacheDesc() const;
    void notifyAddedToCache() const;

    // Only call this if you're sure you need the bits, since it maybe expensive
    // ... cause a decode and cache, or gpu-readback
    bool asBitmap(SkBitmap*) const;

private:
    // Stack-allocated only.
    void* operator new(size_t) = delete;
    void* operator new(size_t, void*) = delete;

    // SkBitmapProvider is always short-lived/stack allocated, and the source image is guaranteed
    // to outlive its scope => we can store a raw ptr to avoid ref churn.
    const SkImage* fImage;
};

#endif
