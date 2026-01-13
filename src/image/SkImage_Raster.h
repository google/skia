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
#include "include/core/SkRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkMipmap.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class GrDirectContext;
class SkColorSpace;
class SkData;
class SkPixmap;
class SkSurface;
class SkImageShader;
enum SkColorType : int;
struct SkIRect;
struct SkImageInfo;

enum class SkCopyPixelsMode {
    kIfMutable,  //!< only copy src pixels if they are marked mutable
    kAlways,     //!< always copy src pixels (even if they are marked immutable)
    kNever,      //!< never copy src pixels (even if they are marked mutable)
};

class SkImage_Raster : public SkImage_Base {
public:
    SkImage_Raster(const SkImageInfo&, sk_sp<SkData>, size_t rb,
                   uint32_t id = kNeedNewImageUniqueID);
    SkImage_Raster(const SkBitmap& bm, bool bitmapMayBeMutable = false);
    ~SkImage_Raster() override;

    // From SkImage.h
    bool isValid(SkRecorder* recorder) const override {
        if (!recorder) {
            return false;
        }
        if (!recorder->cpuRecorder()) {
            return false;
        }
        return true;
    }
    sk_sp<SkImage> makeColorTypeAndColorSpace(SkRecorder*,
                                              SkColorType targetColorType,
                                              sk_sp<SkColorSpace> targetColorSpace,
                                              RequiredProperties) const override;

    // From SkImage_Base.h
    bool onReadPixels(GrDirectContext*, const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
    bool onPeekPixels(SkPixmap*) const override;
    const SkBitmap* onPeekBitmap() const override { return &fBitmap; }

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const override;

    sk_sp<SkImage> onMakeSubset(SkRecorder*, const SkIRect&, RequiredProperties) const override;

    sk_sp<SkSurface> onMakeSurface(SkRecorder*, const SkImageInfo&) const final;

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool onAsLegacyBitmap(GrDirectContext*, SkBitmap*) const override;

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
        static auto constexpr kCopyMode = SkCopyPixelsMode::kAlways;
        sk_sp<SkImage> img = SkImage_Raster::MakeFromBitmap(fBitmap, kCopyMode);
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

    // Convenience method to return a shader that implements the shader+image behavior defined for
    // drawImage/Bitmap where the paint's shader is ignored when the bitmap is a color image, but
    // properly compose them together when it is an alpha image. This allows the returned paint to
    // be assigned to a paint clone without discarding the original behavior.
    sk_sp<SkShader> makeShaderForPaint(const SkPaint& paint,
                                       SkTileMode tmx,
                                       SkTileMode tmy,
                                       const SkSamplingOptions& sampling,
                                       const SkMatrix* localMatrix);

    /**
     *  Examines the bitmap to decide if it can share the existing pixelRef, or
     *  if it needs to make a deep-copy of the pixels.
     *
     *  The bitmap's pixelref will be shared if either the bitmap is marked as
     *  immutable, or CopyPixelsMode allows it.
     *
     *  If the bitmap's colortype cannot be converted into a corresponding
     *  SkImageInfo, or the bitmap's pixels cannot be accessed, this will return
     *  nullptr.
     */
    static sk_sp<SkImage_Raster> MakeFromBitmap(const SkBitmap&, SkCopyPixelsMode);

private:
    SkBitmap fBitmap;
};

#endif // SkImage_Raster_DEFINED
