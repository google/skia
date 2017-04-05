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

#if SK_SUPPORT_GPU
    #include "GrTexture.h"
    #include "GrTextureProxy.h"
#endif

#include <new>

class GrSamplerParams;
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
    virtual SkAlphaType onAlphaType() const = 0;

    virtual bool onPeekPixels(SkPixmap*) const { return false; }

    virtual const SkBitmap* onPeekBitmap() const { return nullptr; }

    virtual bool onReadYUV8Planes(const SkISize sizes[3], void* const planes[3],
                                  const size_t rowBytes[3], SkYUVColorSpace colorSpace) const;

    virtual bool onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                              int srcX, int srcY, CachingHint) const = 0;

    // MDB TODO: this entry point needs to go away
    virtual GrTexture* peekTexture() const { return nullptr; }
#if SK_SUPPORT_GPU
    virtual GrTextureProxy* peekProxy() const { return nullptr; }
    virtual sk_sp<GrTextureProxy> asTextureProxyRef() const { return nullptr; }
    virtual sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerParams&,
                                                    SkColorSpace*, sk_sp<SkColorSpace>*,
                                                    SkScalar scaleAdjust[2]) const = 0;
    virtual sk_sp<GrTextureProxy> refPinnedTextureProxy(uint32_t* uniqueID) const {
        return nullptr;
    }
    virtual GrBackendObject onGetTextureHandle(bool flushPendingGrContextIO,
                                               GrSurfaceOrigin* origin) const {
        return 0;
    }
    virtual GrTexture* onGetTexture() const { return nullptr; }
#endif
    virtual SkImageCacherator* peekCacherator() const { return nullptr; }

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace,
                             CachingHint = kAllow_CachingHint) const = 0;

    virtual sk_sp<SkImage> onMakeSubset(const SkIRect&) const = 0;

    // If a ctx is specified, then only gpu-specific formats are requested.
    virtual SkData* onRefEncoded(GrContext*) const { return nullptr; }

    virtual bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const;

    virtual bool onIsLazyGenerated() const { return false; }

    // Call when this image is part of the key to a resourcecache entry. This allows the cache
    // to know automatically those entries can be purged when this SkImage deleted.
    void notifyAddedToCache() const {
        fAddedToCache.store(true);
    }

    virtual bool onPinAsTexture(GrContext*) const { return false; }
    virtual void onUnpinAsTexture(GrContext*) const {}

    virtual sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>) const = 0;

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
