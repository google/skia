/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDataPixelRef.h"

class SkImage_Raster : public SkImage_Base {
public:
    static bool ValidArgs(const Info& info, size_t rowBytes) {
        const int maxDimension = SK_MaxS32 >> 2;
        const size_t kMaxPixelByteSize = SK_MaxS32;

        if (info.fWidth < 0 || info.fHeight < 0) {
            return false;
        }
        if (info.fWidth > maxDimension || info.fHeight > maxDimension) {
            return false;
        }
        if ((unsigned)info.fColorType > (unsigned)kLastEnum_ColorType) {
            return false;
        }
        if ((unsigned)info.fAlphaType > (unsigned)kLastEnum_AlphaType) {
            return false;
        }

        bool isOpaque;
        if (SkImageInfoToBitmapConfig(info, &isOpaque) == SkBitmap::kNo_Config) {
            return false;
        }

        // TODO: check colorspace

        if (rowBytes < SkImageMinRowBytes(info)) {
            return false;
        }

        int64_t size = (int64_t)info.fHeight * rowBytes;
        if (size > (int64_t)kMaxPixelByteSize) {
            return false;
        }
        return true;
    }

    static SkImage* NewEmpty();

    SkImage_Raster(const SkImage::Info&, SkData*, size_t rb);
    virtual ~SkImage_Raster();

    virtual void onDraw(SkCanvas*, SkScalar, SkScalar, const SkPaint*) SK_OVERRIDE;

    // exposed for SkSurface_Raster via SkNewImageFromPixelRef
    SkImage_Raster(const SkImage::Info&, SkPixelRef*, size_t rowBytes);

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

private:
    SkImage_Raster() : INHERITED(0, 0) {}

    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage_Raster::NewEmpty() {
    // Returns lazily created singleton
    static SkImage* gEmpty;
    if (NULL == gEmpty) {
        gEmpty = SkNEW(SkImage_Raster);
    }
    gEmpty->ref();
    return gEmpty;
}

SkImage_Raster::SkImage_Raster(const Info& info, SkData* data, size_t rowBytes)
        : INHERITED(info.fWidth, info.fHeight) {
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    fBitmap.setConfig(config, info.fWidth, info.fHeight, rowBytes);
    fBitmap.setPixelRef(SkNEW_ARGS(SkDataPixelRef, (data)))->unref();
    fBitmap.setIsOpaque(isOpaque);
    fBitmap.setImmutable();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkPixelRef* pr, size_t rowBytes)
: INHERITED(info.fWidth, info.fHeight) {
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    fBitmap.setConfig(config, info.fWidth, info.fHeight, rowBytes);
    fBitmap.setPixelRef(pr);
    fBitmap.setIsOpaque(isOpaque);
    fBitmap.setImmutable();
}

SkImage_Raster::~SkImage_Raster() {}

void SkImage_Raster::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewRasterCopy(const SkImage::Info& info, const void* pixels, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes)) {
        return NULL;
    }
    if (0 == info.fWidth && 0 == info.fHeight) {
        return SkImage_Raster::NewEmpty();
    }
    // check this after empty-check
    if (NULL == pixels) {
        return NULL;
    }

    // Here we actually make a copy of the caller's pixel data
    SkAutoDataUnref data(SkData::NewWithCopy(pixels, info.fHeight * rowBytes));
    return SkNEW_ARGS(SkImage_Raster, (info, data, rowBytes));
}


SkImage* SkImage::NewRasterData(const SkImage::Info& info, SkData* pixelData, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes)) {
        return NULL;
    }
    if (0 == info.fWidth && 0 == info.fHeight) {
        return SkImage_Raster::NewEmpty();
    }
    // check this after empty-check
    if (NULL == pixelData) {
        return NULL;
    }

    // did they give us enough data?
    size_t size = info.fHeight * rowBytes;
    if (pixelData->size() < size) {
        return NULL;
    }

    SkAutoDataUnref data(pixelData);
    return SkNEW_ARGS(SkImage_Raster, (info, data, rowBytes));
}

SkImage* SkNewImageFromPixelRef(const SkImage::Info& info, SkPixelRef* pr,
                                size_t rowBytes) {
    return SkNEW_ARGS(SkImage_Raster, (info, pr, rowBytes));
}

SkPixelRef* SkBitmapImageGetPixelRef(SkImage* image) {
    return ((SkImage_Raster*)image)->getPixelRef();
}
