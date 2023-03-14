/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Raster_DEFINED
#define SkImage_Raster_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMipmap.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

class GrDirectContext;
class GrFragmentProcessor;
class GrRecordingContext;
class GrSurfaceProxyView;
class SkColorSpace;
class SkData;
class SkMatrix;
class SkPixmap;
enum SkColorType : int;
enum class GrColorType;
enum class GrImageTexGenPolicy : int;
enum class SkTileMode;
struct SkIRect;
struct SkImageInfo;
struct SkRect;

#if defined(SK_GANESH)
#include "include/gpu/GrTypes.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/UploadTask.h"
#endif

class SkImage_Raster : public SkImage_Base {
public:
    SkImage_Raster(const SkImageInfo&, sk_sp<SkData>, size_t rb,
                   uint32_t id = kNeedNewImageUniqueID);
    SkImage_Raster(const SkBitmap& bm, bool bitmapMayBeMutable = false);
    ~SkImage_Raster() override;

    bool onReadPixels(GrDirectContext*, const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
    bool onPeekPixels(SkPixmap*) const override;
    const SkBitmap* onPeekBitmap() const override { return &fBitmap; }

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&, GrDirectContext*) const override;
#if defined(SK_GRAPHITE)
    sk_sp<SkImage> onMakeSubset(const SkIRect&,
                                skgpu::graphite::Recorder*,
                                RequiredImageProperties) const override;
#endif

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool onAsLegacyBitmap(GrDirectContext*, SkBitmap*) const override;

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const override;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    bool onIsValid(GrRecordingContext* context) const override { return true; }
    void notifyAddedToRasterCache() const override {
        // We explicitly DON'T want to call INHERITED::notifyAddedToRasterCache. That ties the
        // lifetime of derived/cached resources to the image. In this case, we only want cached
        // data (eg mips) tied to the lifetime of the underlying pixelRef.
        SkASSERT(fBitmap.pixelRef());
        fBitmap.pixelRef()->notifyAddedToCache();
    }

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kRaster; }

    bool onHasMipmaps() const override { return SkToBool(fBitmap.fMips); }

    SkMipmap* onPeekMips() const override { return fBitmap.fMips.get(); }

    sk_sp<SkImage> onMakeWithMipmaps(sk_sp<SkMipmap> mips) const override {
        // It's dangerous to have two SkBitmaps that share a SkPixelRef but have different SkMipmaps
        // since various caches key on SkPixelRef's generation ID. Also, SkPixelRefs that back
        // SkSurfaces are marked "temporarily immutable" and making an image that uses the same
        // SkPixelRef can interact badly with SkSurface/SkImage copy-on-write. So we just always
        // make a copy with a new ID.
        static auto constexpr kCopyMode = SkCopyPixelsMode::kAlways_SkCopyPixelsMode;
        sk_sp<SkImage> img = SkMakeImageFromRasterBitmap(fBitmap, kCopyMode);
        auto imgRaster = static_cast<SkImage_Raster*>(img.get());
        if (mips) {
            imgRaster->fBitmap.fMips = std::move(mips);
        } else {
            imgRaster->fBitmap.fMips.reset(SkMipmap::Build(fBitmap.pixmap(), nullptr));
        }
        return img;
    }

protected:
    SkBitmap fBitmap;

private:
#if defined(SK_GANESH)
    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                         GrMipmapped,
                                                         GrImageTexGenPolicy) const override;

    std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(GrRecordingContext*,
                                                               SkSamplingOptions,
                                                               const SkTileMode[2],
                                                               const SkMatrix&,
                                                               const SkRect*,
                                                               const SkRect*) const override;
#endif
#if defined(SK_GRAPHITE)
    sk_sp<SkImage> onMakeTextureImage(skgpu::graphite::Recorder*,
                                      RequiredImageProperties) const override;
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                sk_sp<SkColorSpace> targetCS,
                                                skgpu::graphite::Recorder*,
                                                RequiredImageProperties) const override;
#endif

};

#endif // SkImage_Raster_DEFINED
