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
    // Concrete image types will provide bitmaps in the stored format and color space.
    // For lazy images, lazyColorType and lazyColorSpace are used as hints to choose the decoded
    // color type and color space. If lazyColorType is kUnknown, or lazyColorSpace is nullptr,
    // then the most natural format of the encoded data will be used.
    explicit SkBitmapProvider(const SkImage* img,
                              SkColorType lazyColorType, SkColorSpace* lazyColorSpace)
        : fImage(img)
        , fLazyColorType(lazyColorType)
        , fLazyColorSpace(lazyColorSpace) {
        SkASSERT(img);
    }
    SkBitmapProvider(const SkBitmapProvider& other)
        : fImage(other.fImage)
        , fLazyColorType(other.fLazyColorType)
        , fLazyColorSpace(other.fLazyColorSpace)
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
    SkColorType    fLazyColorType;
    SkColorSpace*  fLazyColorSpace;
};

#endif
