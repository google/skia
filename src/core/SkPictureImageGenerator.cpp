/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSurface.h"
#include "src/core/SkTLazy.h"
#include "src/image/SkImage_Base.h"

class SkPictureImageGenerator : public SkImageGenerator {
public:
    SkPictureImageGenerator(const SkImageInfo& info, sk_sp<SkPicture>, const SkMatrix*,
                            const SkPaint*, const SkSurfaceProps& props);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options& opts)
        override;

#if SK_SUPPORT_GPU
    GrSurfaceProxyView onGenerateTexture(GrRecordingContext*, const SkImageInfo&, const SkIPoint&,
                                         GrMipmapped, GrImageTexGenPolicy) override;
#endif

private:
    sk_sp<SkPicture>    fPicture;
    SkMatrix            fMatrix;
    SkTLazy<SkPaint>    fPaint;
    SkSurfaceProps      fProps;

    using INHERITED = SkImageGenerator;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkImageGenerator>
SkImageGenerator::MakeFromPicture(const SkISize& size, sk_sp<SkPicture> picture,
                                  const SkMatrix* matrix, const SkPaint* paint,
                                  SkImage::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace,
                                  SkSurfaceProps props) {
    if (!picture || !colorSpace || size.isEmpty()) {
        return nullptr;
    }

    SkColorType colorType = kN32_SkColorType;
    if (SkImage::BitDepth::kF16 == bitDepth) {
        colorType = kRGBA_F16_SkColorType;
    }

    SkImageInfo info =
            SkImageInfo::Make(size, colorType, kPremul_SkAlphaType, std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(
        new SkPictureImageGenerator(info, std::move(picture), matrix, paint, props));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkPictureImageGenerator::SkPictureImageGenerator(const SkImageInfo& info, sk_sp<SkPicture> picture,
                                                 const SkMatrix* matrix, const SkPaint* paint,
                                                 const SkSurfaceProps& props)
    : INHERITED(info)
    , fPicture(std::move(picture))
    , fProps(props) {

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
                                          const Options& opts) {
    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(info, pixels, rowBytes, &fProps);
    if (!canvas) {
        return false;
    }
    canvas->clear(0);
    canvas->drawPicture(fPicture, &fMatrix, fPaint.getMaybeNull());
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"

GrSurfaceProxyView SkPictureImageGenerator::onGenerateTexture(GrRecordingContext* ctx,
                                                              const SkImageInfo& info,
                                                              const SkIPoint& origin,
                                                              GrMipmapped mipmapped,
                                                              GrImageTexGenPolicy texGenPolicy) {
    SkASSERT(ctx);

    SkBudgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                  ? SkBudgeted::kNo
                                  : SkBudgeted::kYes;
    auto surface = SkSurface::MakeRenderTarget(ctx, budgeted, info, 0, kTopLeft_GrSurfaceOrigin,
                                               &fProps, mipmapped == GrMipmapped::kYes);
    if (!surface) {
        return {};
    }

    SkMatrix matrix = fMatrix;
    matrix.postTranslate(-origin.x(), -origin.y());
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawPicture(fPicture.get(), &matrix, fPaint.getMaybeNull());
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    if (!image) {
        return {};
    }
    auto [view, ct] = as_IB(image)->asView(ctx, mipmapped);
    SkASSERT(view);
    SkASSERT(mipmapped == GrMipmapped::kNo ||
             view.asTextureProxy()->mipmapped() == GrMipmapped::kYes);
    return view;
}
#endif
