/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "SkCanvas.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"

///////////////////////////////////////////////////////////////////////////////
class SkSpecialImage_Base : public SkSpecialImage {
public:
    SkSpecialImage_Base(const SkIRect& subset) : INHERITED(subset) { }
    virtual ~SkSpecialImage_Base() { }

    virtual void onDraw(SkCanvas*, int x, int y, const SkPaint*) const = 0;

    virtual bool onPeekPixels(SkPixmap*) const { return false; }

    virtual GrTexture* onPeekTexture() const { return nullptr; }

    virtual SkSpecialSurface* onNewSurface(const SkImageInfo& info) const { return nullptr; }

private:
    typedef SkSpecialImage INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
static inline const SkSpecialImage_Base* as_IB(const SkSpecialImage* image) {
    return static_cast<const SkSpecialImage_Base*>(image);
}

void SkSpecialImage::draw(SkCanvas* canvas, int x, int y, const SkPaint* paint) const {
    return as_IB(this)->onDraw(canvas, x, y, paint);
}

bool SkSpecialImage::peekPixels(SkPixmap* pixmap) const {
    return as_IB(this)->onPeekPixels(pixmap);
}

GrTexture* SkSpecialImage::peekTexture() const {
    return as_IB(this)->onPeekTexture();
}

SkSpecialSurface* SkSpecialImage::newSurface(const SkImageInfo& info) const {
    return as_IB(this)->onNewSurface(info);
}

///////////////////////////////////////////////////////////////////////////////
#include "SkImage.h"
#if SK_SUPPORT_GPU
#include "SkGr.h"
#include "SkGrPriv.h"
#endif

class SkSpecialImage_Image : public SkSpecialImage_Base {
public:
    SkSpecialImage_Image(const SkIRect& subset, const SkImage* image)
        : INHERITED(subset)
        , fImage(SkRef(image)) {
    }

    ~SkSpecialImage_Image() override { }

    void onDraw(SkCanvas* canvas, int x, int y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y, this->subset().width(), this->subset().height());

        canvas->drawImageRect(fImage, this->subset(),
                              dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    bool onPeekPixels(SkPixmap* pixmap) const override {
        return fImage->peekPixels(pixmap);
    }

    GrTexture* onPeekTexture() const override { return fImage->getTexture(); }

    SkSpecialSurface* onNewSurface(const SkImageInfo& info) const override {
#if SK_SUPPORT_GPU
        GrTexture* texture = fImage->getTexture();
        if (texture) {
            GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(info);
            desc.fFlags = kRenderTarget_GrSurfaceFlag;

            return SkSpecialSurface::NewRenderTarget(texture->getContext(), desc);
        }
#endif
        return SkSpecialSurface::NewRaster(info, nullptr);
    }

private:
    SkAutoTUnref<const SkImage> fImage;

    typedef SkSpecialImage_Base INHERITED;
};

#ifdef SK_DEBUG
static bool rect_fits(const SkIRect& rect, int width, int height) {
    return rect.fLeft >= 0 && rect.fLeft < width && rect.fLeft < rect.fRight &&
           rect.fRight >= 0 && rect.fRight <= width &&
           rect.fTop >= 0 && rect.fTop < height && rect.fTop < rect.fBottom &&
           rect.fBottom >= 0 && rect.fBottom <= height;
}
#endif

SkSpecialImage* SkSpecialImage::NewFromImage(const SkIRect& subset, const SkImage* image) {
    SkASSERT(rect_fits(subset, image->width(), image->height()));
    return new SkSpecialImage_Image(subset, image);
}

///////////////////////////////////////////////////////////////////////////////
#include "SkBitmap.h"
#include "SkImageInfo.h"
#include "SkPixelRef.h"

class SkSpecialImage_Raster : public SkSpecialImage_Base {
public:
    SkSpecialImage_Raster(const SkIRect& subset, const SkBitmap& bm)
        : INHERITED(subset)
        , fBitmap(bm) {
        if (bm.pixelRef()->isPreLocked()) {
            // we only preemptively lock if there is no chance of triggering something expensive
            // like a lazy decode or imagegenerator. PreLocked means it is flat pixels already.
            fBitmap.lockPixels();
        }
    }

    ~SkSpecialImage_Raster() override { }

    void onDraw(SkCanvas* canvas, int x, int y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        canvas->drawBitmapRect(fBitmap, this->subset(),
                               dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    bool onPeekPixels(SkPixmap* pixmap) const override {
        const SkImageInfo info = fBitmap.info();
        if ((kUnknown_SkColorType == info.colorType()) || !fBitmap.getPixels()) {
            return false;
        }
        const void* pixels = fBitmap.getPixels();
        if (pixels) {
            if (pixmap) {
                pixmap->reset(info, pixels, fBitmap.rowBytes());
            }
            return true;
        }
        return false;
    }

    SkSpecialSurface* onNewSurface(const SkImageInfo& info) const override {
        return SkSpecialSurface::NewRaster(info, nullptr);
    }

private:
    SkBitmap fBitmap;

    typedef SkSpecialImage_Base INHERITED;
};

SkSpecialImage* SkSpecialImage::NewFromRaster(const SkIRect& subset, const SkBitmap& bm) {
    SkASSERT(nullptr == bm.getTexture());
    SkASSERT(rect_fits(subset, bm.width(), bm.height()));
    return new SkSpecialImage_Raster(subset, bm);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
#include "GrTexture.h"

class SkSpecialImage_Gpu : public SkSpecialImage_Base {
public:
    SkSpecialImage_Gpu(const SkIRect& subset, GrTexture* tex)
        : INHERITED(subset)
        , fTexture(SkRef(tex)) {
    }

    ~SkSpecialImage_Gpu() override { }

    void onDraw(SkCanvas* canvas, int x, int y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        SkBitmap bm;

        static const bool kUnknownOpacity = false;
        GrWrapTextureInBitmap(fTexture,
                              fTexture->width(), fTexture->height(), kUnknownOpacity, &bm);

        canvas->drawBitmapRect(bm, this->subset(),
                               dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    GrTexture* onPeekTexture() const override { return fTexture; }

    SkSpecialSurface* onNewSurface(const SkImageInfo& info) const override {
        GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(info);
        desc.fFlags = kRenderTarget_GrSurfaceFlag;

        return SkSpecialSurface::NewRenderTarget(fTexture->getContext(), desc);
    }

private:
    SkAutoTUnref<GrTexture> fTexture;

    typedef SkSpecialImage_Base INHERITED;
};

SkSpecialImage* SkSpecialImage::NewFromGpu(const SkIRect& subset, GrTexture* tex) {
    SkASSERT(rect_fits(subset, tex->width(), tex->height()));
    return new SkSpecialImage_Gpu(subset, tex);
}

#else

SkSpecialImage* SkSpecialImage::NewFromGpu(const SkIRect& subset, GrTexture* tex) {
    return nullptr;
}

#endif
