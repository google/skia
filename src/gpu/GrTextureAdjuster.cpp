/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureAdjuster.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "SkGr.h"

GrTextureAdjuster::GrTextureAdjuster(GrContext* context, sk_sp<GrTextureProxy> original,
                                     SkAlphaType alphaType, uint32_t uniqueID, SkColorSpace* cs)
        : INHERITED(original->width(), original->height(),
                    GrPixelConfigIsAlphaOnly(original->config()))
        , fContext(context)
        , fOriginal(std::move(original))
        , fAlphaType(alphaType)
        , fColorSpace(cs)
        , fUniqueID(uniqueID) {}

void GrTextureAdjuster::makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey,
                                    SkColorSpace* dstColorSpace) {
    // Destination color space is irrelevant - we already have a texture so we're just sub-setting
    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, fUniqueID, SkIRect::MakeWH(this->width(), this->height()));
    MakeCopyKeyFromOrigKey(baseKey, params, copyKey);
}

void GrTextureAdjuster::didCacheCopy(const GrUniqueKey& copyKey) {
    // We don't currently have a mechanism for notifications on Images!
}

sk_sp<GrTextureProxy> GrTextureAdjuster::refTextureProxyCopy(const CopyParams& copyParams) {
    GrUniqueKey key;
    this->makeCopyKey(copyParams, &key, nullptr);
    if (key.isValid()) {
        sk_sp<GrTextureProxy> cachedCopy = fContext->resourceProvider()->findProxyByUniqueKey(
                                                            key, this->originalProxy()->origin());
        if (cachedCopy) {
            return cachedCopy;
        }
    }

    sk_sp<GrTextureProxy> proxy = this->originalProxyRef();

    sk_sp<GrTextureProxy> copy = CopyOnGpu(fContext, std::move(proxy), copyParams);
    if (copy) {
        if (key.isValid()) {
            SkASSERT(copy->origin() == this->originalProxy()->origin());
            fContext->resourceProvider()->assignUniqueKeyToProxy(key, copy.get());
            this->didCacheCopy(key);
        }
    }
    return copy;
}

sk_sp<GrTextureProxy> GrTextureAdjuster::refTextureProxySafeForParams(
                                                                 const GrSamplerParams& params,
                                                                 SkScalar scaleAdjust[2]) {
    sk_sp<GrTextureProxy> proxy = this->originalProxyRef();
    CopyParams copyParams;

    if (!fContext) {
        // The texture was abandoned.
        return nullptr;
    }
    if (!fContext->getGpu()->isACopyNeededForTextureParams(proxy.get(), params, &copyParams,
                                                           scaleAdjust)) {
        return proxy;
    }

    sk_sp<GrTextureProxy> copy = this->refTextureProxyCopy(copyParams);
    return copy;
}

std::unique_ptr<GrFragmentProcessor> GrTextureAdjuster::createFragmentProcessor(
        const SkMatrix& origTextureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        const GrSamplerParams::FilterMode* filterOrNullForBicubic,
        SkColorSpace* dstColorSpace) {
    SkMatrix textureMatrix = origTextureMatrix;

    SkRect domain;
    GrSamplerParams params;
    if (filterOrNullForBicubic) {
        params.setFilterMode(*filterOrNullForBicubic);
    }
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(this->refTextureProxySafeForParams(params, scaleAdjust));
    if (!proxy) {
        return nullptr;
    }
    // If we made a copy then we only copied the contentArea, in which case the new texture is all
    // content.
    if (proxy.get() != this->originalProxy()) {
        textureMatrix.postScale(scaleAdjust[0], scaleAdjust[1]);
    }

    DomainMode domainMode =
            DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                                proxy.get(), filterOrNullForBicubic, &domain);
    if (kTightCopy_DomainMode == domainMode) {
        // TODO: Copy the texture and adjust the texture matrix (both parts need to consider
        // non-int constraint rect)
        // For now: treat as bilerp and ignore what goes on above level 0.

        // We only expect MIP maps to require a tight copy.
        SkASSERT(filterOrNullForBicubic &&
                 GrSamplerParams::kMipMap_FilterMode == *filterOrNullForBicubic);
        static const GrSamplerParams::FilterMode kBilerp = GrSamplerParams::kBilerp_FilterMode;
        domainMode =
                DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                                    proxy.get(), &kBilerp, &domain);
        SkASSERT(kTightCopy_DomainMode != domainMode);
    }
    SkASSERT(kNoDomain_DomainMode == domainMode ||
             (domain.fLeft <= domain.fRight && domain.fTop <= domain.fBottom));
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(fColorSpace,
                                                                       dstColorSpace);
    return CreateFragmentProcessorForDomainAndFilter(std::move(proxy),
                                                     std::move(colorSpaceXform),
                                                     textureMatrix, domainMode, domain,
                                                     filterOrNullForBicubic);
}
