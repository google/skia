/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"
#include "SkMallocPixelRef.h"

static const size_t kIgnoreRowBytesValue = (size_t)~0;

class SkSurface_Gpu : public SkSurface_Base {
public:
    static bool Valid(const SkImage::Info&, SkColorSpace*, size_t rb = kIgnoreRowBytesValue);

    SkSurface_Gpu(const SkImage::Info&, SkColorSpace*, void*, size_t rb);
    SkSurface_Gpu(const SkImage::Info&, SkColorSpace*, SkPixelRef*, size_t rb);

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImage::Info&, SkColorSpace*) SK_OVERRIDE;
    virtual SkImage* onNewImageShapshot() SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;

private:
    SkBitmap    fBitmap;
    bool        fWeOwnThePixels;

    typedef SkSurface_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

bool SkSurface_Gpu::Valid(const SkImage::Info& info, SkColorSpace* cs,
                             size_t rowBytes) {
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

SkSurface_Gpu::SkSurface_Gpu(const SkImage::Info& info, SkColorSpace* cs,
                                   void* pixels, size_t rb)
        : INHERITED(info.fWidth, info.fHeight) {
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    fBitmap.setConfig(config, info.fWidth, info.fHeight, rb);
    fBitmap.setPixels(pixels);
    fBitmap.setIsOpaque(isOpaque);
    fWeOwnThePixels = false;
}

SkSurface_Gpu::SkSurface_Gpu(const SkImage::Info& info, SkColorSpace* cs,
                                   SkPixelRef* pr, size_t rb)
        : INHERITED(info.fWidth, info.fHeight) {
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    fBitmap.setConfig(config, info.fWidth, info.fHeight, rb);
    fBitmap.setPixelRef(pr);
    fBitmap.setIsOpaque(isOpaque);
    fWeOwnThePixels = true;

    if (!isOpaque) {
        fBitmap.eraseColor(0);
    }
}

SkCanvas* SkSurface_Gpu::onNewCanvas() {
    return SkNEW_ARGS(SkCanvas, (fBitmap));
}

SkSurface* SkSurface_Gpu::onNewSurface(const SkImage::Info& info,
                                          SkColorSpace* cs) {
    return SkSurface::NewRaster(info, cs);
}

SkImage* SkSurface_Gpu::onNewImageShapshot() {
    // if we don't own the pixels, we need to make a deep-copy
    // if we do, we need to perform a copy-on-write the next time
    // we draw to this bitmap from our canvas...
    return SkNewImageFromBitmap(fBitmap);
}

void SkSurface_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                              const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkSurface::NewRenderTargetDirect(GrContext* ctx,
                                            GrRenderTarget* target) {
    if (NULL == ctx || NULL == target) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Gpu, (ctx, target));
}

SkSurface* SkSurface::NewRenderTarget(GrContext* ctx, const SkImage::Info& info,
                                      SkColorSpace*, int sampleCount) {
    if (NULL == ctx) {
        return NULL;
    }
    if (!SkSurface_Gpu::Valid(info, cs, sampleCount)) {
        return NULL;
    }

//    return SkNEW_ARGS(SkSurface_Gpu, (info, cs, pr, rowBytes));
}

