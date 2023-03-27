/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Base_DEFINED
#define SkImage_Base_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTypes.h"
#include "src/core/SkMipmap.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

#if defined(SK_GANESH)
#include "include/gpu/GrTypes.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
enum class GrImageTexGenPolicy : int;
#endif

#if defined(SK_GRAPHITE)
namespace skgpu {
namespace graphite {
class TextureProxyView;
}
}
#endif

class GrBackendTexture;
class GrDirectContext;
class GrImageContext;
class GrRecordingContext;
class SkBitmap;
class SkColorSpace;
class SkMatrix;
class SkPixmap;
enum SkAlphaType : int;
enum SkColorType : int;
enum SkYUVColorSpace : int;
enum class GrColorType;
enum class SkTileMode;
struct SkIRect;
struct SkISize;
struct SkImageInfo;
struct SkRect;

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
                                             SkIRect srcRect,
                                             RescaleGamma,
                                             RescaleMode,
                                             ReadPixelsCallback,
                                             ReadPixelsContext) const;
    /**
     * Default implementation does a rescale/read/yuv conversion and then calls the callback.
     */
    virtual void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                                   sk_sp<SkColorSpace> dstColorSpace,
                                                   SkIRect srcRect,
                                                   SkISize dstSize,
                                                   RescaleGamma,
                                                   RescaleMode,
                                                   ReadPixelsCallback,
                                                   ReadPixelsContext) const;

    virtual GrImageContext* context() const { return nullptr; }

    /** this->context() try-casted to GrDirectContext. Useful for migrations – avoid otherwise! */
    GrDirectContext* directContext() const;

#if defined(SK_GANESH)
    virtual GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) const {
        return GrSemaphoresSubmitted::kNo;
    }

    // TODO(kjlubick) move the implementations of this out into GrImageUtils.cpp
    virtual std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                                 GrMipmapped,
                                                                 GrImageTexGenPolicy policy) const {
        SK_ABORT("should call GrImageUtils::AsView\n");
        return {};
    }

    // TODO(kjlubick) move the implementations of this out into GrImageUtils.cpp
    virtual std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(GrRecordingContext*,
                                                                       SkSamplingOptions,
                                                                       const SkTileMode[2],
                                                                       const SkMatrix&,
                                                                       const SkRect* subset,
                                                                       const SkRect* domain) const {
        SK_ABORT("should call GrImageUtils::AsFragmentProcessor\n");
        return nullptr;
    }

    // If this image is the current cached image snapshot of a surface then this is called when the
    // surface is destroyed to indicate no further writes may happen to surface backing store.
    virtual void generatingSurfaceIsDeleted() {}

    virtual GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                                 GrSurfaceOrigin* origin) const;
#endif
#if defined(SK_GRAPHITE)
    // Returns a TextureProxyView representation of the image, if possible. This also returns
    // a color type. This may be different than the image's color type when the image is not
    // texture-backed and the capabilities of the GPU require a data type conversion to put
    // the data in a texture.
    std::tuple<skgpu::graphite::TextureProxyView, SkColorType> asView(
            skgpu::graphite::Recorder*,
            skgpu::Mipmapped) const;

#endif

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(GrDirectContext*, SkBitmap*,
                             CachingHint = kAllow_CachingHint) const = 0;

    virtual sk_sp<SkImage> onMakeSubset(const SkIRect&, GrDirectContext*) const = 0;

    virtual sk_sp<SkData> onRefEncoded() const { return nullptr; }

    virtual bool onAsLegacyBitmap(GrDirectContext*, SkBitmap*) const;

    enum class Type {
        kRaster,
        kRasterPinnable,
        kLazy,
        kGanesh,
        kGaneshYUVA,
        kGraphite,
        kGraphiteYUVA,
    };

    virtual Type type() const = 0;

    // True for picture-backed and codec-backed
    bool onIsLazyGenerated() const { return this->type() == Type::kLazy; }

    // True for images instantiated by Ganesh in GPU memory
    bool isGaneshBacked() const {
        return this->type() == Type::kGanesh || this->type() == Type::kGaneshYUVA;
    }

    // True for images instantiated by Graphite in GPU memory
    bool isGraphiteBacked() const {
        return this->type() == Type::kGraphite || this->type() == Type::kGraphiteYUVA;
    }

    bool isYUVA() const {
        return this->type() == Type::kGaneshYUVA || this->type() == Type::kGraphiteYUVA;
    }

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

#if defined(SK_GRAPHITE)
    virtual sk_sp<SkImage> onMakeTextureImage(skgpu::graphite::Recorder*,
                                              RequiredImageProperties) const = 0;
    virtual sk_sp<SkImage> onMakeSubset(const SkIRect&,
                                        skgpu::graphite::Recorder*,
                                        RequiredImageProperties) const = 0;
    virtual sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType,
                                                        sk_sp<SkColorSpace>,
                                                        skgpu::graphite::Recorder*,
                                                        RequiredImageProperties) const = 0;
#endif

protected:
    SkImage_Base(const SkImageInfo& info, uint32_t uniqueID);

#if defined(SK_GANESH)
    static std::unique_ptr<GrFragmentProcessor> MakeFragmentProcessorFromView(GrRecordingContext*,
                                                                              GrSurfaceProxyView,
                                                                              SkAlphaType,
                                                                              SkSamplingOptions,
                                                                              const SkTileMode[2],
                                                                              const SkMatrix&,
                                                                              const SkRect* subset,
                                                                              const SkRect* domain);

    /**
     * Returns input view if it is already mipmapped. Otherwise, attempts to make a mipmapped view
     * with the same contents. If the mipmapped copy is successfully created it will be cached
     * using the image unique ID. A subsequent call with the same unique ID will return the cached
     * view if it has not been purged. The view is cached with a key domain specific to this
     * function.
     */
    static GrSurfaceProxyView FindOrMakeCachedMipmappedView(GrRecordingContext*,
                                                            GrSurfaceProxyView,
                                                            uint32_t imageUniqueID);
#endif

private:
    // Set true by caches when they cache content that's derived from the current pixels.
    mutable std::atomic<bool> fAddedToRasterCache;
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
