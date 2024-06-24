/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/image/SkImage_Raster.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class GrDirectContext;
class SkSurfaceProps;

// fixes https://bug.skia.org/5096
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

SkImage_Raster::SkImage_Raster(const SkImageInfo& info, sk_sp<SkData> data, size_t rowBytes,
                               uint32_t id)
        : SkImage_Base(info, id) {
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, release_data, data.release());
    fBitmap.setImmutable();
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

sk_sp<SkSurface> SkImage_Raster::onMakeSurface(skgpu::graphite::Recorder*,
                                               const SkImageInfo& info) const {
    const SkSurfaceProps* props = nullptr;
    const size_t rowBytes = 0;
    return SkSurfaces::Raster(info, rowBytes, props);
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

sk_sp<SkImage> SkImage_Raster::onMakeSubset(GrDirectContext*, const SkIRect& subset) const {
    SkBitmap copy = copy_bitmap_subset(fBitmap, subset);
    if (copy.isNull()) {
        return nullptr;
    } else {
        return copy.asImage();
    }
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

sk_sp<SkImage> SkImage_Raster::onMakeSubset(skgpu::graphite::Recorder*,
                                            const SkIRect& subset,
                                            RequiredProperties requiredProperties) const {
    sk_sp<SkImage> img;

    if (requiredProperties.fMipmapped) {
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

    return img;
}

///////////////////////////////////////////////////////////////////////////////

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
    return SkImages::RasterFromPixmapCopy(pixmap);
}
