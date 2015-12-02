/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageGenerator.h"
#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkSurface.h"
#include "SkTLazy.h"

class SkPictureImageGenerator : SkImageGenerator {
public:
    static SkImageGenerator* Create(const SkISize&, const SkPicture*, const SkMatrix*,
                                    const SkPaint*);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
                     int* ctableCount) override;
    bool onComputeScaledDimensions(SkScalar scale, SupportedSizes*) override;
    bool onGenerateScaledPixels(const SkISize&, const SkIPoint&, const SkPixmap&) override;

#if SK_SUPPORT_GPU
    GrTexture* onGenerateTexture(GrContext*, const SkIRect*) override;
#endif

private:
    SkPictureImageGenerator(const SkISize&, const SkPicture*, const SkMatrix*, const SkPaint*);

    SkAutoTUnref<const SkPicture> fPicture;
    SkMatrix                      fMatrix;
    SkTLazy<SkPaint>              fPaint;

    typedef SkImageGenerator INHERITED;
};

SkImageGenerator* SkPictureImageGenerator::Create(const SkISize& size, const SkPicture* picture,
                                const SkMatrix* matrix, const SkPaint* paint) {
    if (!picture || size.isEmpty()) {
        return nullptr;
    }

    return new SkPictureImageGenerator(size, picture, matrix, paint);
}

SkPictureImageGenerator::SkPictureImageGenerator(const SkISize& size, const SkPicture* picture,
                                                 const SkMatrix* matrix, const SkPaint* paint)
    : INHERITED(SkImageInfo::MakeN32Premul(size))
    , fPicture(SkRef(picture)) {

    if (matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.reset();
    }

    if (paint) {
        fPaint.set(*paint);
    }
}

bool SkPictureImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                          SkPMColor ctable[], int* ctableCount) {
    if (info != getInfo() || ctable || ctableCount) {
        return false;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(info, pixels, rowBytes)) {
        return false;
    }

    bitmap.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
    canvas.drawPicture(fPicture, &fMatrix, fPaint.getMaybeNull());

    return true;
}

bool SkPictureImageGenerator::onComputeScaledDimensions(SkScalar scale,
                                                        SupportedSizes* sizes) {
    SkASSERT(scale > 0 && scale <= 1);
    const int w = this->getInfo().width();
    const int h = this->getInfo().height();
    const int sw = SkScalarRoundToInt(scale * w);
    const int sh = SkScalarRoundToInt(scale * h);
    if (sw > 0 && sh > 0) {
        sizes->fSizes[0].set(sw, sh);
        sizes->fSizes[1].set(sw, sh);
        return true;
    }
    return false;
}

bool SkPictureImageGenerator::onGenerateScaledPixels(const SkISize& scaledSize,
                                                     const SkIPoint& scaledOrigin,
                                                     const SkPixmap& scaledPixels) {
    int w = scaledSize.width();
    int h = scaledSize.height();

    const SkScalar scaleX = SkIntToScalar(w) / this->getInfo().width();
    const SkScalar scaleY = SkIntToScalar(h) / this->getInfo().height();
    SkMatrix matrix = SkMatrix::MakeScale(scaleX, scaleY);
    matrix.postTranslate(-SkIntToScalar(scaledOrigin.x()), -SkIntToScalar(scaledOrigin.y()));

    SkBitmap bitmap;
    if (!bitmap.installPixels(scaledPixels.info(), scaledPixels.writable_addr(),
                              scaledPixels.rowBytes())) {
        return false;
    }

    bitmap.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
    matrix.preConcat(fMatrix);
    canvas.drawPicture(fPicture, &matrix, fPaint.getMaybeNull());
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkImageGenerator* SkImageGenerator::NewFromPicture(const SkISize& size, const SkPicture* picture,
                                                   const SkMatrix* matrix, const SkPaint* paint) {
    return SkPictureImageGenerator::Create(size, picture, matrix, paint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
#include "GrTexture.h"

GrTexture* SkPictureImageGenerator::onGenerateTexture(GrContext* ctx, const SkIRect* subset) {
    const SkImageInfo& info = this->getInfo();
    SkImageInfo surfaceInfo = subset ? info.makeWH(subset->width(), subset->height()) : info;

    //
    // TODO: respect the usage, by possibly creating a different (pow2) surface
    //
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(ctx,
                                                               SkSurface::kYes_Budgeted,
                                                               surfaceInfo));
    if (!surface.get()) {
        return nullptr;
    }

    SkMatrix matrix = fMatrix;
    if (subset) {
        matrix.postTranslate(-subset->x(), -subset->y());
    }
    surface->getCanvas()->clear(0); // does NewRenderTarget promise to do this for us?
    surface->getCanvas()->drawPicture(fPicture, &matrix, fPaint.getMaybeNull());
    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    if (!image.get()) {
        return nullptr;
    }
    return SkSafeRef(image->getTexture());
}
#endif
