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
#include "include/core/SkTypes.h"
#include "src/core/SkMipmap.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

class GrDirectContext;
class GrImageContext;
class SkBitmap;
class SkColorSpace;
class SkPixmap;
class SkSurface;
enum SkColorType : int;
enum SkYUVColorSpace : int;
struct SkIRect;
struct SkISize;
struct SkImageInfo;

enum {
    kNeedNewImageUniqueID = 0
};

namespace skgpu::graphite {
class Recorder;
}

class SkImage_Base : public SkImage {
public:
    ~SkImage_Base() override;

    // From SkImage.h
    sk_sp<SkImage> makeColorSpace(GrDirectContext*, sk_sp<SkColorSpace>) const override;
    sk_sp<SkImage> makeColorSpace(skgpu::graphite::Recorder*,
                                  sk_sp<SkColorSpace>,
                                  RequiredProperties) const override;
    sk_sp<SkImage> makeColorTypeAndColorSpace(GrDirectContext* dContext,
                                              SkColorType targetColorType,
                                              sk_sp<SkColorSpace> targetCS) const override;
    sk_sp<SkImage> makeColorTypeAndColorSpace(skgpu::graphite::Recorder*,
                                              SkColorType,
                                              sk_sp<SkColorSpace>,
                                              RequiredProperties) const override;
    sk_sp<SkImage> makeSubset(GrDirectContext* direct, const SkIRect& subset) const override;
    sk_sp<SkImage> makeSubset(skgpu::graphite::Recorder*,
                              const SkIRect&,
                              RequiredProperties) const override;

    size_t textureSize() const override { return 0; }

    // Methods that we want to use elsewhere in Skia, but not be a part of the public API.
    virtual bool onPeekPixels(SkPixmap*) const { return false; }

    virtual const SkBitmap* onPeekBitmap() const { return nullptr; }

    virtual bool onReadPixels(GrDirectContext*,
                              const SkImageInfo& dstInfo,
                              void* dstPixels,
                              size_t dstRowBytes,
                              int srcX,
                              int srcY,
                              CachingHint) const = 0;

    // used by makeScaled()
    virtual sk_sp<SkSurface> onMakeSurface(skgpu::graphite::Recorder*,
                                           const SkImageInfo&) const = 0;

    virtual bool readPixelsGraphite(skgpu::graphite::Recorder*,
                                    const SkPixmap& dst,
                                    int srcX,
                                    int srcY) const {
        return false;
    }

    virtual bool onHasMipmaps() const = 0;
    virtual bool onIsProtected() const = 0;

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
                                                   bool readAlpha,
                                                   sk_sp<SkColorSpace> dstColorSpace,
                                                   SkIRect srcRect,
                                                   SkISize dstSize,
                                                   RescaleGamma,
                                                   RescaleMode,
                                                   ReadPixelsCallback,
                                                   ReadPixelsContext) const;

    virtual GrImageContext* context() const { return nullptr; }

    /** this->context() try-casted to GrDirectContext. Useful for migrations – avoid otherwise! */
    virtual GrDirectContext* directContext() const { return nullptr; }

    // If this image is the current cached image snapshot of a surface then this is called when the
    // surface is destroyed to indicate no further writes may happen to surface backing store.
    virtual void generatingSurfaceIsDeleted() {}

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(GrDirectContext*, SkBitmap*,
                             CachingHint = kAllow_CachingHint) const = 0;

    virtual sk_sp<SkImage> onMakeSubset(GrDirectContext*, const SkIRect&) const = 0;

    virtual sk_sp<SkData> onRefEncoded() const { return nullptr; }

    virtual bool onAsLegacyBitmap(GrDirectContext*, SkBitmap*) const;

    enum class Type {
        kRaster,
        kRasterPinnable,
        kLazy,
        kLazyPicture,
        kGanesh,
        kGaneshYUVA,
        kGraphite,
        kGraphiteYUVA,
    };

    virtual Type type() const = 0;

    // True for picture-backed and codec-backed
    bool isLazyGenerated() const override {
        return this->type() == Type::kLazy || this->type() == Type::kLazyPicture;
    }

    bool isRasterBacked() const {
        return this->type() == Type::kRaster || this->type() == Type::kRasterPinnable;
    }

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

    bool isTextureBacked() const override {
        return this->isGaneshBacked() || this->isGraphiteBacked();
    }

    // Call when this image is part of the key to a resourcecache entry. This allows the cache
    // to know automatically those entries can be purged when this SkImage deleted.
    virtual void notifyAddedToRasterCache() const {
        fAddedToRasterCache.store(true);
    }

    virtual sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                        GrDirectContext*) const = 0;

    virtual sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const = 0;

    // on failure, returns nullptr
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    virtual sk_sp<SkImage> onMakeWithMipmaps(sk_sp<SkMipmap>) const {
        return nullptr;
    }

    virtual sk_sp<SkImage> onMakeSubset(skgpu::graphite::Recorder*,
                                        const SkIRect&,
                                        RequiredProperties) const = 0;

protected:
    SkImage_Base(const SkImageInfo& info, uint32_t uniqueID);

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

static inline const SkImage_Base* as_IB(const sk_sp<const SkImage>& image) {
    return static_cast<const SkImage_Base*>(image.get());
}

#endif
