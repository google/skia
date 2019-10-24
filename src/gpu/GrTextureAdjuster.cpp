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
                                     sk_sp<GrTextureProxy> original,
                                     const GrColorInfo& colorInfo,
                                     uint32_t uniqueID,
                                     bool useDecal)
        : INHERITED(context, {colorInfo, original->dimensions()}, useDecal)
        , fOriginal(std::move(original))
        , fUniqueID(uniqueID) {}

void GrTextureAdjuster::makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey) {
    // Destination color space is irrelevant - we already have a texture so we're just sub-setting
    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, fUniqueID, SkIRect::MakeSize(this->dimensions()));
    MakeCopyKeyFromOrigKey(baseKey, params, copyKey);
}

void GrTextureAdjuster::didCacheCopy(const GrUniqueKey& copyKey, uint32_t contextUniqueID) {
    // We don't currently have a mechanism for notifications on Images!
}

sk_sp<GrTextureProxy> GrTextureAdjuster::refTextureProxyCopy(const CopyParams& copyParams,
                                                             bool willBeMipped) {
    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();

    GrUniqueKey key;
    this->makeCopyKey(copyParams, &key);
    sk_sp<GrTextureProxy> cachedCopy;
    if (key.isValid()) {
        cachedCopy = proxyProvider->findOrCreateProxyByUniqueKey(key, this->colorType(),
                                                                 this->originalProxy()->origin());
        if (cachedCopy && (!willBeMipped || GrMipMapped::kYes == cachedCopy->mipMapped())) {
            return cachedCopy;
        }
    }

    sk_sp<GrTextureProxy> proxy = this->originalProxyRef();

    sk_sp<GrTextureProxy> copy = CopyOnGpu(this->context(), std::move(proxy), this->colorType(),
                                           copyParams, willBeMipped);
    if (copy) {
        if (key.isValid()) {
            SkASSERT(copy->origin() == this->originalProxy()->origin());
            if (cachedCopy) {
                SkASSERT(GrMipMapped::kYes == copy->mipMapped() &&
                         GrMipMapped::kNo == cachedCopy->mipMapped());
                // If we had a cachedProxy, that means there already is a proxy in the cache which
                // matches the key, but it does not have mip levels and we require them. Thus we
                // must remove the unique key from that proxy.
                SkASSERT(cachedCopy->getUniqueKey() == key);
                proxyProvider->removeUniqueKeyFromProxy(cachedCopy.get());
            }
            proxyProvider->assignUniqueKeyToProxy(key, copy.get());
            this->didCacheCopy(key, proxyProvider->contextID());
        }
    }
    return copy;
}

sk_sp<GrTextureProxy> GrTextureAdjuster::onRefTextureProxyForParams(
        const GrSamplerState& params,
        bool willBeMipped,
        SkScalar scaleAdjust[2]) {
    sk_sp<GrTextureProxy> proxy = this->originalProxyRef();
    CopyParams copyParams;

    if (this->context()->priv().abandoned()) {
        // The texture was abandoned.
        return nullptr;
    }

    SkASSERT(this->width() <= this->context()->priv().caps()->maxTextureSize() &&
             this->height() <= this->context()->priv().caps()->maxTextureSize());

    bool needsCopyForMipsOnly = false;
    if (!params.isRepeated() ||
        !GrGpu::IsACopyNeededForRepeatWrapMode(this->context()->priv().caps(), proxy.get(),
                                               proxy->dimensions(), params.filter(), &copyParams,
                                               scaleAdjust)) {
        needsCopyForMipsOnly = GrGpu::IsACopyNeededForMips(this->context()->priv().caps(),
                                                           proxy.get(), params.filter(),
                                                           &copyParams);
        if (!needsCopyForMipsOnly) {
            return proxy;
        }
    }

    sk_sp<GrTextureProxy> result = this->refTextureProxyCopy(copyParams, willBeMipped);
    if (!result && needsCopyForMipsOnly) {
        // If we were unable to make a copy and we only needed a copy for mips, then we will return
        // the source texture here and require that the GPU backend is able to fall back to using
        // bilerp if mips are required.
        return this->originalProxyRef();
    }
    return result;
}

std::unique_ptr<GrFragmentProcessor> GrTextureAdjuster::createFragmentProcessor(
        const SkMatrix& origTextureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        const GrSamplerState::Filter* filterOrNullForBicubic) {
    SkMatrix textureMatrix = origTextureMatrix;

    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(
            this->refTextureProxyForParams(filterOrNullForBicubic, scaleAdjust));
    if (!proxy) {
        return nullptr;
    }
    // If we made a copy then we only copied the contentArea, in which case the new texture is all
    // content.
    if (proxy.get() != this->originalProxy()) {
        textureMatrix.postScale(scaleAdjust[0], scaleAdjust[1]);
    }

    SkRect domain;
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
    return this->createFragmentProcessorForDomainAndFilter(
            std::move(proxy), textureMatrix, domainMode, domain, filterOrNullForBicubic);
}
