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
                                                                  bool willBeMipped,
                                                                  SkScalar scaleAdjust[2]) {
    if (this->width() > this->context()->priv().caps()->maxTextureSize() ||
        this->height() > this->context()->priv().caps()->maxTextureSize()) {
        return {};
    }

    CopyParams copyParams;

    GrSurfaceProxyView original = this->refOriginalTextureProxyView(willBeMipped,
                                                                    AllowedTexGenType::kCheap);
    bool needsCopyForMipsOnly = false;
    if (original.proxy()) {
        GrTextureProxy* texProxy = original.asTextureProxy();
        SkASSERT(texProxy);
        if (!params.isRepeated() ||
            !GrGpu::IsACopyNeededForRepeatWrapMode(this->context()->priv().caps(), texProxy,
                                                   texProxy->dimensions(), params.filter(),
                                                   &copyParams, scaleAdjust)) {
            needsCopyForMipsOnly = GrGpu::IsACopyNeededForMips(this->context()->priv().caps(),
                                                               texProxy, params.filter(),
                                                               &copyParams);
            if (!needsCopyForMipsOnly) {
                return original;
            }
        }
    } else {
        if (!params.isRepeated() ||
            !GrGpu::IsACopyNeededForRepeatWrapMode(this->context()->priv().caps(), nullptr,
                                                   this->dimensions(), params.filter(), &copyParams,
                                                   scaleAdjust)) {
            return this->refOriginalTextureProxyView(willBeMipped, AllowedTexGenType::kAny);
        }
    }

    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();

    GrSurfaceOrigin origOrigin = original.proxy() ? original.origin() : kTopLeft_GrSurfaceOrigin;
    GrUniqueKey copyKey;
    this->makeCopyKey(copyParams, &copyKey);
    GrSurfaceProxyView cachedView;
    if (copyKey.isValid()) {
        auto cachedProxy = proxyProvider->findOrCreateProxyByUniqueKey(copyKey, this->colorType());
        if (cachedProxy) {
            GrMipMapped mipped = cachedProxy->mipMapped();
            // TODO: Once we no longer use CopyOnGpu which can fallback to arbitrary formats and
            // colorTypes, we can use the swizzle of the originalView.
            GrSwizzle swizzle = cachedProxy->textureSwizzleDoNotUse();
            cachedView = GrSurfaceProxyView(std::move(cachedProxy), origOrigin, swizzle);
            if (!willBeMipped || GrMipMapped::kYes == mipped) {
                return cachedView;
            }
        }
    }

    GrSurfaceProxyView source;
    if (original.proxy()) {
        source = std::move(original);
    } else if (cachedView.proxy()) {
        source = cachedView;
    } else {
        // Since we will be copying this texture there is no reason to make it mipped
        source = this->refOriginalTextureProxyView(false, AllowedTexGenType::kAny);
    }

    if (!source.proxy()) {
        return {};
    }
    SkASSERT(source.asTextureProxy());

    GrSurfaceProxyView result =
            CopyOnGpu(this->context(), source, this->colorType(), copyParams, willBeMipped);

    if (!result.proxy()) {
        // If we were unable to make a copy and we only needed a copy for mips, then we will return
        // the source texture here and require that the GPU backend is able to fall back to using
        // bilerp if mips are required.
        if (needsCopyForMipsOnly) {
            return source;
        }
        return {};
    }

    if (copyKey.isValid()) {
        SkASSERT(result.origin() == origOrigin);
        if (cachedView.proxy()) {
            SkASSERT(GrMipMapped::kYes == result.asTextureProxy()->mipMapped() &&
                     GrMipMapped::kNo == cachedView.asTextureProxy()->mipMapped());
            // If we had a cachedProxy, that means there already is a proxy in the cache which
            // matches the key, but it does not have mip levels and we require them. Thus we must
            // remove the unique key from that proxy.
            SkASSERT(cachedView.asTextureProxy()->getUniqueKey() == copyKey);
            proxyProvider->removeUniqueKeyFromProxy(cachedView.asTextureProxy());
        }
        proxyProvider->assignUniqueKeyToProxy(copyKey, result.asTextureProxy());
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
    GrSurfaceProxyView view = this->viewForParams(filterOrNullForBicubic, scaleAdjust);
    if (!view.proxy()) {
        return nullptr;
    }
    SkMatrix adjustedMatrix = textureMatrix;
    adjustedMatrix.postScale(scaleAdjust[0], scaleAdjust[1]);

    SkRect domain;
    DomainMode domainMode =
        DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            view.proxy(), fmForDetermineDomain, &domain);
    SkASSERT(kTightCopy_DomainMode != domainMode);
    return this->createFragmentProcessorForDomainAndFilter(
            std::move(view), adjustedMatrix, domainMode, domain, filterOrNullForBicubic);
}
