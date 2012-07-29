#include "SkSurface.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"

///////////////////////////////////////////////////////////////////////////////

class SkSurface_Base : public SkSurface {
public:
    SkSurface_Base(int width, int height) : INHERITED(width, height) {}

    virtual SkCanvas* onNewCanvas() = 0;
    virtual SkSurface* onNewSurface(const SkImage::Info&, SkColorSpace*) = 0;
    virtual SkImage* onNewImageShapshot() = 0;
    
    /**
     *  Default implementation:
     *
     *  image = this->newImageSnapshot();
     *  if (image) {
     *      image->draw(canvas, ...);
     *      image->unref();
     *  }
     */
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*);

private:
    typedef SkSurface INHERITED;
};

void SkSurface_Base::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                            const SkPaint* paint) {
    SkImage* image = this->newImageShapshot();
    if (image) {
        image->draw(canvas, x, y, paint);
        image->unref();
    }
}

static SkSurface_Base* asSB(SkSurface* surface) {
    return static_cast<SkSurface_Base*>(surface);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface::SkSurface(int width, int height) : fWidth(width), fHeight(height) {
    SkASSERT(width >= 0);
    SkASSERT(height >= 0);
    fGenerationID = 0;
}

SkCanvas* SkSurface::newCanvas() {
    return asSB(this)->onNewCanvas();
}

SkSurface* SkSurface::newSurface(const SkImage::Info& info, SkColorSpace* cs) {
    return asSB(this)->onNewSurface(info, cs);
}

SkImage* SkSurface::newImageShapshot() {
    return asSB(this)->onNewImageShapshot();
}

void SkSurface::draw(SkCanvas* canvas, SkScalar x, SkScalar y,
                     const SkPaint* paint) {
    return asSB(this)->onDraw(canvas, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

static const size_t kIgnoreRowBytesValue = (size_t)~0;

class SkSurface_Raster : public SkSurface_Base {
public:
    static bool Valid(const SkImage::Info&, SkColorSpace*, size_t rb = kIgnoreRowBytesValue);

    SkSurface_Raster(const SkImage::Info&, SkColorSpace*, void*, size_t rb);
    SkSurface_Raster(const SkImage::Info&, SkColorSpace*, SkPixelRef*, size_t rb);

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

bool SkSurface_Raster::Valid(const SkImage::Info& info, SkColorSpace* cs,
                             size_t rowBytes) {
    static size_t kMaxTotalSize = (1 << 31) - 1;

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

SkSurface_Raster::SkSurface_Raster(const SkImage::Info& info, SkColorSpace* cs,
                                   void* pixels, size_t rb)
        : INHERITED(info.fWidth, info.fHeight) {
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);
    
    fBitmap.setConfig(config, info.fWidth, info.fHeight, rb);
    fBitmap.setPixels(pixels);
    fBitmap.setIsOpaque(isOpaque);
    fWeOwnThePixels = false;
}

SkSurface_Raster::SkSurface_Raster(const SkImage::Info& info, SkColorSpace* cs,
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

SkCanvas* SkSurface_Raster::onNewCanvas() {
    return SkNEW_ARGS(SkCanvas, (fBitmap));
}

SkSurface* SkSurface_Raster::onNewSurface(const SkImage::Info& info,
                                          SkColorSpace* cs) {
    return SkSurface::NewRaster(info, cs);
}

SkImage* SkSurface_Raster::onNewImageShapshot() {
    // if we don't own the pixels, we need to make a deep-copy
    // if we do, we need to perform a copy-on-write the next time
    // we draw to this bitmap from our canvas...
    return SkNewImageFromBitmap(fBitmap);
}

void SkSurface_Raster::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                              const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkPicture.h"

/**
 *  What does it mean to ask for more than one canvas from a picture?
 *  How do we return an Image and then "continue" recording?
 */
class SkSurface_Picture : public SkSurface_Base {
public:
    SkSurface_Picture(int width, int height);
    virtual ~SkSurface_Picture();
    
    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImage::Info&, SkColorSpace*) SK_OVERRIDE;
    virtual SkImage* onNewImageShapshot() SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;
    
private:
    SkPicture*  fPicture;
    SkPicture*  fRecordingPicture;
    
    typedef SkSurface_Base INHERITED;
};

SkSurface_Picture::SkSurface_Picture(int width, int height) : INHERITED(width, height) {
    fPicture = NULL;
}

SkSurface_Picture::~SkSurface_Picture() {
    SkSafeUnref(fPicture);
}

SkCanvas* SkSurface_Picture::onNewCanvas() {
    if (!fPicture) {
        fPicture = SkNEW(SkPicture);
    }
    SkCanvas* canvas = fPicture->beginRecording(this->width(), this->height());
    canvas->ref();  // our caller will call unref()
    return canvas;
}

SkSurface* SkSurface_Picture::onNewSurface(const SkImage::Info& info, SkColorSpace*) {
    return SkSurface::NewPicture(info.fWidth, info.fHeight);
}

SkImage* SkSurface_Picture::onNewImageShapshot() {
    if (fPicture) {
        return SkNewImageFromPicture(fPicture);
    } else {
        SkImage::Info info;
        info.fWidth = info.fHeight = 0;
        info.fColorType = SkImage::kPMColor_ColorType;
        info.fAlphaType = SkImage::kOpaque_AlphaType;
        return SkImage::NewRasterCopy(info, NULL, NULL, 0);
    }
}

void SkSurface_Picture::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                               const SkPaint* paint) {
    if (!fPicture) {
        return;
    }
    SkImagePrivDrawPicture(canvas, fPicture, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkMallocPixelRef.h"

SkSurface* SkSurface::NewRasterDirect(const SkImage::Info& info,
                                      SkColorSpace* cs,
                                      void* pixels, size_t rowBytes) {
    if (!SkSurface_Raster::Valid(info, cs, rowBytes)) {
        return NULL;
    }
    if (NULL == pixels) {
        return NULL;
    }
    
    return SkNEW_ARGS(SkSurface_Raster, (info, cs, pixels, rowBytes));
}

SkSurface* SkSurface::NewRaster(const SkImage::Info& info, SkColorSpace* cs) {
    if (!SkSurface_Raster::Valid(info, cs)) {
        return NULL;
    }

    size_t kMaxTotalSize = (1 << 31) - 1;
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
    return SkNEW_ARGS(SkSurface_Raster, (info, cs, pr, rowBytes));
}

SkSurface* SkSurface::NewPicture(int width, int height) {
    if ((width | height) < 0) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Picture, (width, height));
}

