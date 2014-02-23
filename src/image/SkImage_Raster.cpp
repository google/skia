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
#include "SkMallocPixelRef.h"

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
        if ((unsigned)info.fColorType > (unsigned)kLastEnum_SkColorType) {
            return false;
        }
        if ((unsigned)info.fAlphaType > (unsigned)kLastEnum_SkAlphaType) {
            return false;
        }

        if (SkImageInfoToBitmapConfig(info) == SkBitmap::kNo_Config) {
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

    SkImage_Raster(const SkImageInfo&, SkData*, size_t rb);
    virtual ~SkImage_Raster();

    virtual void onDraw(SkCanvas*, SkScalar, SkScalar, const SkPaint*) SK_OVERRIDE;
    virtual void onDrawRectToRect(SkCanvas*, const SkRect*, const SkRect&, const SkPaint*) SK_OVERRIDE;
    virtual bool onReadPixels(SkBitmap*, const SkIRect&) const SK_OVERRIDE;
    virtual const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const SK_OVERRIDE;
    virtual bool getROPixels(SkBitmap*) const SK_OVERRIDE;

    // exposed for SkSurface_Raster via SkNewImageFromPixelRef
    SkImage_Raster(const SkImageInfo&, SkPixelRef*, size_t rowBytes);

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

static void release_data(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkData* data, size_t rowBytes)
    : INHERITED(info.fWidth, info.fHeight)
{
    data->ref();
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, release_data, data);
    fBitmap.setImmutable();
    fBitmap.lockPixels();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkPixelRef* pr, size_t rowBytes)
    : INHERITED(info.fWidth, info.fHeight)
{
    fBitmap.setConfig(info, rowBytes);
    fBitmap.setPixelRef(pr);
    fBitmap.lockPixels();
}

SkImage_Raster::~SkImage_Raster() {}

void SkImage_Raster::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

void SkImage_Raster::onDrawRectToRect(SkCanvas* canvas, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint) {
    canvas->drawBitmapRectToRect(fBitmap, src, dst, paint);
}

bool SkImage_Raster::onReadPixels(SkBitmap* dst, const SkIRect& subset) const {
    if (dst->pixelRef()) {
        return this->INHERITED::onReadPixels(dst, subset);
    } else {
        SkBitmap src;
        if (!fBitmap.extractSubset(&src, subset)) {
            return false;
        }
        return src.copyTo(dst, src.colorType());
    }
}

const void* SkImage_Raster::onPeekPixels(SkImageInfo* infoPtr,
                                         size_t* rowBytesPtr) const {
    SkImageInfo info;
    if (!fBitmap.asImageInfo(&info) || !fBitmap.getPixels()) {
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


SkImage* SkImage::NewRasterData(const SkImageInfo& info, SkData* data, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes)) {
        return NULL;
    }
    if (0 == info.fWidth && 0 == info.fHeight) {
        return SkImage_Raster::NewEmpty();
    }
    // check this after empty-check
    if (NULL == data) {
        return NULL;
    }

    // did they give us enough data?
    size_t size = info.fHeight * rowBytes;
    if (data->size() < size) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Raster, (info, data, rowBytes));
}

SkImage* SkNewImageFromPixelRef(const SkImageInfo& info, SkPixelRef* pr,
                                size_t rowBytes) {
    return SkNEW_ARGS(SkImage_Raster, (info, pr, rowBytes));
}

SkPixelRef* SkBitmapImageGetPixelRef(SkImage* image) {
    return ((SkImage_Raster*)image)->getPixelRef();
}
