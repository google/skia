/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
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
                                    const SkPaint*, sk_sp<SkColorSpace>);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
                     int* ctableCount) override;
    bool onComputeScaledDimensions(SkScalar scale, SupportedSizes*) override;
    bool onGenerateScaledPixels(const SkPixmap&) override;

#if SK_SUPPORT_GPU
    GrTexture* onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&) override;
#endif

private:
    SkPictureImageGenerator(const SkImageInfo& info, const SkPicture*, const SkMatrix*,
                            const SkPaint*);

    sk_sp<const SkPicture> fPicture;
    SkMatrix               fMatrix;
    SkTLazy<SkPaint>       fPaint;

    typedef SkImageGenerator INHERITED;
};

SkImageGenerator* SkPictureImageGenerator::Create(const SkISize& size, const SkPicture* picture,
                                                  const SkMatrix* matrix, const SkPaint* paint,
                                                  sk_sp<SkColorSpace> colorSpace) {
    if (!picture || size.isEmpty()) {
        return nullptr;
    }

    SkColorType colorType;
    if (!colorSpace || colorSpace->gammaCloseToSRGB()) {
        colorType = kN32_SkColorType;
    } else if (colorSpace->gammaIsLinear()) {
        colorType = kRGBA_F16_SkColorType;
    } else {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), colorType,
                                         kPremul_SkAlphaType, std::move(colorSpace));
    return new SkPictureImageGenerator(info, picture, matrix, paint);
}

SkPictureImageGenerator::SkPictureImageGenerator(const SkImageInfo& info, const SkPicture* picture,
                                                 const SkMatrix* matrix, const SkPaint* paint)
    : INHERITED(info)
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
    if (ctable || ctableCount) {
        return false;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(info, pixels, rowBytes)) {
        return false;
    }

    bitmap.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
    canvas.drawPicture(fPicture.get(), &fMatrix, fPaint.getMaybeNull());

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

bool SkPictureImageGenerator::onGenerateScaledPixels(const SkPixmap& scaledPixels) {
    int w = scaledPixels.width();
    int h = scaledPixels.height();

    const SkScalar scaleX = SkIntToScalar(w) / this->getInfo().width();
    const SkScalar scaleY = SkIntToScalar(h) / this->getInfo().height();
    SkMatrix matrix = SkMatrix::MakeScale(scaleX, scaleY);

    SkBitmap bitmap;
    if (!bitmap.installPixels(scaledPixels)) {
        return false;
    }

    bitmap.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
    matrix.preConcat(fMatrix);
    canvas.drawPicture(fPicture.get(), &matrix, fPaint.getMaybeNull());
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkImageGenerator* SkImageGenerator::NewFromPicture(const SkISize& size, const SkPicture* picture,
                                                   const SkMatrix* matrix, const SkPaint* paint,
                                                   sk_sp<SkColorSpace> colorSpace) {
    return SkPictureImageGenerator::Create(size, picture, matrix, paint, std::move(colorSpace));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
#include "GrTexture.h"

GrTexture* SkPictureImageGenerator::onGenerateTexture(GrContext* ctx, const SkImageInfo& info,
                                                      const SkIPoint& origin) {
    //
    // TODO: respect the usage, by possibly creating a different (pow2) surface
    //
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kYes, info));
    if (!surface) {
        return nullptr;
    }

    SkMatrix matrix = fMatrix;
    matrix.postTranslate(-origin.x(), -origin.y());
    surface->getCanvas()->clear(0); // does NewRenderTarget promise to do this for us?
    surface->getCanvas()->drawPicture(fPicture.get(), &matrix, fPaint.getMaybeNull());
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    if (!image) {
        return nullptr;
    }
    return SkSafeRef(as_IB(image)->peekTexture());
}
#endif
