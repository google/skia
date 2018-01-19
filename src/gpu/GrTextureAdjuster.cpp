/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureAdjuster.h"

#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrProxyProvider.h"
#include "SkGr.h"

GrTextureAdjuster::GrTextureAdjuster(GrContext* context, sk_sp<GrTextureProxy> original,
                                     SkAlphaType alphaType,
                                     uint32_t uniqueID,
                                     SkColorSpace* cs)
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

sk_sp<GrTextureProxy> GrTextureAdjuster::refTextureProxyCopy(const CopyParams& copyParams,
                                                             bool willBeMipped) {
    GrProxyProvider* proxyProvider = fContext->contextPriv().proxyProvider();

    GrUniqueKey key;
    this->makeCopyKey(copyParams, &key, nullptr);
    if (key.isValid()) {
        sk_sp<GrTextureProxy> cachedCopy =
                proxyProvider->findOrCreateProxyByUniqueKey(key, this->originalProxy()->origin());
        if (cachedCopy) {
            return cachedCopy;
        }
    }

    sk_sp<GrTextureProxy> proxy = this->originalProxyRef();

    sk_sp<GrTextureProxy> copy = CopyOnGpu(fContext, std::move(proxy), copyParams, willBeMipped);
    if (copy) {
        if (key.isValid()) {
            SkASSERT(copy->origin() == this->originalProxy()->origin());
            proxyProvider->assignUniqueKeyToProxy(key, copy.get());
            this->didCacheCopy(key);
        }
    }
    return copy;
}

sk_sp<GrTextureProxy> GrTextureAdjuster::refTextureProxySafeForParams(const GrSamplerState& params,
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

    bool willBeMipped = GrSamplerState::Filter::kMipMap == params.filter();
    return this->refTextureProxyCopy(copyParams, willBeMipped);
}

std::unique_ptr<GrFragmentProcessor> GrTextureAdjuster::createFragmentProcessor(
        const SkMatrix& origTextureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        const GrSamplerState::Filter* filterOrNullForBicubic,
        SkColorSpace* dstColorSpace) {
    SkMatrix textureMatrix = origTextureMatrix;

    SkRect domain;
    GrSamplerState samplerState;
    if (filterOrNullForBicubic) {
        samplerState.setFilterMode(*filterOrNullForBicubic);
    }
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(
            this->refTextureProxySafeForParams(samplerState, scaleAdjust));
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
                 GrSamplerState::Filter::kMipMap == *filterOrNullForBicubic);
        static const GrSamplerState::Filter kBilerp = GrSamplerState::Filter::kBilerp;
        domainMode =
            DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                                proxy.get(), &kBilerp, &domain);
        SkASSERT(kTightCopy_DomainMode != domainMode);
    }
    SkASSERT(kNoDomain_DomainMode == domainMode ||
             (domain.fLeft <= domain.fRight && domain.fTop <= domain.fBottom));
    GrPixelConfig config = proxy->config();
    auto fp = CreateFragmentProcessorForDomainAndFilter(std::move(proxy), textureMatrix,
                                                        domainMode, domain, filterOrNullForBicubic);
    return GrColorSpaceXformEffect::Make(std::move(fp), fColorSpace, config, dstColorSpace);
}
