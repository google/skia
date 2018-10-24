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
    // not sure what this means here
    return nullptr;
}

void GrYUVAImageTextureMaker::makeCopyKey(const CopyParams& stretch, GrUniqueKey* paramsCopyKey) {
    //if (fOriginalKey.isValid() && SkImage::kAllow_CachingHint == fCachingHint) {
    //    GrUniqueKey cacheKey;
    //    fImage->makeCacheKeyFromOrigKey(fOriginalKey, &cacheKey);
    //    MakeCopyKeyFromOrigKey(cacheKey, stretch, paramsCopyKey);
    //}
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

    // need to do something with those input params

    return GrYUVtoRGBEffect::Make(fImage->fProxies, fImage->fYUVAIndices,
                                  fImage->fYUVColorSpace);

}

