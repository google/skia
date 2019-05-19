/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Base_DEFINED
#define SkImage_Base_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include <atomic>

#if SK_SUPPORT_GPU
#include "include/private/GrTextureProxy.h"
#include "include/private/SkTDArray.h"

class GrRecordingContext;
class GrTexture;
#endif

#include <new>

class GrSamplerState;
class SkCachedData;
struct SkYUVASizeInfo;

enum {
    kNeedNewImageUniqueID = 0
};

class SkImage_Base : public SkImage {
public:
    virtual ~SkImage_Base();

    virtual SkIRect onGetSubset() const {
        return { 0, 0, this->width(), this->height() };
    }

    virtual bool onPeekPixels(SkPixmap*) const { return false; }

    virtual const SkBitmap* onPeekBitmap() const { return nullptr; }

    virtual bool onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                              int srcX, int srcY, CachingHint) const = 0;

    virtual GrContext* context() const { return nullptr; }

#if SK_SUPPORT_GPU
    virtual GrSemaphoresSubmitted onFlush(GrContext* context, const GrFlushInfo&) {
        return GrSemaphoresSubmitted::kNo;
    }

    // Return the proxy if this image is backed by a single proxy. For YUVA images, this
    // will return nullptr unless the YUVA planes have been converted to RGBA in which case
    // that single backing proxy will be returned.
    virtual GrTextureProxy* peekProxy() const { return nullptr; }
    virtual sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext*) const { return nullptr; }
    virtual sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext*, const GrSamplerState&,
                                                    SkScalar scaleAdjust[2]) const = 0;
    virtual sk_sp<GrTextureProxy> refPinnedTextureProxy(GrRecordingContext*,
                                                        uint32_t* uniqueID) const {
        return nullptr;
    }
    virtual bool isYUVA() const { return false; }
    virtual GrTexture* onGetTexture() const { return nullptr; }
#endif
    virtual GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                                 GrSurfaceOrigin* origin) const;

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(SkBitmap*, CachingHint = kAllow_CachingHint) const = 0;

    virtual sk_sp<SkImage> onMakeSubset(GrRecordingContext*, const SkIRect&) const = 0;

    virtual sk_sp<SkCachedData> getPlanes(SkYUVASizeInfo*, SkYUVAIndex[4],
                                          SkYUVColorSpace*, const void* planes[4]);
    virtual sk_sp<SkData> onRefEncoded() const { return nullptr; }

    virtual bool onAsLegacyBitmap(SkBitmap*) const;

    // True for picture-backed and codec-backed
    virtual bool onIsLazyGenerated() const { return false; }

    // True for images instantiated in GPU memory
    virtual bool onIsTextureBacked() const { return false; }

    // Call when this image is part of the key to a resourcecache entry. This allows the cache
    // to know automatically those entries can be purged when this SkImage deleted.
    virtual void notifyAddedToRasterCache() const {
        fAddedToRasterCache.store(true);
    }

    virtual bool onIsValid(GrContext*) const = 0;

    virtual bool onPinAsTexture(GrContext*) const { return false; }
    virtual void onUnpinAsTexture(GrContext*) const {}

    virtual sk_sp<SkImage> onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                        SkColorType, sk_sp<SkColorSpace>) const = 0;
protected:
    SkImage_Base(const SkImageInfo& info, uint32_t uniqueID);

private:
    // Set true by caches when they cache content that's derived from the current pixels.
    mutable std::atomic<bool> fAddedToRasterCache;

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
