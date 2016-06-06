/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorTable.h"
#include "SkData.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
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

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY, CachingHint) const override;
    bool onPeekPixels(SkPixmap*) const override;
    SkData* onRefEncoded(GrContext*) const override;
    bool getROPixels(SkBitmap*, CachingHint) const override;
    GrTexture* asTextureRef(GrContext*, const GrTextureParams&,
                            SkSourceGammaTreatment) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;

    // exposed for SkSurface_Raster via SkNewImageFromPixelRef
    SkImage_Raster(const SkImageInfo&, SkPixelRef*, const SkIPoint& origin, size_t rowBytes);

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool isOpaque() const override;
    bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const override;

    SkImage_Raster(const SkBitmap& bm)
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
        SkASSERT(fBitmap.isImmutable());
    }

    bool onIsLazyGenerated() const override {
        return fBitmap.pixelRef() && fBitmap.pixelRef()->isLazyGenerated();
    }

private:
    SkBitmap fBitmap;

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
    fBitmap.setPixelRef(pr, pixelRefOrigin);
    fBitmap.lockPixels();
    SkASSERT(fBitmap.isImmutable());
}

SkImage_Raster::~SkImage_Raster() {}

bool SkImage_Raster::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                                  int srcX, int srcY, CachingHint) const {
    SkBitmap shallowCopy(fBitmap);
    return shallowCopy.readPixels(dstInfo, dstPixels, dstRowBytes, srcX, srcY);
}

bool SkImage_Raster::onPeekPixels(SkPixmap* pm) const {
    return fBitmap.peekPixels(pm);
}

SkData* SkImage_Raster::onRefEncoded(GrContext*) const {
    SkPixelRef* pr = fBitmap.pixelRef();
    const SkImageInfo prInfo = pr->info();
    const SkImageInfo bmInfo = fBitmap.info();

    // we only try if we (the image) cover the entire area of the pixelRef
    if (prInfo.width() == bmInfo.width() && prInfo.height() == bmInfo.height()) {
        return pr->refEncodedData();
    }
    return nullptr;
}

bool SkImage_Raster::getROPixels(SkBitmap* dst, CachingHint) const {
    *dst = fBitmap;
    return true;
}

GrTexture* SkImage_Raster::asTextureRef(GrContext* ctx, const GrTextureParams& params,
                                        SkSourceGammaTreatment gammaTreatment) const {
#if SK_SUPPORT_GPU
    if (!ctx) {
        return nullptr;
    }

    return GrRefCachedBitmapTexture(ctx, fBitmap, params, gammaTreatment);
#endif

    return nullptr;
}

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
    sk_sp<SkData> data(SkData::NewWithCopy(pmap.addr(), size));
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

    sk_sp<SkData> data(SkData::NewWithProc(pmap.addr(), size, proc, ctx));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes(), pmap.ctable());
}

sk_sp<SkImage> SkMakeImageFromPixelRef(const SkImageInfo& info, SkPixelRef* pr,
                                       const SkIPoint& pixelRefOrigin, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes, false, nullptr)) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Raster>(info, pr, pixelRefOrigin, rowBytes);
}

sk_sp<SkImage> SkMakeImageFromRasterBitmap(const SkBitmap& bm, ForceCopyMode forceCopy) {
    SkASSERT(nullptr == bm.getTexture());

    bool hasColorTable = false;
    if (kIndex_8_SkColorType == bm.colorType()) {
        SkAutoLockPixels autoLockPixels(bm);
        hasColorTable = bm.getColorTable() != nullptr;
    }

    if (!SkImage_Raster::ValidArgs(bm.info(), bm.rowBytes(), hasColorTable, nullptr)) {
        return nullptr;
    }

    sk_sp<SkImage> image;
    if (kYes_ForceCopyMode == forceCopy || !bm.isImmutable()) {
        SkBitmap tmp(bm);
        tmp.lockPixels();
        SkPixmap pmap;
        if (tmp.getPixels() && tmp.peekPixels(&pmap)) {
            image = SkImage::MakeRasterCopy(pmap);
        }
    } else {
        image = sk_make_sp<SkImage_Raster>(bm);
    }
    return image;
}

const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* image) {
    return ((const SkImage_Raster*)image)->getPixelRef();
}

bool SkImage_Raster::isOpaque() const {
    return fBitmap.isOpaque();
}

bool SkImage_Raster::onAsLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    if (kRO_LegacyBitmapMode == mode) {
        // When we're a snapshot from a surface, our bitmap may not be marked immutable
        // even though logically always we are, but in that case we can't physically share our
        // pixelref since the caller might call setImmutable() themselves
        // (thus changing our state).
        if (fBitmap.isImmutable()) {
            bitmap->setInfo(fBitmap.info(), fBitmap.rowBytes());
            bitmap->setPixelRef(fBitmap.pixelRef(), fBitmap.pixelRefOrigin());
            return true;
        }
    }
    return this->INHERITED::onAsLegacyBitmap(bitmap, mode);
}
