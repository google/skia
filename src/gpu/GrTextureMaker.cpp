/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureMaker.h"

#include "GrContext.h"
#include "GrGpu.h"

GrTexture* GrTextureMaker::refTextureForParams(const GrSamplerParams& params,
                                               SkColorSpace* dstColorSpace,
                                               sk_sp<SkColorSpace>* texColorSpace) {
    CopyParams copyParams;
    bool willBeMipped = params.filterMode() == GrSamplerParams::kMipMap_FilterMode;

    if (!fContext->caps()->mipMapSupport()) {
        willBeMipped = false;
    }

    if (texColorSpace) {
        *texColorSpace = this->getColorSpace(dstColorSpace);
    }

    if (!fContext->getGpu()->makeCopyForTextureParams(this->width(), this->height(), params,
                                                      &copyParams)) {
        return this->refOriginalTexture(willBeMipped, dstColorSpace);
    }
    GrUniqueKey copyKey;
    this->makeCopyKey(copyParams, &copyKey, dstColorSpace);
    if (copyKey.isValid()) {
        GrTexture* result = fContext->textureProvider()->findAndRefTextureByUniqueKey(copyKey);
        if (result) {
            return result;
        }
    }

    GrTexture* result = this->generateTextureForParams(copyParams, willBeMipped, dstColorSpace);
    if (!result) {
        return nullptr;
    }

    if (copyKey.isValid()) {
        fContext->textureProvider()->assignUniqueKeyToTexture(copyKey, result);
        this->didCacheCopy(copyKey);
    }
    return result;
}

sk_sp<GrFragmentProcessor> GrTextureMaker::createFragmentProcessor(
                                        const SkMatrix& textureMatrix,
                                        const SkRect& constraintRect,
                                        FilterConstraint filterConstraint,
                                        bool coordsLimitedToConstraintRect,
                                        const GrSamplerParams::FilterMode* filterOrNullForBicubic,
                                        SkColorSpace* dstColorSpace) {

    const GrSamplerParams::FilterMode* fmForDetermineDomain = filterOrNullForBicubic;
    if (filterOrNullForBicubic && GrSamplerParams::kMipMap_FilterMode == *filterOrNullForBicubic &&
        kYes_FilterConstraint == filterConstraint) {
        // TODo: Here we should force a copy restricted to the constraintRect since MIP maps will
        // read outside the constraint rect. However, as in the adjuster case, we aren't currently
        // doing that.
        // We instead we compute the domain as though were bilerping which is only correct if we
        // only sample level 0.
        static const GrSamplerParams::FilterMode kBilerp = GrSamplerParams::kBilerp_FilterMode;
        fmForDetermineDomain = &kBilerp;
    }

    GrSamplerParams params;
    if (filterOrNullForBicubic) {
        params.reset(SkShader::kClamp_TileMode, *filterOrNullForBicubic);
    } else {
        // Bicubic doesn't use filtering for it's texture accesses.
        params.reset(SkShader::kClamp_TileMode, GrSamplerParams::kNone_FilterMode);
    }
    sk_sp<SkColorSpace> texColorSpace;
    sk_sp<GrTexture> texture(this->refTextureForParams(params, dstColorSpace, &texColorSpace));
    if (!texture) {
        return nullptr;
    }
    SkRect domain;
    DomainMode domainMode =
        DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            texture->width(), texture->height(),
                            nullptr, fmForDetermineDomain, &domain);
    SkASSERT(kTightCopy_DomainMode != domainMode);
    SkMatrix normalizedTextureMatrix = textureMatrix;
    normalizedTextureMatrix.postIDiv(texture->width(), texture->height());
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(texColorSpace.get(),
                                                                       dstColorSpace);
    return CreateFragmentProcessorForDomainAndFilter(texture.get(), std::move(colorSpaceXform),
                                                     normalizedTextureMatrix, domainMode, domain,
                                                     filterOrNullForBicubic);
}

GrTexture* GrTextureMaker::generateTextureForParams(const CopyParams& copyParams, bool willBeMipped,
                                                    SkColorSpace* dstColorSpace) {
    sk_sp<GrTexture> original(this->refOriginalTexture(willBeMipped, dstColorSpace));
    if (!original) {
        return nullptr;
    }
    return CopyOnGpu(original.get(), nullptr, copyParams);
}
