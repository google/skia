/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTextureMaker.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"

GrSurfaceProxyView GrTextureMaker::onRefTextureProxyViewForParams(GrSamplerState params,
                                                                  bool willBeMipped) {
    if (this->width() > this->context()->priv().caps()->maxTextureSize() ||
        this->height() > this->context()->priv().caps()->maxTextureSize()) {
        return {};
    }

    GrSurfaceProxyView original = this->refOriginalTextureProxyView(willBeMipped,
                                                                    AllowedTexGenType::kCheap);
    if (!original) {
        return this->refOriginalTextureProxyView(willBeMipped, AllowedTexGenType::kAny);
    }

    GrTextureProxy* texProxy = original.asTextureProxy();
    if (!GrGpu::IsACopyNeededForMips(this->context()->priv().caps(), texProxy, params.filter())) {
        return original;
    }

    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();

    GrSurfaceOrigin origOrigin = original.proxy() ? original.origin() : kTopLeft_GrSurfaceOrigin;
    GrUniqueKey mipMappedKey;
    this->makeMipMappedKey(&mipMappedKey);
    if (mipMappedKey.isValid()) {
        auto cachedProxy =
                proxyProvider->findOrCreateProxyByUniqueKey(mipMappedKey, this->colorType());
        if (cachedProxy) {
            SkASSERT(cachedProxy->mipMapped() == GrMipMapped::kYes);
            // TODO: Once we no longer use MakeMipMappedCopy which can fallback to arbitrary formats
            // and colorTypes, we can use the swizzle of the originalView.
            GrSwizzle swizzle = cachedProxy->textureSwizzleDoNotUse();
            return GrSurfaceProxyView(std::move(cachedProxy), origOrigin, swizzle);
        }
    }

    GrSurfaceProxyView source;
    if (original) {
        source = std::move(original);
    } else {
        // Since we will be copying this texture there is no reason to make it mipped
        source = this->refOriginalTextureProxyView(false, AllowedTexGenType::kAny);
        if (!source) {
            return {};
        }
    }

    SkASSERT(source.asTextureProxy());

    GrSurfaceProxyView result = MakeMipMappedCopy(this->context(), source, this->colorType());

    if (!result) {
        // If we were unable to make a copy and we only needed a copy for mips, then we will return
        // the source texture here and require that the GPU backend is able to fall back to using
        // bilerp if mips are required.
        return source;
    }

    if (mipMappedKey.isValid()) {
        SkASSERT(result.origin() == origOrigin);
        proxyProvider->assignUniqueKeyToProxy(mipMappedKey, result.asTextureProxy());
        this->didCacheMipMappedCopy(mipMappedKey, proxyProvider->contextID());
    }
    return result;
}

std::unique_ptr<GrFragmentProcessor> GrTextureMaker::createFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        const GrSamplerState::Filter* filterOrNullForBicubic) {
    const GrSamplerState::Filter* fmForDetermineDomain = filterOrNullForBicubic;
    if (filterOrNullForBicubic && GrSamplerState::Filter::kMipMap == *filterOrNullForBicubic &&
        kYes_FilterConstraint == filterConstraint) {
        // TODO: Here we should force a copy restricted to the constraintRect since MIP maps will
        // read outside the constraint rect. However, as in the adjuster case, we aren't currently
        // doing that.
        // We instead we compute the domain as though were bilerping which is only correct if we
        // only sample level 0.
        static const GrSamplerState::Filter kBilerp = GrSamplerState::Filter::kBilerp;
        fmForDetermineDomain = &kBilerp;
    }

    GrSurfaceProxyView view = this->viewForParams(filterOrNullForBicubic);
    if (!view) {
        return nullptr;
    }

    SkRect domain;
    DomainMode domainMode =
        DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            view.proxy(), fmForDetermineDomain, &domain);
    SkASSERT(kTightCopy_DomainMode != domainMode);
    return this->createFragmentProcessorForDomainAndFilter(
            std::move(view), textureMatrix, domainMode, domain, filterOrNullForBicubic);
}
