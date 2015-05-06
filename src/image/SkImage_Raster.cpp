/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkSurface.h"

class SkImage_Raster : public SkImage_Base {
public:
    static bool ValidArgs(const Info& info, size_t rowBytes) {
        const int maxDimension = SK_MaxS32 >> 2;
        const size_t kMaxPixelByteSize = SK_MaxS32;

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

        // TODO: check colorspace

        if (rowBytes < SkImageMinRowBytes(info)) {
            return false;
        }

        int64_t size = (int64_t)info.height() * rowBytes;
        if (size > (int64_t)kMaxPixelByteSize) {
            return false;
        }
        return true;
    }

    SkImage_Raster(const SkImageInfo&, SkData*, size_t rb, const SkSurfaceProps*);
    virtual ~SkImage_Raster();

    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) const override;
    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY) const override;
    const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const override;
    bool getROPixels(SkBitmap*) const override;

    // exposed for SkSurface_Raster via SkNewImageFromPixelRef
    SkImage_Raster(const SkImageInfo&, SkPixelRef*, const SkIPoint& pixelRefOrigin, size_t rowBytes,
                   const SkSurfaceProps*);

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    SkShader* onNewShader(SkShader::TileMode,
                          SkShader::TileMode,
                          const SkMatrix* localMatrix) const override;

    bool isOpaque() const override;

    SkImage_Raster(const SkBitmap& bm, const SkSurfaceProps* props)
        : INHERITED(bm.width(), bm.height(), props)
        , fBitmap(bm) {}

private:
    SkImage_Raster() : INHERITED(0, 0, NULL) {}

    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static void release_data(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkData* data, size_t rowBytes,
                               const SkSurfaceProps* props)
    : INHERITED(info.width(), info.height(), props)
{
    data->ref();
    void* addr = const_cast<void*>(data->data());
    SkColorTable* ctable = NULL;

    fBitmap.installPixels(info, addr, rowBytes, ctable, release_data, data);
    fBitmap.setImmutable();
    fBitmap.lockPixels();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkPixelRef* pr, const SkIPoint& pixelRefOrigin,
                               size_t rowBytes,  const SkSurfaceProps* props)
    : INHERITED(info.width(), info.height(), props)
{
    fBitmap.setInfo(info, rowBytes);
    fBitmap.setPixelRef(pr, pixelRefOrigin);
    fBitmap.lockPixels();
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
        return NULL;
    }
    *infoPtr = info;
    *rowBytesPtr = fBitmap.rowBytes();
    return fBitmap.getPixels();
}

bool SkImage_Raster::getROPixels(SkBitmap* dst) const {
    *dst = fBitmap;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewRasterCopy(const SkImageInfo& info, const void* pixels, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes) || !pixels) {
        return NULL;
    }

    // Here we actually make a copy of the caller's pixel data
    SkAutoDataUnref data(SkData::NewWithCopy(pixels, info.height() * rowBytes));
    return SkNEW_ARGS(SkImage_Raster, (info, data, rowBytes, NULL));
}


SkImage* SkImage::NewRasterData(const SkImageInfo& info, SkData* data, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes) || !data) {
        return NULL;
    }

    // did they give us enough data?
    size_t size = info.height() * rowBytes;
    if (data->size() < size) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Raster, (info, data, rowBytes, NULL));
}

SkImage* SkImage::NewFromGenerator(SkImageGenerator* generator) {
    SkBitmap bitmap;
    if (!SkInstallDiscardablePixelRef(generator, &bitmap)) {
        return NULL;
    }
    if (0 == bitmap.width() || 0 == bitmap.height()) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Raster, (bitmap, NULL));
}

SkImage* SkNewImageFromPixelRef(const SkImageInfo& info, SkPixelRef* pr,
                                const SkIPoint& pixelRefOrigin, size_t rowBytes,
                                const SkSurfaceProps* props) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes)) {
        return NULL;
    }
    return SkNEW_ARGS(SkImage_Raster, (info, pr, pixelRefOrigin, rowBytes, props));
}

SkImage* SkNewImageFromBitmap(const SkBitmap& bm, bool canSharePixelRef,
                              const SkSurfaceProps* props) {
    if (!SkImage_Raster::ValidArgs(bm.info(), bm.rowBytes())) {
        return NULL;
    }

    SkImage* image = NULL;
    if (canSharePixelRef || bm.isImmutable()) {
        image = SkNEW_ARGS(SkImage_Raster, (bm, props));
    } else {
        bm.lockPixels();
        if (bm.getPixels()) {
            image = SkImage::NewRasterCopy(bm.info(), bm.getPixels(), bm.rowBytes());
        }
        bm.unlockPixels();

        // we don't expose props to NewRasterCopy (need a private vers) so post-init it here
        if (image && props) {
            as_IB(image)->initWithProps(*props);
        }
    }
    return image;
}

const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* image) {
    return ((const SkImage_Raster*)image)->getPixelRef();
}

bool SkImage_Raster::isOpaque() const {
    return fBitmap.isOpaque();
}

