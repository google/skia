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
                                     SkAlphaType alphaType,
                                     const SkIRect& contentArea, uint32_t uniqueID,
                                     SkColorSpace* cs)
    : INHERITED(contentArea.width(), contentArea.height(),
                GrPixelConfigIsAlphaOnly(original->config()))
    , fContext(context)
    , fOriginal(std::move(original))
    , fAlphaType(alphaType)
    , fColorSpace(cs)
    , fUniqueID(uniqueID) {
    SkASSERT(SkIRect::MakeWH(fOriginal->width(), fOriginal->height()).contains(contentArea));
    SkASSERT(0 == contentArea.fLeft && 0 == contentArea.fTop);
    if (contentArea.fRight < fOriginal->width() || contentArea.fBottom < fOriginal->height()) {
        fContentArea.set(contentArea);
    }
}

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
    GrUniqueKey key;
    this->makeCopyKey(copyParams, &key, nullptr);
    if (key.isValid()) {
        sk_sp<GrTextureProxy> cachedCopy =
                fContext->resourceProvider()->findOrCreateProxyByUniqueKey(
                                                             key, this->originalProxy()->origin());
        if (cachedCopy) {
            return cachedCopy;
        }
    }

    sk_sp<GrTextureProxy> proxy = this->originalProxyRef();
    const SkIRect* contentArea = this->contentAreaOrNull();

    sk_sp<GrTextureProxy> copy = CopyOnGpu(fContext, std::move(proxy), contentArea, copyParams,
                                           willBeMipped);
    if (copy) {
        if (key.isValid()) {
            SkASSERT(copy->origin() == this->originalProxy()->origin());
            fContext->resourceProvider()->assignUniqueKeyToProxy(key, copy.get());
            this->didCacheCopy(key);
        }
    }
    return copy;
}

sk_sp<GrTextureProxy> GrTextureAdjuster::refTextureProxySafeForParams(const GrSamplerState& params,
                                                                      SkScalar scaleAdjust[2]) {
    sk_sp<GrTextureProxy> proxy = this->originalProxyRef();
    CopyParams copyParams;
    const SkIRect* contentArea = this->contentAreaOrNull();

    if (!fContext) {
        // The texture was abandoned.
        return nullptr;
    }

    bool willBeMipped = GrSamplerState::Filter::kMipMap == params.filter();
    if (contentArea && willBeMipped) {
        // If we generate a MIP chain for texture it will read pixel values from outside the content
        // area.
        copyParams.fWidth = contentArea->width();
        copyParams.fHeight = contentArea->height();
        copyParams.fFilter = GrSamplerState::Filter::kBilerp;
    } else if (!fContext->getGpu()->isACopyNeededForTextureParams(proxy.get(), params, &copyParams,
                                                                  scaleAdjust)) {
        return proxy;
    }

    return this->refTextureProxyCopy(copyParams, willBeMipped);
}

std::unique_ptr<GrFragmentProcessor> GrTextureAdjuster::createFragmentProcessor(
        const SkMatrix& origTextureMatrix,
        const SkRect& origConstraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        const GrSamplerState::Filter* filterOrNullForBicubic,
        SkColorSpace* dstColorSpace) {
    SkMatrix textureMatrix = origTextureMatrix;
    const SkIRect* contentArea = this->contentAreaOrNull();
    // Convert the constraintRect to be relative to the texture rather than the content area so
    // that both rects are in the same coordinate system.
    SkTCopyOnFirstWrite<SkRect> constraintRect(origConstraintRect);
    if (contentArea) {
        SkScalar l = SkIntToScalar(contentArea->fLeft);
        SkScalar t = SkIntToScalar(contentArea->fTop);
        constraintRect.writable()->offset(l, t);
        textureMatrix.postTranslate(l, t);
    }

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
        contentArea = nullptr;
        textureMatrix.postScale(scaleAdjust[0], scaleAdjust[1]);
    }

    DomainMode domainMode =
        DetermineDomainMode(*constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            proxy.get(),
                            contentArea, filterOrNullForBicubic,
                            &domain);
    if (kTightCopy_DomainMode == domainMode) {
        // TODO: Copy the texture and adjust the texture matrix (both parts need to consider
        // non-int constraint rect)
        // For now: treat as bilerp and ignore what goes on above level 0.

        // We only expect MIP maps to require a tight copy.
        SkASSERT(filterOrNullForBicubic &&
                 GrSamplerState::Filter::kMipMap == *filterOrNullForBicubic);
        static const GrSamplerState::Filter kBilerp = GrSamplerState::Filter::kBilerp;
        domainMode =
            DetermineDomainMode(*constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                                proxy.get(),
                                contentArea, &kBilerp, &domain);
        SkASSERT(kTightCopy_DomainMode != domainMode);
    }
    SkASSERT(kNoDomain_DomainMode == domainMode ||
             (domain.fLeft <= domain.fRight && domain.fTop <= domain.fBottom));
    auto fp = CreateFragmentProcessorForDomainAndFilter(std::move(proxy), textureMatrix,
                                                        domainMode, domain, filterOrNullForBicubic);
    return GrColorSpaceXformEffect::Make(std::move(fp), fColorSpace, dstColorSpace);
}
