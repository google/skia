/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Base_DEFINED
#define SkImage_Base_DEFINED

#include "SkAtomics.h"
#include "SkImage.h"
#include "SkSurface.h"

#include <new>

class GrTextureParams;
class SkImageCacherator;

enum {
    kNeedNewImageUniqueID = 0
};

class SkImage_Base : public SkImage {
public:
    SkImage_Base(int width, int height, uint32_t uniqueID);
    virtual ~SkImage_Base();

    // User: returns image info for this SkImage.
    // Implementors: if you can not return the value, return an invalid ImageInfo with w=0 & h=0
    // & unknown color space.
    virtual SkImageInfo onImageInfo() const = 0;

    virtual bool onPeekPixels(SkPixmap*) const { return false; }

    // Default impl calls onDraw
    virtual bool onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                              int srcX, int srcY, CachingHint) const;

    virtual GrTexture* peekTexture() const { return nullptr; }
    virtual SkImageCacherator* peekCacherator() const { return nullptr; }

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(SkBitmap*, CachingHint = kAllow_CachingHint) const = 0;

    virtual sk_sp<SkSurface> onNewSurface(const SkImageInfo& info) const {
        return SkSurface::MakeRaster(info);
    }

    // Caller must call unref when they are done.
    virtual GrTexture* asTextureRef(GrContext*, const GrTextureParams&,
                                    SkSourceGammaTreatment) const = 0;

    virtual sk_sp<SkImage> onMakeSubset(const SkIRect&) const = 0;

    // If a ctx is specified, then only gpu-specific formats are requested.
    virtual SkData* onRefEncoded(GrContext*) const { return nullptr; }

    virtual bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const;

    virtual bool onIsLazyGenerated() const { return false; }

    // Return a bitmap suitable for passing to image-filters
    // For now, that means wrapping textures into SkGrPixelRefs...
    virtual bool asBitmapForImageFilters(SkBitmap* bitmap) const {
        return this->getROPixels(bitmap, kAllow_CachingHint);
    }

    // Call when this image is part of the key to a resourcecache entry. This allows the cache
    // to know automatically those entries can be purged when this SkImage deleted.
    void notifyAddedToCache() const {
        fAddedToCache.store(true);
    }

private:
    // Set true by caches when they cache content that's derived from the current pixels.
    mutable SkAtomic<bool> fAddedToCache;

    typedef SkImage INHERITED;
};

static inline SkImage_Base* as_IB(SkImage* image) {
    return static_cast<SkImage_Base*>(image);
}

static inline SkImage_Base* as_IB(const sk_sp<SkImage>& image) {
    return static_cast<SkImage_Base*>(image.get());
}

static inline const SkImage_Base* as_IB(const SkImage* image) {
    return static_cast<const SkImage_Base*>(image);
}

#endif
