/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkBitmap.h"
#include "SkBitmapProcShader.h"
#include "SkCanvas.h"
#include "SkColorTable.h"
#include "SkData.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTextureAdjuster.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#endif

// fixes https://bug.skia.org/5096
static bool is_not_subset(const SkBitmap& bm) {
    SkASSERT(bm.pixelRef());
    SkISize dim = bm.pixelRef()->info().dimensions();
    SkASSERT(dim != bm.dimensions() || bm.pixelRefOrigin().isZero());
    return dim == bm.dimensions();
}

class SkImage_Raster : public SkImage_Base {
public:
    static bool ValidArgs(const Info& info, size_t rowBytes, bool hasColorTable,
                          size_t* minSize) {
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

        const bool needsCT = kIndex_8_SkColorType == info.colorType();
        if (needsCT != hasColorTable) {
            return false;
        }

        if (rowBytes < info.minRowBytes()) {
            return false;
        }

        size_t size = info.getSafeSize(rowBytes);
        if (0 == size) {
            return false;
        }

        if (minSize) {
            *minSize = size;
        }
        return true;
    }

    SkImage_Raster(const SkImageInfo&, sk_sp<SkData>, size_t rb, SkColorTable*);
    virtual ~SkImage_Raster();

    SkImageInfo onImageInfo() const override {
        return fBitmap.info();
    }
    SkAlphaType onAlphaType() const override {
        return fBitmap.alphaType();
    }

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY, CachingHint) const override;
    bool onPeekPixels(SkPixmap*) const override;
    const SkBitmap* onPeekBitmap() const override { return &fBitmap; }

    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    GrTexture* asTextureRef(GrContext*, const GrSamplerParams&, SkColorSpace*,
                            sk_sp<SkColorSpace>*) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;

    // exposed for SkSurface_Raster via SkNewImageFromPixelRef
    SkImage_Raster(const SkImageInfo&, SkPixelRef*, const SkIPoint& origin, size_t rowBytes);

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const override;

    SkImage_Raster(const SkBitmap& bm, bool bitmapMayBeMutable = false)
        : INHERITED(bm.width(), bm.height(),
                    is_not_subset(bm) ? bm.getGenerationID()
                                      : (uint32_t)kNeedNewImageUniqueID)
        , fBitmap(bm)
    {
        if (bm.pixelRef()->isPreLocked()) {
            // we only preemptively lock if there is no chance of triggering something expensive
            // like a lazy decode or imagegenerator. PreLocked means it is flat pixels already.
            fBitmap.lockPixels();
        }
        SkASSERT(bitmapMayBeMutable || fBitmap.isImmutable());
    }

    bool onIsLazyGenerated() const override {
        return fBitmap.pixelRef() && fBitmap.pixelRef()->isLazyGenerated();
    }

#if SK_SUPPORT_GPU
    sk_sp<GrTexture> refPinnedTexture(uint32_t* uniqueID) const override;
    bool onPinAsTexture(GrContext*) const override;
    void onUnpinAsTexture(GrContext*) const override;
#endif

private:
    SkBitmap fBitmap;

#if SK_SUPPORT_GPU
    mutable sk_sp<GrTexture> fPinnedTexture;
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

SkImage_Raster::SkImage_Raster(const Info& info, sk_sp<SkData> data, size_t rowBytes,
                               SkColorTable* ctable)
    : INHERITED(info.width(), info.height(), kNeedNewImageUniqueID)
{
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, ctable, release_data, data.release());
    fBitmap.setImmutable();
    fBitmap.lockPixels();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkPixelRef* pr, const SkIPoint& pixelRefOrigin,
                               size_t rowBytes)
    : INHERITED(info.width(), info.height(), pr->getGenerationID())
{
    fBitmap.setInfo(info, rowBytes);
    fBitmap.setPixelRef(sk_ref_sp(pr), pixelRefOrigin.x(), pixelRefOrigin.y());
    fBitmap.lockPixels();
    SkASSERT(fBitmap.isImmutable());
}

SkImage_Raster::~SkImage_Raster() {
#if SK_SUPPORT_GPU
    SkASSERT(nullptr == fPinnedTexture.get());  // want the caller to have manually unpinned
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

bool SkImage_Raster::getROPixels(SkBitmap* dst, SkColorSpace* dstColorSpace, CachingHint) const {
    *dst = fBitmap;
    return true;
}

GrTexture* SkImage_Raster::asTextureRef(GrContext* ctx, const GrSamplerParams& params,
                                        SkColorSpace* dstColorSpace,
                                        sk_sp<SkColorSpace>* texColorSpace) const {
#if SK_SUPPORT_GPU
    if (!ctx) {
        return nullptr;
    }

    if (texColorSpace) {
        *texColorSpace = sk_ref_sp(fBitmap.colorSpace());
    }

    uint32_t uniqueID;
    sk_sp<GrTexture> tex = this->refPinnedTexture(&uniqueID);
    if (tex) {
        GrTextureAdjuster adjuster(fPinnedTexture.get(), fBitmap.alphaType(), fBitmap.bounds(),
                                   fPinnedUniqueID, fBitmap.colorSpace());
        return adjuster.refTextureSafeForParams(params, nullptr);
    }

    return GrRefCachedBitmapTexture(ctx, fBitmap, params);
#endif

    return nullptr;
}

#if SK_SUPPORT_GPU

sk_sp<GrTexture> SkImage_Raster::refPinnedTexture(uint32_t* uniqueID) const {
    if (fPinnedTexture) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
        *uniqueID = fPinnedUniqueID;
        return fPinnedTexture;
    }
    return nullptr;
}

bool SkImage_Raster::onPinAsTexture(GrContext* ctx) const {
    if (fPinnedTexture) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
        SkASSERT(fPinnedTexture->getContext() == ctx);
    } else {
        SkASSERT(fPinnedCount == 0);
        SkASSERT(fPinnedUniqueID == 0);
        fPinnedTexture.reset(
            GrRefCachedBitmapTexture(ctx, fBitmap, GrSamplerParams::ClampNoFilter()));
        if (!fPinnedTexture) {
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
    if (fPinnedTexture) {
        SkASSERT(fPinnedTexture->getContext() == ctx);
    }

    if (0 == --fPinnedCount) {
        fPinnedTexture.reset(nullptr);
        fPinnedUniqueID = 0;
    }
}
#endif

sk_sp<SkImage> SkImage_Raster::onMakeSubset(const SkIRect& subset) const {
    // TODO : could consider heurist of sharing pixels, if subset is pretty close to complete

    SkImageInfo info = SkImageInfo::MakeN32(subset.width(), subset.height(), fBitmap.alphaType());
    auto surface(SkSurface::MakeRaster(info));
    if (!surface) {
        return nullptr;
    }
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawImage(this, SkIntToScalar(-subset.x()), SkIntToScalar(-subset.y()),
                                    nullptr);
    return surface->makeImageSnapshot();
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeRasterCopy(const SkPixmap& pmap) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(pmap.info(), pmap.rowBytes(),
                                   pmap.ctable() != nullptr, &size) || !pmap.addr()) {
        return nullptr;
    }

    // Here we actually make a copy of the caller's pixel data
    sk_sp<SkData> data(SkData::MakeWithCopy(pmap.addr(), size));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes(), pmap.ctable());
}


sk_sp<SkImage> SkImage::MakeRasterData(const SkImageInfo& info, sk_sp<SkData> data,
                                       size_t rowBytes) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, false, &size) || !data) {
        return nullptr;
    }

    // did they give us enough data?
    if (data->size() < size) {
        return nullptr;
    }

    SkColorTable* ctable = nullptr;
    return sk_make_sp<SkImage_Raster>(info, std::move(data), rowBytes, ctable);
}

sk_sp<SkImage> SkImage::MakeFromRaster(const SkPixmap& pmap, RasterReleaseProc proc,
                                       ReleaseContext ctx) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(pmap.info(), pmap.rowBytes(), false, &size) || !pmap.addr()) {
        return nullptr;
    }

    sk_sp<SkData> data(SkData::MakeWithProc(pmap.addr(), size, proc, ctx));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes(), pmap.ctable());
}

sk_sp<SkImage> SkMakeImageFromPixelRef(const SkImageInfo& info, SkPixelRef* pr,
                                       const SkIPoint& pixelRefOrigin, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes, false, nullptr)) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Raster>(info, pr, pixelRefOrigin, rowBytes);
}

sk_sp<SkImage> SkMakeImageFromRasterBitmap(const SkBitmap& bm, SkCopyPixelsMode cpm,
                                           SkTBlitterAllocator* allocator) {
    bool hasColorTable = false;
    if (kIndex_8_SkColorType == bm.colorType()) {
        SkAutoLockPixels autoLockPixels(bm);
        hasColorTable = bm.getColorTable() != nullptr;
    }

    if (!SkImage_Raster::ValidArgs(bm.info(), bm.rowBytes(), hasColorTable, nullptr)) {
        return nullptr;
    }

    sk_sp<SkImage> image;
    if (kAlways_SkCopyPixelsMode == cpm || (!bm.isImmutable() && kNever_SkCopyPixelsMode != cpm)) {
        SkBitmap tmp(bm);
        tmp.lockPixels();
        SkPixmap pmap;
        if (tmp.getPixels() && tmp.peekPixels(&pmap)) {
            image = SkImage::MakeRasterCopy(pmap);
        }
    } else {
        if (allocator) {
            image.reset(allocator->createT<SkImage_Raster>(bm, kNever_SkCopyPixelsMode == cpm));
            image.get()->ref(); // account for the allocator being an owner
        } else {
            image = sk_make_sp<SkImage_Raster>(bm, kNever_SkCopyPixelsMode == cpm);
        }
    }
    return image;
}

const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* image) {
    return ((const SkImage_Raster*)image)->getPixelRef();
}

bool SkImage_Raster::onAsLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    if (kRO_LegacyBitmapMode == mode) {
        // When we're a snapshot from a surface, our bitmap may not be marked immutable
        // even though logically always we are, but in that case we can't physically share our
        // pixelref since the caller might call setImmutable() themselves
        // (thus changing our state).
        if (fBitmap.isImmutable()) {
            bitmap->setInfo(fBitmap.info(), fBitmap.rowBytes());
            bitmap->setPixelRef(sk_ref_sp(fBitmap.pixelRef()),
                                fBitmap.pixelRefOrigin().x(),
                                fBitmap.pixelRefOrigin().y());
            return true;
        }
    }
    return this->INHERITED::onAsLegacyBitmap(bitmap, mode);
}
