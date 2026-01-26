/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/image/SkImage_Raster.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCPURecorder.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkImageShader.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class GrDirectContext;
class SkSurfaceProps;

// fixes skbug.com/40036261
static bool is_not_subset(const SkBitmap& bm) {
    SkASSERT(bm.pixelRef());
    SkISize dim = SkISize::Make(bm.pixelRef()->width(), bm.pixelRef()->height());
    SkASSERT(dim != bm.dimensions() || bm.pixelRefOrigin().isZero());
    return dim == bm.dimensions();
}

static void release_data(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}

SkImage_Raster::SkImage_Raster(const SkImageInfo& info,
                               sk_sp<SkData> data,
                               size_t rowBytes,
                               sk_sp<SkMipmap> mips,
                               uint32_t id)
        : SkImage_Base(info, id), fMips(mips) {
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, release_data, data.release());
    fBitmap.setImmutable();
}

SkImage_Raster::SkImage_Raster(const SkBitmap& bm, sk_sp<SkMipmap> mips, bool bitmapMayBeMutable)
        : SkImage_Base(bm.info(),
                       is_not_subset(bm) ? bm.getGenerationID() : (uint32_t)kNeedNewImageUniqueID)
        , fBitmap(bm)
        , fMips(mips) {
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

sk_sp<SkSurface> SkImage_Raster::onMakeSurface(SkRecorder* recorder,
                                               const SkImageInfo& info) const {
    if (!recorder) {
        // TODO(kjlubick) remove this after old SkImage::makeScaled(image info, sampling) API gone
        recorder = skcpu::Recorder::TODO();
    }
    const SkSurfaceProps* props = nullptr;
    constexpr size_t rowBytes = 0;
    return recorder->cpuRecorder()->makeBitmapSurface(info, rowBytes, props);
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

static sk_sp<SkMipmap> copy_mipmaps(const SkBitmap& src, SkMipmap* srcMips) {
    if (!srcMips) {
        return nullptr;
    }

    sk_sp<SkMipmap> dst;
    dst.reset(SkMipmap::Build(src.pixmap(),
                              /* factoryProc= */ nullptr,
                              /* computeContents= */ false));
    if (!dst) {
        return nullptr;
    }
    for (int i = 0; i < dst->countLevels(); ++i) {
        SkMipmap::Level srcLevel, dstLevel;
        srcMips->getLevel(i, &srcLevel);
        dst->getLevel(i, &dstLevel);
        srcLevel.fPixmap.readPixels(dstLevel.fPixmap);
    }

    return dst;
}

sk_sp<SkImage> SkImage_Raster::onMakeSubset(SkRecorder*,
                                            const SkIRect& subset,
                                            RequiredProperties requiredProperties) const {
    sk_sp<SkImage> img;

    if (requiredProperties.fMipmapped) {
        bool fullCopy = subset == SkIRect::MakeSize(fBitmap.dimensions());

        sk_sp<SkMipmap> mips = fullCopy ? copy_mipmaps(fBitmap, fMips.get()) : nullptr;

        // SkImage::withMipmaps will always make a copy for us so we can temporarily share
        // the pixel ref with fBitmap
        SkBitmap tmpSubset;
        if (!fBitmap.extractSubset(&tmpSubset, subset)) {
            return nullptr;
        }

        sk_sp<SkImage> tmp(
                new SkImage_Raster(tmpSubset, /*mips=*/nullptr, /* bitmapMayBeMutable= */ true));

        // withMipmaps will auto generate the mipmaps if a nullptr is passed in
        SkASSERT(!mips || mips->validForRootLevel(tmp->imageInfo()));
        img = tmp->withMipmaps(std::move(mips));
    } else {
        SkBitmap copy = copy_bitmap_subset(fBitmap, subset);
        if (!copy.isNull()) {
            img = SkImages::RasterFromBitmap(copy);
        }
    }

    return img;
}

sk_sp<SkImage_Raster> SkImage_Raster::MakeFromBitmap(const SkBitmap& bm,
                                              SkCopyPixelsMode cpm,
                                              sk_sp<SkMipmap> mips) {
    if (!SkImageInfoIsValid(bm.info()) || bm.rowBytes() < bm.info().minRowBytes()) {
        return nullptr;
    }

    if (!bm.getPixels()) {
        return nullptr;
    }

    if (SkCopyPixelsMode::kAlways == cpm ||
        (!bm.isImmutable() && SkCopyPixelsMode::kNever != cpm)) {
        size_t size = bm.computeByteSize();
        if (SkImageInfo::ByteSizeOverflowed(size)) {
            return nullptr;
        }

        sk_sp<SkData> data(SkData::MakeWithCopy(bm.getPixels(), size));

        return sk_sp<SkImage_Raster>(new SkImage_Raster(
                bm.info(), std::move(data), bm.rowBytes(), std::move(mips), kNeedNewImageUniqueID));
    }
    return sk_sp<SkImage_Raster>(
            new SkImage_Raster(bm, std::move(mips), SkCopyPixelsMode::kNever == cpm));
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

sk_sp<SkShader> SkImage_Raster::makeShaderForPaint(const SkPaint& paint,
                                                   SkTileMode tmx,
                                                   SkTileMode tmy,
                                                   const SkSamplingOptions& sampling,
                                                   const SkMatrix* localMatrix) {
    auto s = SkImageShader::Make(sk_ref_sp<SkImage>(this), tmx, tmy, sampling, localMatrix);
    if (!s) {
        return nullptr;
    }
    if (SkColorTypeIsAlphaOnly(this->colorType()) && paint.getShader()) {
        // Compose the image shader with the paint's shader. Alpha images+shaders should output the
        // texture's alpha multiplied by the shader's color. DstIn (d*sa) will achieve this with
        // the source image and dst shader (MakeBlend takes dst first, src second).
        s = SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(), std::move(s));
    }
    return s;
}

sk_sp<SkImage> SkImage_Raster::onMakeWithMipmaps(sk_sp<SkMipmap> mips) const {
    // It's dangerous to have two SkBitmaps that share a SkPixelRef but have different SkMipmaps
    // since various caches key on SkPixelRef's generation ID. Also, SkPixelRefs that back
    // SkSurfaces are marked "temporarily immutable" and making an image that uses the same
    // SkPixelRef can interact badly with SkSurface/SkImage copy-on-write. So we just always
    // make a copy with a new ID.
    if (!mips) {
        mips.reset(SkMipmap::Build(fBitmap.pixmap(), nullptr));
    }
    return SkImage_Raster::MakeFromBitmap(fBitmap, SkCopyPixelsMode::kAlways, std::move(mips));
}

sk_sp<SkImage> SkImage_Raster::makeColorTypeAndColorSpace(SkRecorder*,
                                                          SkColorType targetColorType,
                                                          sk_sp<SkColorSpace> targetColorSpace,
                                                          RequiredProperties) const {
    SkPixmap src;
    SkAssertResult(fBitmap.peekPixels(&src));

    SkBitmap dst;
    if (!dst.tryAllocPixels(
                fBitmap.info().makeColorType(targetColorType).makeColorSpace(targetColorSpace))) {
        return nullptr;
    }

    SkAssertResult(dst.writePixels(src));
    dst.setImmutable();
    return SkImages::RasterFromBitmap(dst);
}

sk_sp<SkImage> SkImage_Raster::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // TODO: If our bitmap is immutable, then we could theoretically create another image sharing
    // our pixelRef. That doesn't work (without more invasive logic), because the image gets its
    // gen ID from the bitmap, which gets it from the pixelRef.
    SkPixmap pixmap = fBitmap.pixmap();
    pixmap.setColorSpace(std::move(newCS));
    return SkImages::RasterFromPixmapCopy(pixmap);
}
