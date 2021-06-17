/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkSurface.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/codec/SkColorTable.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkTLazy.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkBitmapProcShader.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#endif

// fixes https://bug.skia.org/5096
static bool is_not_subset(const SkBitmap& bm) {
    SkASSERT(bm.pixelRef());
    SkISize dim = SkISize::Make(bm.pixelRef()->width(), bm.pixelRef()->height());
    SkASSERT(dim != bm.dimensions() || bm.pixelRefOrigin().isZero());
    return dim == bm.dimensions();
}

class SkImage_Raster : public SkImage_Base {
public:
    static bool ValidArgs(const SkImageInfo& info, size_t rowBytes, size_t* minSize) {
        const int maxDimension = SK_MaxS32 >> 2;

        // TODO(mtklein): eliminate anything here that setInfo() has already checked.
        SkBitmap b;
        if (!b.setInfo(info, rowBytes)) {
            return false;
        }

        if (info.width() <= 0 || info.height() <= 0) {
            return false;
        }
        if (info.width() > maxDimension || info.height() > maxDimension) {
            return false;
        }
        if ((unsigned)info.colorType() > (unsigned)kLastEnum_SkColorType) {
            return false;
        }
        if ((unsigned)info.alphaType() > (unsigned)kLastEnum_SkAlphaType) {
            return false;
        }

        if (kUnknown_SkColorType == info.colorType()) {
            return false;
        }
        if (!info.validRowBytes(rowBytes)) {
            return false;
        }

        size_t size = info.computeByteSize(rowBytes);
        if (SkImageInfo::ByteSizeOverflowed(size)) {
            return false;
        }

        if (minSize) {
            *minSize = size;
        }
        return true;
    }

    SkImage_Raster(const SkImageInfo&, sk_sp<SkData>, size_t rb,
                   uint32_t id = kNeedNewImageUniqueID);
    ~SkImage_Raster() override;

    bool onReadPixels(GrDirectContext*, const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
    bool onPeekPixels(SkPixmap*) const override;
    const SkBitmap* onPeekBitmap() const override { return &fBitmap; }

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&, GrDirectContext*) const override;

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool onAsLegacyBitmap(GrDirectContext*, SkBitmap*) const override;

    SkImage_Raster(const SkBitmap& bm, bool bitmapMayBeMutable = false)
            : INHERITED(bm.info(),
                        is_not_subset(bm) ? bm.getGenerationID() : (uint32_t)kNeedNewImageUniqueID)
            , fBitmap(bm) {
        SkASSERT(bitmapMayBeMutable || fBitmap.isImmutable());
    }

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

#if SK_SUPPORT_GPU
    bool onPinAsTexture(GrRecordingContext*) const override;
    void onUnpinAsTexture(GrRecordingContext*) const override;
    bool isPinnedOnContext(GrRecordingContext*) const override;
#endif

    bool onHasMipmaps() const override { return SkToBool(fBitmap.fMips); }

    SkMipmap* onPeekMips() const override { return fBitmap.fMips.get(); }

    sk_sp<SkImage> onMakeWithMipmaps(sk_sp<SkMipmap> mips) const override {
        auto img = new SkImage_Raster(fBitmap);
        if (mips) {
            img->fBitmap.fMips = std::move(mips);
        } else {
            img->fBitmap.fMips.reset(SkMipmap::Build(fBitmap.pixmap(), nullptr));
        }
        return sk_sp<SkImage>(img);
    }

private:
#if SK_SUPPORT_GPU
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

    SkBitmap fBitmap;

#if SK_SUPPORT_GPU
    mutable GrSurfaceProxyView fPinnedView;
    mutable int32_t fPinnedCount = 0;
    mutable uint32_t fPinnedUniqueID = SK_InvalidUniqueID;
    mutable uint32_t fPinnedContextID = SK_InvalidUniqueID;
    mutable GrColorType fPinnedColorType = GrColorType::kUnknown;
#endif

    using INHERITED = SkImage_Base;
};

///////////////////////////////////////////////////////////////////////////////

static void release_data(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}

SkImage_Raster::SkImage_Raster(const SkImageInfo& info, sk_sp<SkData> data, size_t rowBytes,
                               uint32_t id)
        : INHERITED(info, id) {
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, release_data, data.release());
    fBitmap.setImmutable();
}

SkImage_Raster::~SkImage_Raster() {
#if SK_SUPPORT_GPU
    SkASSERT(!fPinnedView);  // want the caller to have manually unpinned
#endif
}

bool SkImage_Raster::onReadPixels(GrDirectContext*,
                                  const SkImageInfo& dstInfo,
                                  void* dstPixels,
                                  size_t dstRowBytes,
                                  int srcX,
                                  int srcY,
                                  CachingHint) const {
    SkBitmap shallowCopy(fBitmap);
    return shallowCopy.readPixels(dstInfo, dstPixels, dstRowBytes, srcX, srcY);
}

bool SkImage_Raster::onPeekPixels(SkPixmap* pm) const {
    return fBitmap.peekPixels(pm);
}

bool SkImage_Raster::getROPixels(GrDirectContext*, SkBitmap* dst, CachingHint) const {
    *dst = fBitmap;
    return true;
}

#if SK_SUPPORT_GPU
bool SkImage_Raster::onPinAsTexture(GrRecordingContext* rContext) const {
    if (fPinnedView) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
        if (rContext->priv().contextID() != fPinnedContextID) {
            return false;
        }
    } else {
        SkASSERT(fPinnedCount == 0);
        SkASSERT(fPinnedUniqueID == 0);
        std::tie(fPinnedView, fPinnedColorType) = GrMakeCachedBitmapProxyView(rContext,
                                                                              fBitmap,
                                                                              GrMipmapped::kNo);
        if (!fPinnedView) {
            fPinnedColorType = GrColorType::kUnknown;
            return false;
        }
        fPinnedUniqueID = fBitmap.getGenerationID();
        fPinnedContextID = rContext->priv().contextID();
    }
    // Note: we only increment if the texture was successfully pinned
    ++fPinnedCount;
    return true;
}

void SkImage_Raster::onUnpinAsTexture(GrRecordingContext* rContext) const {
    // Note: we always decrement, even if fPinnedTexture is null
    SkASSERT(fPinnedCount > 0);
    SkASSERT(fPinnedUniqueID != 0);
#if 0 // This would be better but Android currently calls with an already freed context ptr.
    if (rContext->priv().contextID() != fPinnedContextID) {
        return;
    }
#endif

    if (0 == --fPinnedCount) {
        fPinnedView = GrSurfaceProxyView();
        fPinnedUniqueID = SK_InvalidUniqueID;
        fPinnedContextID = SK_InvalidUniqueID;
        fPinnedColorType = GrColorType::kUnknown;
    }
}

bool SkImage_Raster::isPinnedOnContext(GrRecordingContext* rContext) const {
    return fPinnedContextID == rContext->priv().contextID();
}
#endif

sk_sp<SkImage> SkImage_Raster::onMakeSubset(const SkIRect& subset, GrDirectContext*) const {
    SkImageInfo info = fBitmap.info().makeDimensions(subset.size());
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(info)) {
        return nullptr;
    }

    void* dst = bitmap.getPixels();
    void* src = fBitmap.getAddr(subset.x(), subset.y());
    if (!dst || !src) {
        SkDEBUGFAIL("SkImage_Raster::onMakeSubset with nullptr src or dst");
        return nullptr;
    }

    SkRectMemcpy(dst, bitmap.rowBytes(), src, fBitmap.rowBytes(), bitmap.rowBytes(),
                 subset.height());

    bitmap.setImmutable();
    return bitmap.asImage();
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> MakeRasterCopyPriv(const SkPixmap& pmap, uint32_t id) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    // Here we actually make a copy of the caller's pixel data
    sk_sp<SkData> data(SkData::MakeWithCopy(pmap.addr(), size));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes(), id);
}

sk_sp<SkImage> SkImage::MakeRasterCopy(const SkPixmap& pmap) {
    return MakeRasterCopyPriv(pmap, kNeedNewImageUniqueID);
}

sk_sp<SkImage> SkImage::MakeRasterData(const SkImageInfo& info, sk_sp<SkData> data,
                                       size_t rowBytes) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, &size) || !data) {
        return nullptr;
    }

    // did they give us enough data?
    if (data->size() < size) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Raster>(info, std::move(data), rowBytes);
}

// TODO: this could be improved to decode and make use of the mipmap
// levels potentially present in the compressed data. For now, any
// mipmap levels are discarded.
sk_sp<SkImage> SkImage::MakeRasterFromCompressed(sk_sp<SkData> data,
                                                 int width, int height,
                                                 CompressionType type) {
    size_t expectedSize = SkCompressedFormatDataSize(type, { width, height }, false);
    if (!data || data->size() < expectedSize) {
        return nullptr;
    }

    SkAlphaType at = SkCompressionTypeIsOpaque(type) ? kOpaque_SkAlphaType
                                                     : kPremul_SkAlphaType;

    SkImageInfo ii = SkImageInfo::MakeN32(width, height, at);

    if (!SkImage_Raster::ValidArgs(ii, ii.minRowBytes(), nullptr)) {
        return nullptr;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(ii)) {
        return nullptr;
    }

    if (!SkDecompress(std::move(data), { width, height }, type, &bitmap)) {
        return nullptr;
    }

    bitmap.setImmutable();
    return MakeFromBitmap(bitmap);
}

sk_sp<SkImage> SkImage::MakeFromRaster(const SkPixmap& pmap, RasterReleaseProc proc,
                                       ReleaseContext ctx) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    sk_sp<SkData> data(SkData::MakeWithProc(pmap.addr(), size, proc, ctx));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes());
}

sk_sp<SkImage> SkMakeImageFromRasterBitmapPriv(const SkBitmap& bm, SkCopyPixelsMode cpm,
                                               uint32_t idForCopy) {
    if (kAlways_SkCopyPixelsMode == cpm || (!bm.isImmutable() && kNever_SkCopyPixelsMode != cpm)) {
        SkPixmap pmap;
        if (bm.peekPixels(&pmap)) {
            return MakeRasterCopyPriv(pmap, idForCopy);
        } else {
            return sk_sp<SkImage>();
        }
    }

    return sk_make_sp<SkImage_Raster>(bm, kNever_SkCopyPixelsMode == cpm);
}

sk_sp<SkImage> SkMakeImageFromRasterBitmap(const SkBitmap& bm, SkCopyPixelsMode cpm) {
    if (!SkImageInfoIsValid(bm.info()) || bm.rowBytes() < bm.info().minRowBytes()) {
        return nullptr;
    }

    return SkMakeImageFromRasterBitmapPriv(bm, cpm, kNeedNewImageUniqueID);
}

const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* image) {
    return ((const SkImage_Raster*)image)->getPixelRef();
}

bool SkImage_Raster::onAsLegacyBitmap(GrDirectContext*, SkBitmap* bitmap) const {
    // When we're a snapshot from a surface, our bitmap may not be marked immutable
    // even though logically always we are, but in that case we can't physically share our
    // pixelref since the caller might call setImmutable() themselves
    // (thus changing our state).
    if (fBitmap.isImmutable()) {
        SkIPoint origin = fBitmap.pixelRefOrigin();
        bitmap->setInfo(fBitmap.info(), fBitmap.rowBytes());
        bitmap->setPixelRef(sk_ref_sp(fBitmap.pixelRef()), origin.x(), origin.y());
        return true;
    }
    return this->INHERITED::onAsLegacyBitmap(nullptr, bitmap);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Raster::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                            sk_sp<SkColorSpace> targetCS,
                                                            GrDirectContext*) const {
    SkPixmap src;
    SkAssertResult(fBitmap.peekPixels(&src));

    SkBitmap dst;
    dst.allocPixels(fBitmap.info().makeColorType(targetCT).makeColorSpace(targetCS));

    SkAssertResult(dst.writePixels(src));
    dst.setImmutable();
    return dst.asImage();
}

sk_sp<SkImage> SkImage_Raster::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // TODO: If our bitmap is immutable, then we could theoretically create another image sharing
    // our pixelRef. That doesn't work (without more invasive logic), because the image gets its
    // gen ID from the bitmap, which gets it from the pixelRef.
    SkPixmap pixmap = fBitmap.pixmap();
    pixmap.setColorSpace(std::move(newCS));
    return SkImage::MakeRasterCopy(pixmap);
}

#if SK_SUPPORT_GPU
std::tuple<GrSurfaceProxyView, GrColorType> SkImage_Raster::onAsView(
        GrRecordingContext* rContext,
        GrMipmapped mipmapped,
        GrImageTexGenPolicy policy) const {
    if (fPinnedView) {
        // We ignore the mipmap request here. If the pinned view isn't mipmapped then we will
        // fallback to bilinear. The pin API is used by Android Framework which does not expose
        // mipmapping.Moreover, we're moving towards requiring that images be made with mip levels
        // if mipmapping is desired (skbug.com/10411)
        mipmapped = GrMipmapped::kNo;
        if (policy != GrImageTexGenPolicy::kDraw) {
            return {CopyView(rContext, fPinnedView, mipmapped, policy), fPinnedColorType};
        }
        return {fPinnedView, fPinnedColorType};
    }
    if (policy == GrImageTexGenPolicy::kDraw) {
        return GrMakeCachedBitmapProxyView(rContext, fBitmap, mipmapped);
    }
    auto budgeted = (policy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted)
            ? SkBudgeted::kNo
            : SkBudgeted::kYes;
    return GrMakeUncachedBitmapProxyView(rContext,
                                         fBitmap,
                                         mipmapped,
                                         SkBackingFit::kExact,
                                         budgeted);
}

std::unique_ptr<GrFragmentProcessor> SkImage_Raster::onAsFragmentProcessor(
        GrRecordingContext* rContext,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    auto mm = sampling.mipmap == SkMipmapMode::kNone ? GrMipmapped::kNo : GrMipmapped::kYes;
    return MakeFragmentProcessorFromView(rContext,
                                         std::get<0>(this->asView(rContext, mm)),
                                         this->alphaType(),
                                         sampling,
                                         tileModes,
                                         m,
                                         subset,
                                         domain);
}
#endif
