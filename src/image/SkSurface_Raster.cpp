/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkMallocPixelRef.h"

static const size_t kIgnoreRowBytesValue = (size_t)~0;

class SkSurface_Raster : public SkSurface_Base {
public:
    static bool Valid(const SkImage::Info&, size_t rb = kIgnoreRowBytesValue);

    SkSurface_Raster(const SkImage::Info&, void*, size_t rb);
    SkSurface_Raster(const SkImage::Info&, SkPixelRef*, size_t rb);

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImage::Info&) SK_OVERRIDE;
    virtual SkImage* onNewImageSnapshot() SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;
    virtual void onCopyOnWrite(ContentChangeMode) SK_OVERRIDE;

private:
    SkBitmap    fBitmap;
    bool        fWeOwnThePixels;

    typedef SkSurface_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

bool SkSurface_Raster::Valid(const SkImage::Info& info, size_t rowBytes) {
    static const size_t kMaxTotalSize = SK_MaxS32;

    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    int shift = 0;
    switch (config) {
        case SkBitmap::kA8_Config:
            shift = 0;
            break;
        case SkBitmap::kRGB_565_Config:
            shift = 1;
            break;
        case SkBitmap::kARGB_8888_Config:
            shift = 2;
            break;
        default:
            return false;
    }

    // TODO: examine colorspace

    if (kIgnoreRowBytesValue == rowBytes) {
        return true;
    }

    uint64_t minRB = (uint64_t)info.fWidth << shift;
    if (minRB > rowBytes) {
        return false;
    }

    size_t alignedRowBytes = rowBytes >> shift << shift;
    if (alignedRowBytes != rowBytes) {
        return false;
    }

    uint64_t size = (uint64_t)info.fHeight * rowBytes;
    if (size > kMaxTotalSize) {
        return false;
    }

    return true;
}

SkSurface_Raster::SkSurface_Raster(const SkImage::Info& info, void* pixels, size_t rb)
        : INHERITED(info.fWidth, info.fHeight) {
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    fBitmap.setConfig(config, info.fWidth, info.fHeight, rb);
    fBitmap.setPixels(pixels);
    fBitmap.setIsOpaque(isOpaque);
    fWeOwnThePixels = false;    // We are "Direct"
}

SkSurface_Raster::SkSurface_Raster(const SkImage::Info& info, SkPixelRef* pr, size_t rb)
        : INHERITED(info.fWidth, info.fHeight) {
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    fBitmap.setConfig(config, info.fWidth, info.fHeight, rb);
    fBitmap.setPixelRef(pr);
    fBitmap.setIsOpaque(isOpaque);
    fWeOwnThePixels = true;

    if (!isOpaque) {
        fBitmap.eraseColor(SK_ColorTRANSPARENT);
    }
}

SkCanvas* SkSurface_Raster::onNewCanvas() {
    return SkNEW_ARGS(SkCanvas, (fBitmap));
}

SkSurface* SkSurface_Raster::onNewSurface(const SkImage::Info& info) {
    return SkSurface::NewRaster(info);
}

void SkSurface_Raster::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                              const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

SkImage* SkSurface_Raster::onNewImageSnapshot() {
    return SkNewImageFromBitmap(fBitmap, fWeOwnThePixels);
}

void SkSurface_Raster::onCopyOnWrite(ContentChangeMode mode) {
    // are we sharing pixelrefs with the image?
    SkASSERT(NULL != this->getCachedImage());
    if (SkBitmapImageGetPixelRef(this->getCachedImage()) == fBitmap.pixelRef()) {
        SkASSERT(fWeOwnThePixels);
        if (kDiscard_ContentChangeMode == mode) {
            fBitmap.setPixelRef(NULL, 0);
            fBitmap.allocPixels();
        } else {
            SkBitmap prev(fBitmap);
            prev.deepCopyTo(&fBitmap, prev.config());
        }
        // Now fBitmap is a deep copy of itself (and therefore different from
        // what is being used by the image. Next we update the canvas to use
        // this as its backend, so we can't modify the image's pixels anymore.
        SkASSERT(NULL != this->getCachedCanvas());
        this->getCachedCanvas()->getDevice()->replaceBitmapBackendForRasterSurface(fBitmap);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkSurface::NewRasterDirect(const SkImage::Info& info, void* pixels, size_t rowBytes) {
    if (!SkSurface_Raster::Valid(info, rowBytes)) {
        return NULL;
    }
    if (NULL == pixels) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Raster, (info, pixels, rowBytes));
}

SkSurface* SkSurface::NewRaster(const SkImage::Info& info) {
    if (!SkSurface_Raster::Valid(info)) {
        return NULL;
    }

    static const size_t kMaxTotalSize = SK_MaxS32;
    size_t rowBytes = SkImageMinRowBytes(info);
    uint64_t size64 = (uint64_t)info.fHeight * rowBytes;
    if (size64 > kMaxTotalSize) {
        return NULL;
    }

    size_t size = (size_t)size64;
    void* pixels = sk_malloc_throw(size);
    if (NULL == pixels) {
        return NULL;
    }

    SkAutoTUnref<SkPixelRef> pr(SkNEW_ARGS(SkMallocPixelRef, (pixels, size, NULL, true)));
    return SkNEW_ARGS(SkSurface_Raster, (info, pr, rowBytes));
}
