/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTextureAdjuster.h"
#include "src/gpu/SkGr.h"

GrTextureAdjuster::GrTextureAdjuster(GrRecordingContext* context,
                                     GrSurfaceProxyView original,
                                     const GrColorInfo& colorInfo,
                                     uint32_t uniqueID,
                                     bool useDecal)
        : INHERITED(context, {colorInfo, original.proxy()->dimensions()}, useDecal)
        , fOriginal(std::move(original))
        , fUniqueID(uniqueID) {}

void GrTextureAdjuster::makeMipMappedKey(GrUniqueKey* mipMappedKey) {
    // Destination color space is irrelevant - we already have a texture so we're just sub-setting
    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, fUniqueID, SkIRect::MakeSize(this->dimensions()));
    MakeMipMappedKeyFromOriginalKey(baseKey, mipMappedKey);
}

void GrTextureAdjuster::didCacheMipMappedCopy(const GrUniqueKey& mipMappedKey,
                                              uint32_t contextUniqueID) {
    // We don't currently have a mechanism for notifications on Images!
}

GrSurfaceProxyView GrTextureAdjuster::makeMippedCopy() {
    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();

    GrUniqueKey key;
    this->makeMipMappedKey(&key);
    sk_sp<GrTextureProxy> cachedCopy;
    const GrSurfaceProxyView& originalView = this->originalProxyView();
    if (key.isValid()) {
        cachedCopy = proxyProvider->findOrCreateProxyByUniqueKey(key, this->colorType());
        if (cachedCopy) {
            return {std::move(cachedCopy), originalView.origin(), originalView.swizzle()};
        }
    }

    GrSurfaceProxyView copyView = GrCopyBaseMipMapToTextureProxy(
            this->context(), originalView.proxy(), originalView.origin(), this->colorType());
    if (!copyView) {
        return {};
    }
    if (key.isValid()) {
        SkASSERT(copyView.origin() == originalView.origin());
        proxyProvider->assignUniqueKeyToProxy(key, copyView.asTextureProxy());
        this->didCacheMipMappedCopy(key, proxyProvider->contextID());
    }
    return copyView;
}

GrSurfaceProxyView GrTextureAdjuster::onRefTextureProxyViewForParams(GrSamplerState params,
                                                                     bool willBeMipped) {
    if (this->context()->priv().abandoned()) {
        // The texture was abandoned.
        return {};
    }

    SkASSERT(this->width() <= this->context()->priv().caps()->maxTextureSize() &&
             this->height() <= this->context()->priv().caps()->maxTextureSize());

    GrSurfaceProxyView view = this->originalProxyViewRef();
    GrTextureProxy* texProxy = view.asTextureProxy();
    SkASSERT(texProxy);
    if (!GrGpu::IsACopyNeededForMips(this->context()->priv().caps(), texProxy, params.filter())) {
        return view;
    }

    GrSurfaceProxyView copy = this->makeMippedCopy();
    if (!copy) {
        // If we were unable to make a copy and we only needed a copy for mips, then we will return
        // the source texture here and require that the GPU backend is able to fall back to using
        // bilerp if mips are required.
        return view;
    }
    SkASSERT(copy.asTextureProxy());
    return copy;
}

std::unique_ptr<GrFragmentProcessor> GrTextureAdjuster::createFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        const GrSamplerState::Filter* filterOrNullForBicubic) {
    GrSurfaceProxyView view = this->viewForParams(filterOrNullForBicubic);
    if (!view) {
        return nullptr;
    }
    SkASSERT(view.asTextureProxy());

    SkRect domain;
    DomainMode domainMode =
        DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            view.proxy(), filterOrNullForBicubic, &domain);
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
                                view.proxy(), &kBilerp, &domain);
        SkASSERT(kTightCopy_DomainMode != domainMode);
    }
    SkASSERT(kNoDomain_DomainMode == domainMode ||
             (domain.fLeft <= domain.fRight && domain.fTop <= domain.fBottom));
    return this->createFragmentProcessorForDomainAndFilter(
            std::move(view), textureMatrix, domainMode, domain, filterOrNullForBicubic);
}
