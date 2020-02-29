/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrImageTextureMaker.h"

#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_GpuYUVA.h"
#include "src/image/SkImage_Lazy.h"

static GrImageInfo get_image_info(GrRecordingContext* context, const SkImage* client) {
    SkASSERT(client->isLazyGenerated());
    const SkImage_Lazy* lazyImage = static_cast<const SkImage_Lazy*>(client);

    GrColorType ct = lazyImage->colorTypeOfLockTextureProxy(context->priv().caps());

    return {ct, client->alphaType(), client->refColorSpace(), client->dimensions()};
}

GrImageTextureMaker::GrImageTextureMaker(GrRecordingContext* context, const SkImage* client,
                                         SkImage::CachingHint chint)
        : INHERITED(context, get_image_info(context, client))
        , fImage(static_cast<const SkImage_Lazy*>(client))
        , fCachingHint(chint) {
    SkASSERT(client->isLazyGenerated());
}

GrSurfaceProxyView GrImageTextureMaker::refOriginalTextureProxyView(GrMipMapped mipMapped) {
    return fImage->lockTextureProxyView(this->context(), fCachingHint, mipMapped);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

GrYUVAImageTextureMaker::GrYUVAImageTextureMaker(GrContext* context, const SkImage* client)
        : INHERITED(context, client->imageInfo())
        , fImage(static_cast<const SkImage_GpuYUVA*>(client)) {
    SkASSERT(as_IB(client)->isYUVA());
}

GrSurfaceProxyView GrYUVAImageTextureMaker::refOriginalTextureProxyView(GrMipMapped mipMapped) {
    if (mipMapped == GrMipMapped::kYes) {
        return fImage->refMippedView(this->context());
    } else {
        if (const GrSurfaceProxyView* view = fImage->view(this->context())) {
            return *view;
        } else {
            return {};
        }
    }
}

std::unique_ptr<GrFragmentProcessor> GrYUVAImageTextureMaker::createFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        GrSamplerState::WrapMode wrapX,
        GrSamplerState::WrapMode wrapY,
        const GrSamplerState::Filter* filterOrNullForBicubic) {
    // Check simple cases to see if we need to fall back to flattening the image (or whether it's
    // already been flattened.) GrYUVtoRGBEffect only supports kClamp (for now.)
    if (!filterOrNullForBicubic || wrapX != GrSamplerState::WrapMode::kClamp ||
        wrapY != GrSamplerState::WrapMode::kClamp || fImage->fRGBView.proxy()) {
        return this->INHERITED::createFragmentProcessor(
                textureMatrix, constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                wrapX, wrapY, filterOrNullForBicubic);
    }

    // Check to see if the client has given us pre-mipped textures or we can generate them
    // If not, fall back to bilerp. Also fall back to bilerp when a domain is requested
    GrSamplerState::Filter filter = *filterOrNullForBicubic;
    if (GrSamplerState::Filter::kMipMap == filter &&
        (filterConstraint == GrTextureProducer::kYes_FilterConstraint ||
         !fImage->setupMipmapsForPlanes(this->context()))) {
        filter = GrSamplerState::Filter::kBilerp;
    }

    // Cannot rely on GrTextureProducer's domain infrastructure since we need to calculate domain's
    // per plane, which may be different, so respect the filterConstraint without any additional
    // analysis.
    const SkRect* domain = nullptr;
    if (filterConstraint == GrTextureProducer::kYes_FilterConstraint) {
        domain = &constraintRect;
    }

    const auto& caps = *fImage->context()->priv().caps();
    auto fp = GrYUVtoRGBEffect::Make(fImage->fViews, fImage->fYUVAIndices, fImage->fYUVColorSpace,
                                     filter, caps, textureMatrix, domain);
    if (fImage->fFromColorSpace) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp), fImage->fFromColorSpace.get(),
                                           fImage->alphaType(), fImage->colorSpace());
    }
    return fp;
}
