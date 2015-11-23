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

        if (rowBytes < SkImageMinRowBytes(info)) {
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

    SkImage_Raster(const SkImageInfo&, SkData*, size_t rb, SkColorTable*);
    virtual ~SkImage_Raster();

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY, CachingHint) const override;
    const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const override;
    SkData* onRefEncoded() const override;
    bool getROPixels(SkBitmap*, CachingHint) const override;
    GrTexture* asTextureRef(GrContext*, const GrTextureParams&) const override;
    SkImage* onNewSubset(const SkIRect&) const override;

    // exposed for SkSurface_Raster via SkNewImageFromPixelRef
    SkImage_Raster(const SkImageInfo&, SkPixelRef*, const SkIPoint& origin, size_t rowBytes);

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool isOpaque() const override;
    bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const override;

    SkImage_Raster(const SkBitmap& bm)
        : INHERITED(bm.width(), bm.height(), bm.getGenerationID())
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

SkImage_Raster::SkImage_Raster(const Info& info, SkData* data, size_t rowBytes,
                               SkColorTable* ctable)
    : INHERITED(info.width(), info.height(), kNeedNewImageUniqueID)
{
    data->ref();
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, ctable, release_data, data);
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

const void* SkImage_Raster::onPeekPixels(SkImageInfo* infoPtr, size_t* rowBytesPtr) const {
    const SkImageInfo info = fBitmap.info();
    if ((kUnknown_SkColorType == info.colorType()) || !fBitmap.getPixels()) {
        return nullptr;
    }
    *infoPtr = info;
    *rowBytesPtr = fBitmap.rowBytes();
    return fBitmap.getPixels();
}

SkData* SkImage_Raster::onRefEncoded() const {
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

GrTexture* SkImage_Raster::asTextureRef(GrContext* ctx, const GrTextureParams& params) const {
#if SK_SUPPORT_GPU
    if (!ctx) {
        return nullptr;
    }

    return GrRefCachedBitmapTexture(ctx, fBitmap, params);
#endif
    
    return nullptr;
}

SkImage* SkImage_Raster::onNewSubset(const SkIRect& subset) const {
    // TODO : could consider heurist of sharing pixels, if subset is pretty close to complete

    SkImageInfo info = SkImageInfo::MakeN32(subset.width(), subset.height(), fBitmap.alphaType());
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    if (!surface) {
        return nullptr;
    }
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawImage(this, SkIntToScalar(-subset.x()), SkIntToScalar(-subset.y()),
                                    nullptr);
    return surface->newImageSnapshot();
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewRasterCopy(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                                SkColorTable* ctable) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, ctable != nullptr, &size) || !pixels) {
        return nullptr;
    }

    // Here we actually make a copy of the caller's pixel data
    SkAutoDataUnref data(SkData::NewWithCopy(pixels, size));
    return new SkImage_Raster(info, data, rowBytes, ctable);
}


SkImage* SkImage::NewRasterData(const SkImageInfo& info, SkData* data, size_t rowBytes) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, false, &size) || !data) {
        return nullptr;
    }

    // did they give us enough data?
    if (data->size() < size) {
        return nullptr;
    }

    SkColorTable* ctable = nullptr;
    return new SkImage_Raster(info, data, rowBytes, ctable);
}

SkImage* SkImage::NewFromRaster(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                                RasterReleaseProc proc, ReleaseContext ctx) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, false, &size) || !pixels) {
        return nullptr;
    }

    SkColorTable* ctable = nullptr;
    SkAutoDataUnref data(SkData::NewWithProc(pixels, size, proc, ctx));
    return new SkImage_Raster(info, data, rowBytes, ctable);
}

SkImage* SkNewImageFromPixelRef(const SkImageInfo& info, SkPixelRef* pr,
                                const SkIPoint& pixelRefOrigin, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes, false, nullptr)) {
        return nullptr;
    }
    return new SkImage_Raster(info, pr, pixelRefOrigin, rowBytes);
}

SkImage* SkNewImageFromRasterBitmap(const SkBitmap& bm, ForceCopyMode forceCopy) {
    SkASSERT(nullptr == bm.getTexture());

    bool hasColorTable = false;
    if (kIndex_8_SkColorType == bm.colorType()) {
        SkAutoLockPixels autoLockPixels(bm);
        hasColorTable = bm.getColorTable() != nullptr;
    }

    if (!SkImage_Raster::ValidArgs(bm.info(), bm.rowBytes(), hasColorTable, nullptr)) {
        return nullptr;
    }

    SkImage* image = nullptr;
    if (kYes_ForceCopyMode == forceCopy || !bm.isImmutable()) {
        SkBitmap tmp(bm);
        tmp.lockPixels();
        if (tmp.getPixels()) {
            image = SkImage::NewRasterCopy(tmp.info(), tmp.getPixels(), tmp.rowBytes(),
                                           tmp.getColorTable());
        }
    } else {
        image = new SkImage_Raster(bm);
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
