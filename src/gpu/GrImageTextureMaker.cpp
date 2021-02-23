/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrImageTextureMaker.h"

#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_GpuYUVA.h"
#include "src/image/SkImage_Lazy.h"

static GrImageInfo get_image_info(GrRecordingContext* context, const SkImage* client) {
    SkASSERT(client->isLazyGenerated());
    const SkImage_Lazy* lazyImage = static_cast<const SkImage_Lazy*>(client);

    GrColorType ct = lazyImage->colorTypeOfLockTextureProxy(context->priv().caps());

    return {ct, client->alphaType(), client->refColorSpace(), client->dimensions()};
}

GrImageTextureMaker::GrImageTextureMaker(GrRecordingContext* context,
                                         const SkImage* client,
                                         GrImageTexGenPolicy texGenPolicy)
        : INHERITED(context, get_image_info(context, client))
        , fImage(static_cast<const SkImage_Lazy*>(client))
        , fTexGenPolicy(texGenPolicy) {
    SkASSERT(client->isLazyGenerated());
}

GrSurfaceProxyView GrImageTextureMaker::refOriginalTextureProxyView(GrMipmapped mipMapped) {
    return fImage->lockTextureProxyView(this->context(), fTexGenPolicy, mipMapped);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

GrYUVAImageTextureMaker::GrYUVAImageTextureMaker(GrRecordingContext* context, const SkImage* client)
        : INHERITED(context, client->imageInfo())
        , fImage(static_cast<const SkImage_GpuYUVA*>(client)) {
    SkASSERT(as_IB(client)->isYUVA());
}

GrSurfaceProxyView GrYUVAImageTextureMaker::refOriginalTextureProxyView(GrMipmapped mipmapped) {
    auto [view, ct] = fImage->asView(this->context(), mipmapped);
    SkASSERT(ct == this->colorType());
    return view;
}

std::unique_ptr<GrFragmentProcessor> GrYUVAImageTextureMaker::createFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect* subset,
        const SkRect* domain,
        GrSamplerState samplerState) {
    // Check whether it's already been flattened.
    if (fImage->fRGBView.proxy()) {
        return this->INHERITED::createFragmentProcessor(textureMatrix, subset, domain,
                                                        samplerState);
    }

    // Check to see if the client has given us pre-mipped textures or if we can generate them
    // If not disable mip mapping.
    if (samplerState.mipmapped() == GrMipmapped::kYes &&
        !fImage->setupMipmapsForPlanes(this->context())) {
        samplerState.setMipmapMode(GrSamplerState::MipmapMode::kNone);
    }

    const auto& caps = *fImage->context()->priv().caps();
    auto fp = GrYUVtoRGBEffect::Make(fImage->fYUVAProxies,
                                     samplerState,
                                     caps,
                                     textureMatrix,
                                     subset,
                                     domain);
    if (fImage->fFromColorSpace) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           fImage->fFromColorSpace.get(), fImage->alphaType(),
                                           fImage->colorSpace(), kPremul_SkAlphaType);
    }
    return fp;
}

std::unique_ptr<GrFragmentProcessor> GrYUVAImageTextureMaker::createBicubicFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect* subset,
        const SkRect* domain,
        GrSamplerState::WrapMode wrapX,
        GrSamplerState::WrapMode wrapY,
        SkImage::CubicResampler kernel) {
    const auto& caps = *fImage->context()->priv().caps();
    GrSamplerState samplerState(wrapX, wrapY, GrSamplerState::Filter::kNearest);
    auto fp = GrYUVtoRGBEffect::Make(fImage->fYUVAProxies, samplerState, caps, SkMatrix::I(),
                                     subset, domain);
    fp = GrBicubicEffect::Make(std::move(fp),
                               fImage->alphaType(),
                               textureMatrix,
                               kernel,
                               GrBicubicEffect::Direction::kXY);
    if (fImage->fFromColorSpace) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           fImage->fFromColorSpace.get(), fImage->alphaType(),
                                           fImage->colorSpace(), kPremul_SkAlphaType);
    }
    return fp;
}
