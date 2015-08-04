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
#include "SkTLazy.h"

class SkPictureImageGenerator : SkImageGenerator {
public:
    static SkImageGenerator* Create(const SkISize&, const SkPicture*, const SkMatrix*,
                                    const SkPaint*);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
                     int* ctableCount) override;

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
