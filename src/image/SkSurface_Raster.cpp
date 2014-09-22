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
    static bool Valid(const SkImageInfo&, size_t rb = kIgnoreRowBytesValue);

    SkSurface_Raster(const SkImageInfo&, void*, size_t rb,
                     void (*releaseProc)(void* pixels, void* context), void* context,
                     const SkSurfaceProps*);
    SkSurface_Raster(SkPixelRef*, const SkSurfaceProps*);

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImageInfo&) SK_OVERRIDE;
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

bool SkSurface_Raster::Valid(const SkImageInfo& info, size_t rowBytes) {
    static const size_t kMaxTotalSize = SK_MaxS32;

    int shift = 0;
    switch (info.colorType()) {
        case kAlpha_8_SkColorType:
            shift = 0;
            break;
        case kRGB_565_SkColorType:
            shift = 1;
            break;
        case kN32_SkColorType:
            shift = 2;
            break;
        default:
            return false;
    }

    if (kIgnoreRowBytesValue == rowBytes) {
        return true;
    }

    uint64_t minRB = (uint64_t)info.width() << shift;
    if (minRB > rowBytes) {
        return false;
    }

    size_t alignedRowBytes = rowBytes >> shift << shift;
    if (alignedRowBytes != rowBytes) {
        return false;
    }

    uint64_t size = sk_64_mul(info.height(), rowBytes);
    if (size > kMaxTotalSize) {
        return false;
    }

    return true;
}

SkSurface_Raster::SkSurface_Raster(const SkImageInfo& info, void* pixels, size_t rb,
                                   void (*releaseProc)(void* pixels, void* context), void* context,
                                   const SkSurfaceProps* props)
    : INHERITED(info, props)
{
    fBitmap.installPixels(info, pixels, rb, NULL, releaseProc, context);
    fWeOwnThePixels = false;    // We are "Direct"
}

SkSurface_Raster::SkSurface_Raster(SkPixelRef* pr, const SkSurfaceProps* props)
    : INHERITED(pr->info().width(), pr->info().height(), props)
{
    const SkImageInfo& info = pr->info();

    fBitmap.setInfo(info, info.minRowBytes());
    fBitmap.setPixelRef(pr);
    fWeOwnThePixels = true;

    if (!info.isOpaque()) {
        fBitmap.eraseColor(SK_ColorTRANSPARENT);
    }
}

SkCanvas* SkSurface_Raster::onNewCanvas() {
    return SkNEW_ARGS(SkCanvas, (fBitmap, this->props()));
}

SkSurface* SkSurface_Raster::onNewSurface(const SkImageInfo& info) {
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
    SkASSERT(this->getCachedImage());
    if (SkBitmapImageGetPixelRef(this->getCachedImage()) == fBitmap.pixelRef()) {
        SkASSERT(fWeOwnThePixels);
        if (kDiscard_ContentChangeMode == mode) {
            fBitmap.setPixelRef(NULL);
            fBitmap.allocPixels();
        } else {
            SkBitmap prev(fBitmap);
            prev.deepCopyTo(&fBitmap);
        }
        // Now fBitmap is a deep copy of itself (and therefore different from
        // what is being used by the image. Next we update the canvas to use
        // this as its backend, so we can't modify the image's pixels anymore.
        SkASSERT(this->getCachedCanvas());
        this->getCachedCanvas()->getDevice()->replaceBitmapBackendForRasterSurface(fBitmap);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkSurface::NewRasterDirectReleaseProc(const SkImageInfo& info, void* pixels, size_t rb,
                                                 void (*releaseProc)(void* pixels, void* context),
                                                 void* context, const SkSurfaceProps* props) {
    if (NULL == releaseProc) {
        context = NULL;
    }
    if (!SkSurface_Raster::Valid(info, rb)) {
        return NULL;
    }
    if (NULL == pixels) {
        return NULL;
    }
    
    return SkNEW_ARGS(SkSurface_Raster, (info, pixels, rb, releaseProc, context, props));
}

SkSurface* SkSurface::NewRasterDirect(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                      const SkSurfaceProps* props) {
    return NewRasterDirectReleaseProc(info, pixels, rowBytes, NULL, NULL, props);
}

SkSurface* SkSurface::NewRaster(const SkImageInfo& info, const SkSurfaceProps* props) {
    if (!SkSurface_Raster::Valid(info)) {
        return NULL;
    }

    SkAutoTUnref<SkPixelRef> pr(SkMallocPixelRef::NewAllocate(info, 0, NULL));
    if (NULL == pr.get()) {
        return NULL;
    }
    return SkNEW_ARGS(SkSurface_Raster, (pr, props));
}
