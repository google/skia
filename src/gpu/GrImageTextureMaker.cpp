/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColorSpaceXform.h"
#include "GrImageTextureMaker.h"
#include "SkGr.h"
#include "SkImage_GpuYUVA.h"
#include "SkImage_Lazy.h"
#include "effects/GrYUVtoRGBEffect.h"

GrImageTextureMaker::GrImageTextureMaker(GrRecordingContext* context, const SkImage* client,
                                         SkImage::CachingHint chint, bool useDecal)
        : INHERITED(context, client->width(), client->height(), client->isAlphaOnly(), useDecal)
        , fImage(static_cast<const SkImage_Lazy*>(client))
        , fCachingHint(chint) {
    SkASSERT(client->isLazyGenerated());
    GrMakeKeyFromImageID(&fOriginalKey, client->uniqueID(),
                         SkIRect::MakeWH(this->width(), this->height()));
}

sk_sp<GrTextureProxy> GrImageTextureMaker::refOriginalTextureProxy(bool willBeMipped,
                                                                   AllowedTexGenType onlyIfFast) {
    return fImage->lockTextureProxy(this->context(), fOriginalKey, fCachingHint,
                                    willBeMipped, onlyIfFast);
}

void GrImageTextureMaker::makeCopyKey(const CopyParams& stretch, GrUniqueKey* paramsCopyKey) {
    if (fOriginalKey.isValid() && SkImage::kAllow_CachingHint == fCachingHint) {
        GrUniqueKey cacheKey;
        fImage->makeCacheKeyFromOrigKey(fOriginalKey, &cacheKey);
        MakeCopyKeyFromOrigKey(cacheKey, stretch, paramsCopyKey);
    }
}

SkAlphaType GrImageTextureMaker::alphaType() const {
    return fImage->alphaType();
}
SkColorSpace* GrImageTextureMaker::colorSpace() const {
    return fImage->colorSpace();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

GrYUVAImageTextureMaker::GrYUVAImageTextureMaker(GrContext* context, const SkImage* client,
                                                 bool useDecal)
    : INHERITED(context, client->width(), client->height(), client->isAlphaOnly(), useDecal)
    , fImage(static_cast<const SkImage_GpuYUVA*>(client)) {
    SkASSERT(as_IB(client)->isYUVA());
    GrMakeKeyFromImageID(&fOriginalKey, client->uniqueID(),
                         SkIRect::MakeWH(this->width(), this->height()));
}

sk_sp<GrTextureProxy> GrYUVAImageTextureMaker::refOriginalTextureProxy(bool willBeMipped,
                                                                   AllowedTexGenType onlyIfFast) {
    if (AllowedTexGenType::kCheap == onlyIfFast) {
        return nullptr;
    }

    if (willBeMipped) {
        return fImage->asMippedTextureProxyRef(this->context());
    } else {
        return fImage->asTextureProxyRef(this->context());
    }
}

void GrYUVAImageTextureMaker::makeCopyKey(const CopyParams& stretch, GrUniqueKey* paramsCopyKey) {
    // TODO: Do we ever want to disable caching?
    if (fOriginalKey.isValid()) {
        GrUniqueKey cacheKey;
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&cacheKey, fOriginalKey, kDomain, 0, "Image");
        MakeCopyKeyFromOrigKey(cacheKey, stretch, paramsCopyKey);
    }
}

SkAlphaType GrYUVAImageTextureMaker::alphaType() const {
    return fImage->alphaType();
}
SkColorSpace* GrYUVAImageTextureMaker::colorSpace() const {
    return fImage->colorSpace();
}

std::unique_ptr<GrFragmentProcessor> GrYUVAImageTextureMaker::createFragmentProcessor(
    const SkMatrix& textureMatrix,
    const SkRect& constraintRect,
    FilterConstraint filterConstraint,
    bool coordsLimitedToConstraintRect,
    const GrSamplerState::Filter* filterOrNullForBicubic) {

    // Check simple cases to see if we need to fall back to flattening the image
    // TODO: See if we can relax this -- for example, if filterConstraint
    //       is kYes_FilterConstraint we still may not need a TextureDomain
    //       in some cases. Or allow YUVtoRGBEffect to take a wrap mode to
    //       handle ClampToBorder when a decal is needed.
    if (!textureMatrix.isIdentity() || kNo_FilterConstraint != filterConstraint ||
        !coordsLimitedToConstraintRect || !filterOrNullForBicubic ||
        this->domainNeedsDecal()) {
        return this->INHERITED::createFragmentProcessor(textureMatrix, constraintRect,
                                                        filterConstraint,
                                                        coordsLimitedToConstraintRect,
                                                        filterOrNullForBicubic);
    }

    // Check to see if the client has given us pre-mipped textures or we can generate them
    // If not, fall back to bilerp
    GrSamplerState::Filter filter = *filterOrNullForBicubic;
    if (GrSamplerState::Filter::kMipMap == filter &&
        !fImage->setupMipmapsForPlanes(this->context())) {
        filter = GrSamplerState::Filter::kBilerp;
    }

    auto fp = GrYUVtoRGBEffect::Make(fImage->fProxies, fImage->fYUVAIndices,
                                     fImage->fYUVColorSpace, filter);
    if (fImage->fFromColorSpace) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp), fImage->fFromColorSpace.get(),
                                           fImage->alphaType(), fImage->colorSpace());
    }
    return fp;
}
