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
#include "SkImageGeneratorPriv.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkSurface.h"

class SkImage_Raster : public SkImage_Base {
public:
    static bool ValidArgs(const Info& info, size_t rowBytes, SkColorTable* ctable,
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
        const bool hasCT = nullptr != ctable;
        if (needsCT != hasCT) {
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

    SkImage_Raster(const SkImageInfo&, SkData*, size_t rb, SkColorTable*, const SkSurfaceProps*);
    virtual ~SkImage_Raster();

    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) const override;
    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY) const override;
    void onPreroll(GrContext*) const override;
    const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const override;
    SkData* onRefEncoded() const override;
    bool getROPixels(SkBitmap*) const override;

    // exposed for SkSurface_Raster via SkNewImageFromPixelRef
    SkImage_Raster(const SkImageInfo&, SkPixelRef*, const SkIPoint& pixelRefOrigin, size_t rowBytes,
                   const SkSurfaceProps*);

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    SkShader* onNewShader(SkShader::TileMode,
                          SkShader::TileMode,
                          const SkMatrix* localMatrix) const override;

    bool isOpaque() const override;
    bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const override;

    SkImage_Raster(const SkBitmap& bm, const SkSurfaceProps* props)
        : INHERITED(bm.width(), bm.height(), bm.getGenerationID(), props)
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
    SkImage_Raster() : INHERITED(0, 0, kNeedNewImageUniqueID, nullptr) {
        fBitmap.setImmutable();
    }

    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static void release_data(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkData* data, size_t rowBytes,
                               SkColorTable* ctable, const SkSurfaceProps* props)
    : INHERITED(info.width(), info.height(), kNeedNewImageUniqueID, props)
{
    data->ref();
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, ctable, release_data, data);
    fBitmap.setImmutable();
    fBitmap.lockPixels();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkPixelRef* pr, const SkIPoint& pixelRefOrigin,
                               size_t rowBytes,  const SkSurfaceProps* props)
    : INHERITED(info.width(), info.height(), pr->getGenerationID(), props)
{
    fBitmap.setInfo(info, rowBytes);
    fBitmap.setPixelRef(pr, pixelRefOrigin);
    fBitmap.lockPixels();
    SkASSERT(fBitmap.isImmutable());
}

SkImage_Raster::~SkImage_Raster() {}

SkShader* SkImage_Raster::onNewShader(SkShader::TileMode tileX, SkShader::TileMode tileY,
                                      const SkMatrix* localMatrix) const {
    return SkShader::CreateBitmapShader(fBitmap, tileX, tileY, localMatrix);
}

SkSurface* SkImage_Raster::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) const {
    return SkSurface::NewRaster(info, &props);
}

bool SkImage_Raster::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                                  int srcX, int srcY) const {
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

void SkImage_Raster::onPreroll(GrContext* ctx) const {
    // SkImage can be called from lots of threads, but our fBitmap is *not* thread-safe,
    // so we have to perform this lock/unlock in a non-racy way... we make a copy!
    SkBitmap localShallowCopy(fBitmap);
    localShallowCopy.lockPixels();
    localShallowCopy.unlockPixels();
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

bool SkImage_Raster::getROPixels(SkBitmap* dst) const {
    *dst = fBitmap;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewRasterCopy(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                                SkColorTable* ctable) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, ctable, &size) || !pixels) {
        return nullptr;
    }

    // Here we actually make a copy of the caller's pixel data
    SkAutoDataUnref data(SkData::NewWithCopy(pixels, size));
    return new SkImage_Raster(info, data, rowBytes, ctable, nullptr);
}


SkImage* SkImage::NewRasterData(const SkImageInfo& info, SkData* data, size_t rowBytes) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, nullptr, &size) || !data) {
        return nullptr;
    }

    // did they give us enough data?
    if (data->size() < size) {
        return nullptr;
    }

    SkColorTable* ctable = nullptr;
    return new SkImage_Raster(info, data, rowBytes, ctable, nullptr);
}

SkImage* SkImage::NewFromRaster(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                                RasterReleaseProc proc, ReleaseContext ctx) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, nullptr, &size) || !pixels) {
        return nullptr;
    }

    SkColorTable* ctable = nullptr;
    SkAutoDataUnref data(SkData::NewWithProc(pixels, size, proc, ctx));
    return new SkImage_Raster(info, data, rowBytes, ctable, nullptr);
}

SkImage* SkImage::NewFromGenerator(SkImageGenerator* generator, const SkIRect* subset) {
    SkBitmap bitmap;
    if (!SkInstallDiscardablePixelRef(generator, subset, &bitmap, nullptr)) {
        return nullptr;
    }
    if (0 == bitmap.width() || 0 == bitmap.height()) {
        return nullptr;
    }

    return new SkImage_Raster(bitmap, nullptr);
}

SkImage* SkNewImageFromPixelRef(const SkImageInfo& info, SkPixelRef* pr,
                                const SkIPoint& pixelRefOrigin, size_t rowBytes,
                                const SkSurfaceProps* props) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes, nullptr, nullptr)) {
        return nullptr;
    }
    return new SkImage_Raster(info, pr, pixelRefOrigin, rowBytes, props);
}

SkImage* SkNewImageFromRasterBitmap(const SkBitmap& bm, const SkSurfaceProps* props,
                                    ForceCopyMode forceCopy) {
    SkASSERT(nullptr == bm.getTexture());

    if (!SkImage_Raster::ValidArgs(bm.info(), bm.rowBytes(), nullptr, nullptr)) {
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

        // we don't expose props to NewRasterCopy (need a private vers) so post-init it here
        if (image && props) {
            as_IB(image)->initWithProps(*props);
        }
    } else {
        image = new SkImage_Raster(bm, props);
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
