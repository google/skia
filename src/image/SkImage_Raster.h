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
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMipmap.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class GrDirectContext;
class GrRecordingContext;
class SkColorSpace;
class SkData;
class SkPixmap;
class SkSurface;
enum SkColorType : int;
struct SkIRect;
struct SkImageInfo;

namespace skgpu { namespace graphite { class Recorder; } }

class SkImage_Raster : public SkImage_Base {
public:
    SkImage_Raster(const SkImageInfo&, sk_sp<SkData>, size_t rb,
                   uint32_t id = kNeedNewImageUniqueID);
    SkImage_Raster(const SkBitmap& bm, bool bitmapMayBeMutable = false);
    ~SkImage_Raster() override;

    // From SkImage.h
    bool isValid(GrRecordingContext* context) const override { return true; }

    // From SkImage_Base.h
    bool onReadPixels(GrDirectContext*, const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
    bool onPeekPixels(SkPixmap*) const override;
    const SkBitmap* onPeekBitmap() const override { return &fBitmap; }

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const override;
    sk_sp<SkImage> onMakeSubset(GrDirectContext*, const SkIRect&) const override;
    sk_sp<SkImage> onMakeSubset(skgpu::graphite::Recorder*,
                                const SkIRect&,
                                RequiredProperties) const override;

    sk_sp<SkSurface> onMakeSurface(skgpu::graphite::Recorder*, const SkImageInfo&) const override;

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool onAsLegacyBitmap(GrDirectContext*, SkBitmap*) const override;

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const override;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    void notifyAddedToRasterCache() const override {
        // We explicitly DON'T want to call INHERITED::notifyAddedToRasterCache. That ties the
        // lifetime of derived/cached resources to the image. In this case, we only want cached
        // data (eg mips) tied to the lifetime of the underlying pixelRef.
        SkASSERT(fBitmap.pixelRef());
        fBitmap.pixelRef()->notifyAddedToCache();
    }

    bool onHasMipmaps() const override { return SkToBool(fBitmap.fMips); }
    bool onIsProtected() const override { return false; }

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

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kRaster; }

    SkBitmap bitmap() const { return fBitmap; }

private:
    SkBitmap fBitmap;
};

sk_sp<SkImage> MakeRasterCopyPriv(const SkPixmap& pmap, uint32_t id);

#endif // SkImage_Raster_DEFINED
