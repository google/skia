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

sk_sp<GrTextureProxy> GrTextureMaker::refTextureProxyForParams(const GrSamplerState& params,
                                                               SkColorSpace* dstColorSpace,
                                                               sk_sp<SkColorSpace>* texColorSpace,
                                                               SkScalar scaleAdjust[2]) {
    CopyParams copyParams;
    bool willBeMipped = params.filter() == GrSamplerState::Filter::kMipMap;

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

    GrSurfaceOrigin origOrigin;
    GrUniqueKey copyKey;
    this->makeCopyKey(copyParams, &copyKey, dstColorSpace);
    if (copyKey.isValid()) {
        if (original) {
            origOrigin = original->origin();
        } else {
            origOrigin = kTopLeft_GrSurfaceOrigin;
        }
        sk_sp<GrTextureProxy> result(fContext->resourceProvider()->findOrCreateProxyByUniqueKey(
                                                                            copyKey, origOrigin));
        if (result) {
            return result;
        }
    }

    sk_sp<GrTextureProxy> result;
    if (original) {
        result = CopyOnGpu(fContext, std::move(original), nullptr, copyParams, willBeMipped);
    } else {
        result = this->generateTextureProxyForParams(copyParams, willBeMipped, dstColorSpace);
    }

    if (!result) {
        return nullptr;
    }

    if (copyKey.isValid()) {
        SkASSERT(result->origin() == origOrigin);
        fContext->resourceProvider()->assignUniqueKeyToProxy(copyKey, result.get());
        this->didCacheCopy(copyKey);
    }
    return result;
}

std::unique_ptr<GrFragmentProcessor> GrTextureMaker::createFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        const GrSamplerState::Filter* filterOrNullForBicubic,
        SkColorSpace* dstColorSpace) {
    const GrSamplerState::Filter* fmForDetermineDomain = filterOrNullForBicubic;
    if (filterOrNullForBicubic && GrSamplerState::Filter::kMipMap == *filterOrNullForBicubic &&
        kYes_FilterConstraint == filterConstraint) {
        // TODo: Here we should force a copy restricted to the constraintRect since MIP maps will
        // read outside the constraint rect. However, as in the adjuster case, we aren't currently
        // doing that.
        // We instead we compute the domain as though were bilerping which is only correct if we
        // only sample level 0.
        static const GrSamplerState::Filter kBilerp = GrSamplerState::Filter::kBilerp;
        fmForDetermineDomain = &kBilerp;
    }

    GrSamplerState samplerState;
    if (filterOrNullForBicubic) {
        samplerState = GrSamplerState(GrSamplerState::WrapMode::kClamp, *filterOrNullForBicubic);
    } else {
        // Bicubic doesn't use filtering for it's texture accesses.
        samplerState = GrSamplerState::ClampNearest();
    }
    sk_sp<SkColorSpace> texColorSpace;
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(this->refTextureProxyForParams(samplerState, dstColorSpace,
                                                               &texColorSpace, scaleAdjust));
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
    auto fp = CreateFragmentProcessorForDomainAndFilter(std::move(proxy), adjustedMatrix,
                                                        domainMode, domain, filterOrNullForBicubic);
    return GrColorSpaceXformEffect::Make(std::move(fp), texColorSpace.get(), dstColorSpace);
}

sk_sp<GrTextureProxy> GrTextureMaker::generateTextureProxyForParams(const CopyParams& copyParams,
                                                                    bool willBeMipped,
                                                                    SkColorSpace* dstColorSpace) {
    sk_sp<GrTextureProxy> original(this->refOriginalTextureProxy(willBeMipped, dstColorSpace,
                                                                 AllowedTexGenType::kAny));
    if (!original) {
        return nullptr;
    }

    return CopyOnGpu(fContext, std::move(original), nullptr, copyParams, willBeMipped);
}
