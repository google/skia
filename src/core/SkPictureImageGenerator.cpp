/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkCanvas.h"
#include "SkMakeUnique.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureImageGenerator.h"
#include "SkSurface.h"

std::unique_ptr<SkImageGenerator>
SkPictureImageGenerator::Make(const SkISize& size, sk_sp<SkPicture> picture, const SkMatrix* matrix,
                              const SkPaint* paint, SkImage::BitDepth bitDepth,
                              sk_sp<SkColorSpace> colorSpace) {
    if (!picture || size.isEmpty()) {
        return nullptr;
    }

    if (SkImage::BitDepth::kF16 == bitDepth && (!colorSpace || !colorSpace->gammaIsLinear())) {
        return nullptr;
    }

    if (colorSpace && (!colorSpace->gammaCloseToSRGB() && !colorSpace->gammaIsLinear())) {
        return nullptr;
    }

    SkColorType colorType = kN32_SkColorType;
    if (SkImage::BitDepth::kF16 == bitDepth) {
        colorType = kRGBA_F16_SkColorType;
    }

    SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), colorType,
                                         kPremul_SkAlphaType, std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(
                             new SkPictureImageGenerator(info, std::move(picture), matrix, paint));
}

SkPictureImageGenerator::SkPictureImageGenerator(const SkImageInfo& info, sk_sp<SkPicture> picture,
                                                 const SkMatrix* matrix, const SkPaint* paint)
    : INHERITED(info)
    , fPicture(std::move(picture)) {

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

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkImageGenerator>
SkImageGenerator::MakeFromPicture(const SkISize& size, sk_sp<SkPicture> picture,
                                  const SkMatrix* matrix, const SkPaint* paint,
                                  SkImage::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace) {
    // Check this here (rather than in SkPictureImageGenerator::Create) so SkPictureShader
    // has a private entry point to create legacy picture backed images.
    if (!colorSpace) {
        return nullptr;
    }

    return SkPictureImageGenerator::Make(size, std::move(picture), matrix, paint, bitDepth,
                                         std::move(colorSpace));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkPictureImageGenerator::onGenerateTexture(GrContext* ctx,
                                                                 const SkImageInfo& info,
                                                                 const SkIPoint& origin) {
    SkASSERT(ctx);

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
    return as_IB(image)->asTextureProxyRef();
}
#endif
