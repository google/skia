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
#include "src/core/SkMipmap.h"
#include <atomic>

#if SK_SUPPORT_GPU
#include "include/private/SkTDArray.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/GrTextureProxy.h"

class GrTexture;
#endif

#include <new>

class GrDirectContext;
class GrImageContext;
class GrSamplerState;
class SkCachedData;
struct SkYUVASizeInfo;

enum {
    kNeedNewImageUniqueID = 0
};

class SkImage_Base : public SkImage {
public:
    ~SkImage_Base() override;

    virtual bool onPeekPixels(SkPixmap*) const { return false; }

    virtual const SkBitmap* onPeekBitmap() const { return nullptr; }

    virtual bool onReadPixels(GrDirectContext*,
                              const SkImageInfo& dstInfo,
                              void* dstPixels,
                              size_t dstRowBytes,
                              int srcX,
                              int srcY,
                              CachingHint) const = 0;

    virtual SkMipmap* onPeekMips() const { return nullptr; }

    sk_sp<SkMipmap> refMips() const {
        return sk_ref_sp(this->onPeekMips());
    }

    /**
     * Default implementation does a rescale/read and then calls the callback.
     */
    virtual void onAsyncRescaleAndReadPixels(const SkImageInfo&,
                                             const SkIRect& srcRect,
                                             RescaleGamma,
                                             SkFilterQuality,
                                             ReadPixelsCallback,
                                             ReadPixelsContext);
    /**
     * Default implementation does a rescale/read/yuv conversion and then calls the callback.
     */
    virtual void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                                   sk_sp<SkColorSpace> dstColorSpace,
                                                   const SkIRect& srcRect,
                                                   const SkISize& dstSize,
                                                   RescaleGamma,
                                                   SkFilterQuality,
                                                   ReadPixelsCallback,
                                                   ReadPixelsContext);

    virtual GrImageContext* context() const { return nullptr; }

    /** this->context() try-casted to GrDirectContext. Useful for migrations – avoid otherwise! */
    GrDirectContext* directContext() const;

#if SK_SUPPORT_GPU
    virtual GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) {
        return GrSemaphoresSubmitted::kNo;
    }

    // Return the proxy if this image is backed by a single proxy. For YUVA images, this
    // will return nullptr unless the YUVA planes have been converted to RGBA in which case
    // that single backing proxy will be returned.
    virtual GrTextureProxy* peekProxy() const { return nullptr; }

    // If it exists, this returns a pointer to the GrSurfaceProxyView of image. The caller does not
    // own the returned view and must copy it if they want to gain a ref to the internal proxy.
    // If the returned view is not null, then it is guaranteed to have a valid proxy. Additionally
    // this call will flatten a SkImage_GpuYUV to a single texture.
    virtual const GrSurfaceProxyView* view(GrRecordingContext*) const { return nullptr; }

    virtual GrSurfaceProxyView refView(GrRecordingContext*, GrMipmapped) const = 0;
    virtual GrSurfaceProxyView refPinnedView(GrRecordingContext*, uint32_t* uniqueID) const {
        return {};
    }
    virtual bool isYUVA() const { return false; }
#endif
    virtual GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                                 GrSurfaceOrigin* origin) const;

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(GrDirectContext*, SkBitmap*,
                             CachingHint = kAllow_CachingHint) const = 0;

    virtual sk_sp<SkImage> onMakeSubset(const SkIRect&, GrDirectContext*) const = 0;

    virtual sk_sp<SkData> onRefEncoded() const { return nullptr; }

    virtual bool onAsLegacyBitmap(GrDirectContext*, SkBitmap*) const;

    // True for picture-backed and codec-backed
    virtual bool onIsLazyGenerated() const { return false; }

    // True for images instantiated in GPU memory
    virtual bool onIsTextureBacked() const { return false; }

    // Call when this image is part of the key to a resourcecache entry. This allows the cache
    // to know automatically those entries can be purged when this SkImage deleted.
    virtual void notifyAddedToRasterCache() const {
        fAddedToRasterCache.store(true);
    }

    virtual bool onIsValid(GrRecordingContext*) const = 0;

    virtual bool onPinAsTexture(GrRecordingContext*) const { return false; }
    virtual void onUnpinAsTexture(GrRecordingContext*) const {}

    virtual sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                        GrDirectContext*) const = 0;

    virtual sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const = 0;

    // on failure, returns nullptr
    virtual sk_sp<SkImage> onMakeWithMipmaps(sk_sp<SkMipmap>) const {
        return nullptr;
    }

protected:
    SkImage_Base(const SkImageInfo& info, uint32_t uniqueID);

private:
    // Set true by caches when they cache content that's derived from the current pixels.
    mutable std::atomic<bool> fAddedToRasterCache;

    using INHERITED = SkImage;
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
