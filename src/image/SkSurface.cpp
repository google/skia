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

class SkSurface_Raster : public SkSurface {
public:
    static bool Valid(const SkImage::Info&, SkColorSpace*, size_t rb);

    SkSurface_Raster(const SkImage::Info&, SkColorSpace*, void*, size_t rb);

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImage::Info&, SkColorSpace*) SK_OVERRIDE;
    virtual SkImage* onNewImageShapshot() SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;

private:
    SkBitmap    fBitmap;

    typedef SkSurface INHERITED;
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
}

SkCanvas* SkSurface_Raster::onNewCanvas() {
    return SkNEW_ARGS(SkCanvas, (fBitmap));
}

SkSurface* SkSurface_Raster::onNewSurface(const SkImage::Info& info,
                                          SkColorSpace* cs) {
    return SkSurface::NewRaster(info, cs);
}

SkImage* SkSurface_Raster::onNewImageShapshot() {
    return SkNewImageFromBitmap(fBitmap);
}

void SkSurface_Raster::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                              const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

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

