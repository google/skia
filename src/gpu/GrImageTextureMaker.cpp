/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageTextureMaker.h"
#include "SkGr.h"
#include "SkImage_GpuYUVA.h"
#include "SkImage_Lazy.h"
#include "effects/GrYUVtoRGBEffect.h"

GrImageTextureMaker::GrImageTextureMaker(GrContext* context, const SkImage* client,
                                         SkImage::CachingHint chint)
        : INHERITED(context, client->width(), client->height(), client->isAlphaOnly())
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

GrYUVAImageTextureMaker::GrYUVAImageTextureMaker(GrContext* context, const SkImage* client )
    : INHERITED(context, client->width(), client->height(), client->isAlphaOnly())
    , fImage(static_cast<const SkImage_GpuYUVA*>(client)) {
    SkASSERT(as_IB(client)->isYUVA());
    GrMakeKeyFromImageID(&fOriginalKey, client->uniqueID(),
                         SkIRect::MakeWH(this->width(), this->height()));
}

sk_sp<GrTextureProxy> GrYUVAImageTextureMaker::refOriginalTextureProxy(bool willBeMipped,
                                                                   AllowedTexGenType onlyIfFast) {
    return (AllowedTexGenType::kCheap == onlyIfFast) ? nullptr : fImage->asTextureProxyRef();
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
    if (!textureMatrix.isIdentity() || kNo_FilterConstraint != filterConstraint ||
        !coordsLimitedToConstraintRect || !filterOrNullForBicubic) {
        return this->INHERITED::createFragmentProcessor(textureMatrix, constraintRect,
                                                        filterConstraint,
                                                        coordsLimitedToConstraintRect,
                                                        filterOrNullForBicubic);
    }

    // Check to see if the client has given us pre-mipped textures
    // If not, fall back to bilerp
    // TODO: investigate flattening the image and generating miplevels
    GrSamplerState::Filter filter = *filterOrNullForBicubic;
    if (GrSamplerState::Filter::kMipMap == filter && !fImage->canBeMipmapped(fContext)) {
        filter = GrSamplerState::Filter::kBilerp;
    }

    return GrYUVtoRGBEffect::Make(fImage->fProxies, fImage->fYUVAIndices,
                                  fImage->fYUVColorSpace, filter);

}

