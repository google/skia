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

sk_sp<GrTextureProxy> GrTextureMaker::onRefTextureProxyForParams(const GrSamplerState& params,
                                                                 bool willBeMipped,
                                                                 SkScalar scaleAdjust[2]) {
    if (this->width() > this->context()->priv().caps()->maxTextureSize() ||
        this->height() > this->context()->priv().caps()->maxTextureSize()) {
        return nullptr;
    }

    CopyParams copyParams;

    sk_sp<GrTextureProxy> original(this->refOriginalTextureProxy(willBeMipped,
                                                                 AllowedTexGenType::kCheap));
    bool needsCopyForMipsOnly = false;
    if (original) {
        if (!params.isRepeated() ||
            !GrGpu::IsACopyNeededForRepeatWrapMode(this->context()->priv().caps(), original.get(),
                                                   original->width(), original->height(),
                                                   params.filter(), &copyParams, scaleAdjust)) {
            needsCopyForMipsOnly = GrGpu::IsACopyNeededForMips(this->context()->priv().caps(),
                                                               original.get(), params.filter(),
                                                               &copyParams);
            if (!needsCopyForMipsOnly) {
                return original;
            }
        }
    } else {
        if (!params.isRepeated() ||
            !GrGpu::IsACopyNeededForRepeatWrapMode(this->context()->priv().caps(), nullptr,
                                                   this->width(), this->height(),
                                                   params.filter(), &copyParams, scaleAdjust)) {
            return this->refOriginalTextureProxy(willBeMipped, AllowedTexGenType::kAny);
        }
    }

    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();

    GrSurfaceOrigin origOrigin = original ? original->origin() : kTopLeft_GrSurfaceOrigin;
    GrUniqueKey copyKey;
    this->makeCopyKey(copyParams, &copyKey);
    sk_sp<GrTextureProxy> cachedProxy;
    if (copyKey.isValid()) {
        cachedProxy = proxyProvider->findOrCreateProxyByUniqueKey(copyKey, origOrigin);
        if (cachedProxy && (!willBeMipped || GrMipMapped::kYes == cachedProxy->mipMapped())) {
            return cachedProxy;
        }
    }

    sk_sp<GrTextureProxy> source;
    if (original) {
        source = std::move(original);
    } else if (cachedProxy) {
        source = cachedProxy;
    } else {
        // Since we will be copying this texture there is no reason to make it mipped
        source = this->refOriginalTextureProxy(false, AllowedTexGenType::kAny);
    }

    if (!source) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> result =
            CopyOnGpu(this->context(), source, this->colorType(), copyParams, willBeMipped);

    if (!result) {
        // If we were unable to make a copy and we only needed a copy for mips, then we will return
        // the source texture here and require that the GPU backend is able to fall back to using
        // bilerp if mips are required.
        if (needsCopyForMipsOnly) {
            return source;
        }
        return nullptr;
    }

    if (copyKey.isValid()) {
        SkASSERT(result->origin() == origOrigin);
        if (cachedProxy) {
            SkASSERT(GrMipMapped::kYes == result->mipMapped() &&
                     GrMipMapped::kNo == cachedProxy->mipMapped());
            // If we had a cachedProxy, that means there already is a proxy in the cache which
            // matches the key, but it does not have mip levels and we require them. Thus we must
            // remove the unique key from that proxy.
            SkASSERT(cachedProxy->getUniqueKey() == copyKey);
            proxyProvider->removeUniqueKeyFromProxy(cachedProxy.get());
        }
        proxyProvider->assignUniqueKeyToProxy(copyKey, result.get());
        this->didCacheCopy(copyKey, proxyProvider->contextID());
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

    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(this->refTextureProxyForParams(filterOrNullForBicubic,
                                                               scaleAdjust));
    if (!proxy) {
        return nullptr;
    }
    SkMatrix adjustedMatrix = textureMatrix;
    adjustedMatrix.postScale(scaleAdjust[0], scaleAdjust[1]);

    SkRect domain;
    DomainMode domainMode =
        DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            proxy.get(), fmForDetermineDomain, &domain);
    SkASSERT(kTightCopy_DomainMode != domainMode);
    return this->createFragmentProcessorForDomainAndFilter(
            std::move(proxy), adjustedMatrix, domainMode, domain, filterOrNullForBicubic);
}
