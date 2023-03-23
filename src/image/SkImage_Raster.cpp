/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Raster.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMath.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

class GrDirectContext;
class SkMatrix;
enum class SkTextureCompressionType;
enum class SkTileMode;

#if defined(SK_GANESH)
#include "include/gpu/GpuTypes.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h" // IWYU pragma: keep
#include "src/gpu/ganesh/GrSurfaceProxyView.h" // IWYU pragma: keep
#include "src/gpu/ganesh/SkGr.h"
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

static void release_data(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}

SkImage_Raster::SkImage_Raster(const SkImageInfo& info, sk_sp<SkData> data, size_t rowBytes,
                               uint32_t id)
        : SkImage_Base(info, id) {
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, release_data, data.release());
    fBitmap.setImmutable();
}

// fixes https://bug.skia.org/5096
static bool is_not_subset(const SkBitmap& bm) {
    SkASSERT(bm.pixelRef());
    SkISize dim = SkISize::Make(bm.pixelRef()->width(), bm.pixelRef()->height());
    SkASSERT(dim != bm.dimensions() || bm.pixelRefOrigin().isZero());
    return dim == bm.dimensions();
}

SkImage_Raster::SkImage_Raster(const SkBitmap& bm, bool bitmapMayBeMutable)
        : SkImage_Base(bm.info(),
                    is_not_subset(bm) ? bm.getGenerationID() : (uint32_t)kNeedNewImageUniqueID)
        , fBitmap(bm) {
    SkASSERT(bitmapMayBeMutable || fBitmap.isImmutable());
}

SkImage_Raster::~SkImage_Raster() {}

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

static SkBitmap copy_bitmap_subset(const SkBitmap& orig, const SkIRect& subset) {
    SkImageInfo info = orig.info().makeDimensions(subset.size());
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(info)) {
        return {};
    }

    void* dst = bitmap.getPixels();
    void* src = orig.getAddr(subset.x(), subset.y());
    if (!dst || !src) {
        SkDEBUGFAIL("SkImage_Raster::onMakeSubset with nullptr src or dst");
        return {};
    }

    SkRectMemcpy(dst, bitmap.rowBytes(), src, orig.rowBytes(), bitmap.rowBytes(),
                 subset.height());

    bitmap.setImmutable();
    return bitmap;
}

sk_sp<SkImage> SkImage_Raster::onMakeSubset(const SkIRect& subset, GrDirectContext*) const {
    SkBitmap copy = copy_bitmap_subset(fBitmap, subset);
    if (copy.isNull()) {
        return nullptr;
    } else {
        return copy.asImage();
    }
}

#if defined(SK_GRAPHITE)
static sk_sp<SkMipmap> copy_mipmaps(const SkBitmap& src, SkMipmap* srcMips) {
    if (!srcMips) {
        return nullptr;
    }

    sk_sp<SkMipmap> dst;
    dst.reset(SkMipmap::Build(src.pixmap(), nullptr, /* computeContents= */ false));
    for (int i = 0; i < dst->countLevels(); ++i) {
        SkMipmap::Level srcLevel, dstLevel;
        srcMips->getLevel(i, &srcLevel);
        dst->getLevel(i, &dstLevel);
        srcLevel.fPixmap.readPixels(dstLevel.fPixmap);
    }

    return dst;
}

sk_sp<SkImage> SkImage_Raster::onMakeSubset(const SkIRect& subset,
                                            skgpu::graphite::Recorder* recorder,
                                            RequiredImageProperties requiredProperties) const {
    sk_sp<SkImage> img;

    if (requiredProperties.fMipmapped == skgpu::Mipmapped::kYes) {
        bool fullCopy = subset == SkIRect::MakeSize(fBitmap.dimensions());

        sk_sp<SkMipmap> mips = fullCopy ? copy_mipmaps(fBitmap, fBitmap.fMips.get()) : nullptr;

        // SkImage::withMipmaps will always make a copy for us so we can temporarily share
        // the pixel ref with fBitmap
        SkBitmap tmpSubset;
        if (!fBitmap.extractSubset(&tmpSubset, subset)) {
            return nullptr;
        }

        sk_sp<SkImage> tmp(new SkImage_Raster(tmpSubset, /* bitmapMayBeMutable= */ true));

        // withMipmaps will auto generate the mipmaps if a nullptr is passed in
        SkASSERT(!mips || mips->validForRootLevel(tmp->imageInfo()));
        img = tmp->withMipmaps(std::move(mips));
    } else {
        SkBitmap copy = copy_bitmap_subset(fBitmap, subset);
        if (!copy.isNull()) {
            img = copy.asImage();
        }
    }

    if (!img) {
        return nullptr;
    }

    if (recorder) {
        return img->makeTextureImage(recorder, requiredProperties);
    } else {
        return img;
    }
}
#endif // SK_GRAPHITE

///////////////////////////////////////////////////////////////////////////////

static bool valid_args(const SkImageInfo& info, size_t rowBytes, size_t* minSize) {
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

sk_sp<SkImage> MakeRasterCopyPriv(const SkPixmap& pmap, uint32_t id) {
    size_t size;
    if (!valid_args(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
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
    if (!valid_args(info, rowBytes, &size) || !data) {
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
                                                 SkTextureCompressionType type) {
    size_t expectedSize = SkCompressedFormatDataSize(type, { width, height }, false);
    if (!data || data->size() < expectedSize) {
        return nullptr;
    }

    SkAlphaType at = SkTextureCompressionTypeIsOpaque(type) ? kOpaque_SkAlphaType
                                                     : kPremul_SkAlphaType;

    SkImageInfo ii = SkImageInfo::MakeN32(width, height, at);

    if (!valid_args(ii, ii.minRowBytes(), nullptr)) {
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
    if (!valid_args(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
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
    return this->SkImage_Base::onAsLegacyBitmap(nullptr, bitmap);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Raster::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                            sk_sp<SkColorSpace> targetCS,
                                                            GrDirectContext*) const {
    SkPixmap src;
    SkAssertResult(fBitmap.peekPixels(&src));

    SkBitmap dst;
    if (!dst.tryAllocPixels(fBitmap.info().makeColorType(targetCT).makeColorSpace(targetCS))) {
        return nullptr;
    }

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

#if defined(SK_GANESH)
std::tuple<GrSurfaceProxyView, GrColorType> SkImage_Raster::onAsView(
        GrRecordingContext* rContext,
        GrMipmapped mipmapped,
        GrImageTexGenPolicy policy) const {
    if (policy == GrImageTexGenPolicy::kDraw) {
        // If the draw doesn't require mipmaps but this SkImage has them go ahead and make a
        // mipmapped texture. There are three reasons for this:
        // 1) Avoiding another texture creation if a later draw requires mipmaps.
        // 2) Ensuring we upload the bitmap's levels instead of generating on the GPU from the base.
        if (this->hasMipmaps()) {
            mipmapped = GrMipmapped::kYes;
        }
        return GrMakeCachedBitmapProxyView(rContext,
                                           fBitmap,
                                           /*label=*/"TextureForImageRasterWithPolicyEqualKDraw",
                                           mipmapped);
    }
    auto budgeted = (policy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted)
                            ? skgpu::Budgeted::kNo
                            : skgpu::Budgeted::kYes;
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

#if defined(SK_GRAPHITE)
sk_sp<SkImage> SkImage_Raster::onMakeTextureImage(skgpu::graphite::Recorder* recorder,
                                                  RequiredImageProperties requiredProps) const {
    return skgpu::graphite::MakeFromBitmap(recorder,
                                           this->imageInfo().colorInfo(),
                                           fBitmap,
                                           this->refMips(),
                                           skgpu::Budgeted::kNo,
                                           requiredProps);
}

sk_sp<SkImage> SkImage_Raster::onMakeColorTypeAndColorSpace(
        SkColorType targetCT,
        sk_sp<SkColorSpace> targetCS,
        skgpu::graphite::Recorder* recorder,
        RequiredImageProperties requiredProps) const {
    SkPixmap src;
    SkAssertResult(fBitmap.peekPixels(&src));

    SkBitmap dst;
    if (!dst.tryAllocPixels(fBitmap.info().makeColorType(targetCT).makeColorSpace(targetCS))) {
        return nullptr;
    }

    SkAssertResult(dst.writePixels(src));
    dst.setImmutable();

    sk_sp<SkImage> tmp = dst.asImage();
    if (recorder) {
        return tmp->makeTextureImage(recorder, requiredProps);
    } else {
        return tmp;
    }
}

#endif
