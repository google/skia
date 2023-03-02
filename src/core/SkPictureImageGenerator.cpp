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
#include "include/core/SkSurfaceProps.h"
#include "src/base/SkTLazy.h"
#include "src/image/SkImage_Base.h"

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrTextureProxy.h"
#endif

class SkPictureImageGenerator : public SkImageGenerator {
public:
    SkPictureImageGenerator(const SkImageInfo&, sk_sp<SkPicture>, const SkMatrix*,
                            const SkPaint*, const SkSurfaceProps&);

protected:
    bool onGetPixels(const SkImageInfo&, void* pixels, size_t rowBytes, const Options&) override;

#if defined(SK_GANESH)
    GrSurfaceProxyView onGenerateTexture(GrRecordingContext*, const SkImageInfo&,
                                         GrMipmapped, GrImageTexGenPolicy) override;
#endif

#if defined(SK_GRAPHITE)
    sk_sp<SkImage> onMakeTextureImage(skgpu::graphite::Recorder*,
                                      const SkImageInfo&,
                                      skgpu::Mipmapped) override;
#endif

private:
    sk_sp<SkPicture> fPicture;
    SkMatrix         fMatrix;
    SkTLazy<SkPaint> fPaint;
    SkSurfaceProps   fProps;

    using INHERITED = SkImageGenerator;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkImageGenerator>
SkImageGenerator::MakeFromPicture(const SkISize& size, sk_sp<SkPicture> picture,
                                  const SkMatrix* matrix, const SkPaint* paint,
                                  SkImage::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace) {
    return SkImageGenerator::MakeFromPicture(size, picture, matrix, paint, bitDepth,
                                             colorSpace, {});
}

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
        : SkImageGenerator(info)
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

#if defined(SK_GANESH)
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"

GrSurfaceProxyView SkPictureImageGenerator::onGenerateTexture(GrRecordingContext* ctx,
                                                              const SkImageInfo& info,
                                                              GrMipmapped mipmapped,
                                                              GrImageTexGenPolicy texGenPolicy) {
    SkASSERT(ctx);

    skgpu::Budgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                       ? skgpu::Budgeted::kNo
                                       : skgpu::Budgeted::kYes;
    auto surface = SkSurface::MakeRenderTarget(ctx, budgeted, info, 0, kTopLeft_GrSurfaceOrigin,
                                               &fProps, mipmapped == GrMipmapped::kYes);
    if (!surface) {
        return {};
    }

    surface->getCanvas()->clear(SkColors::kTransparent);
    surface->getCanvas()->drawPicture(fPicture.get(), &fMatrix, fPaint.getMaybeNull());
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

#endif // defined(SK_GANESH)

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Log.h"

sk_sp<SkImage> SkPictureImageGenerator::onMakeTextureImage(skgpu::graphite::Recorder* recorder,
                                                           const SkImageInfo& info,
                                                           skgpu::Mipmapped mipmapped) {
    using namespace skgpu::graphite;

    sk_sp<SkSurface> surface = SkSurface::MakeGraphite(recorder, info, mipmapped);
    if (!surface) {
        SKGPU_LOG_E("Failed to create Surface");
        return nullptr;
    }

    surface->getCanvas()->clear(SkColors::kTransparent);
    surface->getCanvas()->drawPicture(fPicture.get(), &fMatrix, fPaint.getMaybeNull());
    return surface->asImage();
}

#endif // SK_GRAPHITE
