/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImageGenerator.h"
#include "SkCanvas.h"
#include "SkMakeUnique.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkSurface.h"
#include "SkTLazy.h"

class SkPictureImageGenerator : public SkImageGenerator {
public:
    SkPictureImageGenerator(const SkImageInfo& info, sk_sp<SkPicture>, const SkMatrix*,
                            const SkPaint*);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options& opts)
        override;

#if SK_SUPPORT_GPU
    TexGenType onCanGenerateTexture() const override { return TexGenType::kExpensive; }
    sk_sp<GrTextureProxy> onGenerateTexture(GrRecordingContext*, const SkImageInfo&,
                                            const SkIPoint&, bool willNeedMipMaps) override;
#endif

private:
    sk_sp<SkPicture>    fPicture;
    SkMatrix            fMatrix;
    SkTLazy<SkPaint>    fPaint;

    typedef SkImageGenerator INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkImageGenerator>
SkImageGenerator::MakeFromPicture(const SkISize& size, sk_sp<SkPicture> picture,
                                  const SkMatrix* matrix, const SkPaint* paint,
                                  SkImage::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace) {
    if (!picture || !colorSpace || size.isEmpty()) {
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

///////////////////////////////////////////////////////////////////////////////////////////////////

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
                                          const Options& opts) {
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(info, pixels, rowBytes, &props);
    if (!canvas) {
        return false;
    }
    canvas->clear(0);
    canvas->drawPicture(fPicture, &fMatrix, fPaint.getMaybeNull());
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"

sk_sp<GrTextureProxy> SkPictureImageGenerator::onGenerateTexture(
        GrRecordingContext* ctx, const SkImageInfo& info,
        const SkIPoint& origin, bool willNeedMipMaps) {
    SkASSERT(ctx);

    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);

    // CONTEXT TODO: remove this use of 'backdoor' to create an SkSkSurface
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx->priv().backdoor(),
                                                         SkBudgeted::kYes, info, 0,
                                                         kTopLeft_GrSurfaceOrigin, &props,
                                                         willNeedMipMaps));
    if (!surface) {
        return nullptr;
    }

    SkMatrix matrix = fMatrix;
    matrix.postTranslate(-origin.x(), -origin.y());
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawPicture(fPicture.get(), &matrix, fPaint.getMaybeNull());
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    if (!image) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy = as_IB(image)->asTextureProxyRef(ctx);
    SkASSERT(!willNeedMipMaps || GrMipMapped::kYes == proxy->mipMapped());
    return proxy;
}
#endif
