/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureMaker.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"

sk_sp<GrTextureProxy> GrTextureMaker::refTextureProxyForParams(const GrSamplerParams& params,
                                                               SkColorSpace* dstColorSpace,
                                                               sk_sp<SkColorSpace>* texColorSpace,
                                                               SkScalar scaleAdjust[2]) {
    CopyParams copyParams;
    bool willBeMipped = params.filterMode() == GrSamplerParams::kMipMap_FilterMode;

    if (!fContext->caps()->mipMapSupport()) {
        willBeMipped = false;
    }

    if (texColorSpace) {
        *texColorSpace = this->getColorSpace(dstColorSpace);
    }

    sk_sp<GrTextureProxy> original(this->refOriginalTextureProxy(willBeMipped, dstColorSpace,
                                                                 AllowedTexGenType::kCheap));
    if (original) {
        if (!fContext->getGpu()->isACopyNeededForTextureParams(original.get(), params, &copyParams,
                                                               scaleAdjust)) {
            return original;
        }
    } else {
        if (!fContext->getGpu()->isACopyNeededForTextureParams(this->width(), this->height(),
                                                               params, &copyParams, scaleAdjust)) {
            return this->refOriginalTextureProxy(willBeMipped, dstColorSpace,
                                                 AllowedTexGenType::kAny);
        }
    }

    GrUniqueKey copyKey;
    this->makeCopyKey(copyParams, &copyKey, dstColorSpace);
    if (copyKey.isValid()) {
        sk_sp<GrTextureProxy> result(fContext->resourceProvider()->findProxyByUniqueKey(copyKey));
        if (result) {
            return result;
        }
    }

    sk_sp<GrTextureProxy> result;
    if (original) {
        result = CopyOnGpu(fContext, std::move(original), nullptr, copyParams);
    } else {
        result = this->generateTextureProxyForParams(copyParams, willBeMipped, dstColorSpace);
    }

    if (!result) {
        return nullptr;
    }

    if (copyKey.isValid()) {
        fContext->resourceProvider()->assignUniqueKeyToProxy(copyKey, result.get());
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
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(this->refTextureProxyForParams(params, dstColorSpace,
                                                               &texColorSpace,
                                                               scaleAdjust));
    if (!proxy) {
        return nullptr;
    }
    SkMatrix adjustedMatrix = textureMatrix;
    adjustedMatrix.postScale(scaleAdjust[0], scaleAdjust[1]);
    SkRect domain;
    DomainMode domainMode =
        DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            proxy.get(),
                            nullptr, fmForDetermineDomain, &domain);
    SkASSERT(kTightCopy_DomainMode != domainMode);
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(texColorSpace.get(),
                                                                       dstColorSpace);
    return CreateFragmentProcessorForDomainAndFilter(std::move(proxy),
                                                     std::move(colorSpaceXform),
                                                     adjustedMatrix, domainMode, domain,
                                                     filterOrNullForBicubic);
}

sk_sp<GrTextureProxy> GrTextureMaker::generateTextureProxyForParams(const CopyParams& copyParams,
                                                                    bool willBeMipped,
                                                                    SkColorSpace* dstColorSpace) {
    sk_sp<GrTextureProxy> original(this->refOriginalTextureProxy(willBeMipped, dstColorSpace,
                                                                 AllowedTexGenType::kAny));
    if (!original) {
        return nullptr;
    }

    return CopyOnGpu(fContext, std::move(original), nullptr, copyParams);
}
