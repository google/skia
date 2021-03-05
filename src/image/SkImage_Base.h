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
#include "src/gpu/SkGr.h"

class GrTexture;
#endif

#include <new>

class GrDirectContext;
class GrImageContext;
class GrSamplerState;
class SkCachedData;

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

    virtual bool onHasMipmaps() const = 0;

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
                                             RescaleMode,
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
                                                   RescaleMode,
                                                   ReadPixelsCallback,
                                                   ReadPixelsContext);

    virtual GrImageContext* context() const { return nullptr; }

    /** this->context() try-casted to GrDirectContext. Useful for migrations – avoid otherwise! */
    GrDirectContext* directContext() const;

#if SK_SUPPORT_GPU
    virtual GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) {
        return GrSemaphoresSubmitted::kNo;
    }

    // Returns a GrSurfaceProxyView representation of the image, if possible. This also returns
    // a color type. This may be different than the image's color type when the image is not
    // texture-backed and the capabilities of the GPU require a data type conversion to put
    // the data in a texture.
    std::tuple<GrSurfaceProxyView, GrColorType> asView(
            GrRecordingContext* context,
            GrMipmapped mipmapped,
            GrImageTexGenPolicy policy = GrImageTexGenPolicy::kDraw) const;

    virtual bool isYUVA() const { return false; }

    // If this image is the current cached image snapshot of a surface then this is called when the
    // surface is destroyed to indicate no further writes may happen to surface backing store.
    virtual void generatingSurfaceIsDeleted() {}
#endif

    virtual bool onPinAsTexture(GrRecordingContext*) const { return false; }
    virtual void onUnpinAsTexture(GrRecordingContext*) const {}
    virtual bool isPinnedOnContext(GrRecordingContext*) const { return false; }

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

    // Amount of texture memory used by texture-backed images.
    virtual size_t onTextureSize() const { return 0; }

    // Call when this image is part of the key to a resourcecache entry. This allows the cache
    // to know automatically those entries can be purged when this SkImage deleted.
    virtual void notifyAddedToRasterCache() const {
        fAddedToRasterCache.store(true);
    }

    virtual bool onIsValid(GrRecordingContext*) const = 0;

    virtual sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                        GrDirectContext*) const = 0;

    virtual sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const = 0;

    // on failure, returns nullptr
    virtual sk_sp<SkImage> onMakeWithMipmaps(sk_sp<SkMipmap>) const {
        return nullptr;
    }

protected:
    SkImage_Base(const SkImageInfo& info, uint32_t uniqueID);

#if SK_SUPPORT_GPU
    // Utility for making a copy of an existing view when the GrImageTexGenPolicy is not kDraw.
    static GrSurfaceProxyView CopyView(GrRecordingContext*,
                                       GrSurfaceProxyView src,
                                       GrMipmapped,
                                       GrImageTexGenPolicy);
#endif

private:
#if SK_SUPPORT_GPU
    virtual std::tuple<GrSurfaceProxyView, GrColorType> onAsView(
            GrRecordingContext*,
            GrMipmapped,
            GrImageTexGenPolicy policy) const = 0;
#endif
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

#if SK_SUPPORT_GPU
inline GrSurfaceProxyView SkImage_Base::CopyView(GrRecordingContext* context,
                                                 GrSurfaceProxyView src,
                                                 GrMipmapped mipmapped,
                                                 GrImageTexGenPolicy policy) {
    SkBudgeted budgeted = policy == GrImageTexGenPolicy::kNew_Uncached_Budgeted
                          ? SkBudgeted::kYes
                          : SkBudgeted::kNo;
    return GrSurfaceProxyView::Copy(context,
                                    std::move(src),
                                    mipmapped,
                                    SkBackingFit::kExact,
                                    budgeted);
}
#endif

#endif
