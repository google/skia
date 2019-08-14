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
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkTLazy.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkBitmapProcShader.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/GrTextureAdjuster.h"
#include "src/gpu/SkGr.h"
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

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY, CachingHint) const override;
    bool onPeekPixels(SkPixmap*) const override;
    const SkBitmap* onPeekBitmap() const override { return &fBitmap; }

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext*, const GrSamplerState&,
                                            SkScalar scaleAdjust[2]) const override;
#endif

    bool getROPixels(SkBitmap*, CachingHint) const override;
    sk_sp<SkImage> onMakeSubset(GrRecordingContext*, const SkIRect&) const override;

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool onAsLegacyBitmap(SkBitmap*) const override;

    SkImage_Raster(const SkBitmap& bm, bool bitmapMayBeMutable = false)
            : INHERITED(bm.info(),
                        is_not_subset(bm) ? bm.getGenerationID() : (uint32_t)kNeedNewImageUniqueID)
            , fBitmap(bm) {
        SkASSERT(bitmapMayBeMutable || fBitmap.isImmutable());
    }

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                SkColorType, sk_sp<SkColorSpace>) const override;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    bool onIsValid(GrContext* context) const override { return true; }
    void notifyAddedToRasterCache() const override {
        // We explicitly DON'T want to call INHERITED::notifyAddedToRasterCache. That ties the
        // lifetime of derived/cached resources to the image. In this case, we only want cached
        // data (eg mips) tied to the lifetime of the underlying pixelRef.
        SkASSERT(fBitmap.pixelRef());
        fBitmap.pixelRef()->notifyAddedToCache();
    }

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> refPinnedTextureProxy(GrRecordingContext*,
                                                uint32_t* uniqueID) const override;
    bool onPinAsTexture(GrContext*) const override;
    void onUnpinAsTexture(GrContext*) const override;
#endif

private:
    SkBitmap fBitmap;

#if SK_SUPPORT_GPU
    mutable sk_sp<GrTextureProxy> fPinnedProxy;
    mutable int32_t fPinnedCount = 0;
    mutable uint32_t fPinnedUniqueID = 0;
#endif

    typedef SkImage_Base INHERITED;
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
    SkASSERT(nullptr == fPinnedProxy.get());  // want the caller to have manually unpinned
#endif
}

bool SkImage_Raster::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                                  int srcX, int srcY, CachingHint) const {
    SkBitmap shallowCopy(fBitmap);
    return shallowCopy.readPixels(dstInfo, dstPixels, dstRowBytes, srcX, srcY);
}

bool SkImage_Raster::onPeekPixels(SkPixmap* pm) const {
    return fBitmap.peekPixels(pm);
}

bool SkImage_Raster::getROPixels(SkBitmap* dst, CachingHint) const {
    *dst = fBitmap;
    return true;
}

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkImage_Raster::asTextureProxyRef(GrRecordingContext* context,
                                                        const GrSamplerState& params,
                                                        SkScalar scaleAdjust[2]) const {
    if (!context) {
        return nullptr;
    }

    uint32_t uniqueID;
    sk_sp<GrTextureProxy> tex = this->refPinnedTextureProxy(context, &uniqueID);
    if (tex) {
        GrTextureAdjuster adjuster(context, fPinnedProxy,
                                   SkColorTypeToGrColorType(fBitmap.colorType()),
                                   fBitmap.alphaType(), fPinnedUniqueID, fBitmap.colorSpace());
        return adjuster.refTextureProxyForParams(params, scaleAdjust);
    }

    return GrRefCachedBitmapTextureProxy(context, fBitmap, params, scaleAdjust);
}
#endif

#if SK_SUPPORT_GPU

sk_sp<GrTextureProxy> SkImage_Raster::refPinnedTextureProxy(GrRecordingContext*,
                                                            uint32_t* uniqueID) const {
    if (fPinnedProxy) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
        *uniqueID = fPinnedUniqueID;
        return fPinnedProxy;
    }
    return nullptr;
}

bool SkImage_Raster::onPinAsTexture(GrContext* ctx) const {
    if (fPinnedProxy) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
    } else {
        SkASSERT(fPinnedCount == 0);
        SkASSERT(fPinnedUniqueID == 0);
        fPinnedProxy = GrRefCachedBitmapTextureProxy(ctx, fBitmap, GrSamplerState::ClampNearest(),
                                                     nullptr);
        if (!fPinnedProxy) {
            return false;
        }
        fPinnedUniqueID = fBitmap.getGenerationID();
    }
    // Note: we only increment if the texture was successfully pinned
    ++fPinnedCount;
    return true;
}

void SkImage_Raster::onUnpinAsTexture(GrContext* ctx) const {
    // Note: we always decrement, even if fPinnedTexture is null
    SkASSERT(fPinnedCount > 0);
    SkASSERT(fPinnedUniqueID != 0);

    if (0 == --fPinnedCount) {
        fPinnedProxy.reset(nullptr);
        fPinnedUniqueID = 0;
    }
}
#endif

sk_sp<SkImage> SkImage_Raster::onMakeSubset(GrRecordingContext*, const SkIRect& subset) const {
    SkImageInfo info = fBitmap.info().makeWH(subset.width(), subset.height());
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
    return MakeFromBitmap(bitmap);
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

bool SkImage_Raster::onAsLegacyBitmap(SkBitmap* bitmap) const {
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
    return this->INHERITED::onAsLegacyBitmap(bitmap);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Raster::onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                            SkColorType targetCT,
                                                            sk_sp<SkColorSpace> targetCS) const {
    SkPixmap src;
    SkAssertResult(fBitmap.peekPixels(&src));

    SkBitmap dst;
    dst.allocPixels(fBitmap.info().makeColorType(targetCT).makeColorSpace(targetCS));

    SkAssertResult(dst.writePixels(src));
    dst.setImmutable();
    return SkImage::MakeFromBitmap(dst);
}

sk_sp<SkImage> SkImage_Raster::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // TODO: If our bitmap is immutable, then we could theoretically create another image sharing
    // our pixelRef. That doesn't work (without more invasive logic), because the image gets its
    // gen ID from the bitmap, which gets it from the pixelRef.
    SkPixmap pixmap = fBitmap.pixmap();
    pixmap.setColorSpace(std::move(newCS));
    return SkImage::MakeRasterCopy(pixmap);
}
