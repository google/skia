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
#if SK_SUPPORT_GPU
    GrTexture* onGenerateTexture(GrContext*, SkImageUsageType, const SkIRect*) override;
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

    return SkNEW_ARGS(SkPictureImageGenerator, (size, picture, matrix, paint));
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

SkImageGenerator* SkImageGenerator::NewFromPicture(const SkISize& size, const SkPicture* picture,
                                                   const SkMatrix* matrix, const SkPaint* paint) {
    return SkPictureImageGenerator::Create(size, picture, matrix, paint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
#include "GrTexture.h"

GrTexture* SkPictureImageGenerator::onGenerateTexture(GrContext* ctx, SkImageUsageType usage,
                                                      const SkIRect* subset) {
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
