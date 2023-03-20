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
#include <string_view>
#include <tuple>

#if defined(SK_GANESH)
#include "include/gpu/GrTypes.h"
#include "src/gpu/ganesh/SkGr.h"
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
class GrFragmentProcessor;
class GrImageContext;
class GrRecordingContext;
class GrSurfaceProxyView;
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

    // Returns a GrSurfaceProxyView representation of the image, if possible. This also returns
    // a color type. This may be different than the image's color type when the image is not
    // texture-backed and the capabilities of the GPU require a data type conversion to put
    // the data in a texture.
    std::tuple<GrSurfaceProxyView, GrColorType> asView(
            GrRecordingContext* context,
            GrMipmapped mipmapped,
            GrImageTexGenPolicy policy = GrImageTexGenPolicy::kDraw) const;

    /**
     * Returns a GrFragmentProcessor that can be used with the passed GrRecordingContext to
     * draw the image. SkSamplingOptions indicates the filter and SkTileMode[] indicates the x and
     * y tile modes. The passed matrix is applied to the coordinates before sampling the image.
     * Optional 'subset' indicates whether the tile modes should be applied to a subset of the image
     * Optional 'domain' is a bound on the coordinates of the image that will be required and can be
     * used to optimize the shader if 'subset' is also specified.
     */
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(GrRecordingContext*,
                                                             SkSamplingOptions,
                                                             const SkTileMode[2],
                                                             const SkMatrix&,
                                                             const SkRect* subset = nullptr,
                                                             const SkRect* domain = nullptr) const;

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
#if defined(SK_GANESH) || defined(SK_GRAPHITE)
    bool isYUVA() const {
        return this->type() == Type::kGaneshYUVA || this->type() == Type::kGraphiteYUVA;
    }
#endif

    virtual bool onPinAsTexture(GrRecordingContext*) const { return false; }
    virtual void onUnpinAsTexture(GrRecordingContext*) const {}
    virtual bool isPinnedOnContext(GrRecordingContext*) const { return false; }

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(GrDirectContext*, SkBitmap*,
                             CachingHint = kAllow_CachingHint) const = 0;

    virtual sk_sp<SkImage> onMakeSubset(const SkIRect&, GrDirectContext*) const = 0;

    virtual sk_sp<SkData> onRefEncoded() const { return nullptr; }

    virtual bool onAsLegacyBitmap(GrDirectContext*, SkBitmap*) const;

    enum class Type {
        kUnknown,
        kRaster,
        kRasterPinnable,
        kLazy,
        kGanesh,
        kGaneshYUVA,
        kGraphite,
        kGraphiteYUVA,
    };

    virtual Type type() const { return Type::kUnknown; }

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
    // Utility for making a copy of an existing view when the GrImageTexGenPolicy is not kDraw.
    static GrSurfaceProxyView CopyView(GrRecordingContext*,
                                       GrSurfaceProxyView src,
                                       GrMipmapped,
                                       GrImageTexGenPolicy,
                                       std::string_view label);

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
#if defined(SK_GANESH)
    virtual std::tuple<GrSurfaceProxyView, GrColorType> onAsView(
            GrRecordingContext*,
            GrMipmapped,
            GrImageTexGenPolicy policy) const = 0;

    virtual std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(
            GrRecordingContext*,
            SkSamplingOptions,
            const SkTileMode[2],
            const SkMatrix&,
            const SkRect* subset,
            const SkRect* domain) const = 0;
#endif

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
