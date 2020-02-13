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

void GrTextureAdjuster::makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey) {
    // Destination color space is irrelevant - we already have a texture so we're just sub-setting
    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, fUniqueID, SkIRect::MakeSize(this->dimensions()));
    MakeCopyKeyFromOrigKey(baseKey, params, copyKey);
}

void GrTextureAdjuster::didCacheCopy(const GrUniqueKey& copyKey, uint32_t contextUniqueID) {
    // We don't currently have a mechanism for notifications on Images!
}

GrSurfaceProxyView GrTextureAdjuster::copy(const CopyParams& copyParams, bool willBeMipped,
                                           bool copyForMipsOnly) {
    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();

    GrUniqueKey key;
    this->makeCopyKey(copyParams, &key);
    sk_sp<GrTextureProxy> cachedCopy;
    const GrSurfaceProxyView& originalView = this->originalProxyView();
    if (key.isValid()) {
        cachedCopy = proxyProvider->findOrCreateProxyByUniqueKey(key, this->colorType());
        if (cachedCopy && (!willBeMipped || GrMipMapped::kYes == cachedCopy->mipMapped())) {
            // TODO: Once we no longer use CopyOnGpu which can fallback to arbitrary formats and
            // colorTypes, we can use the swizzle of the originalView.
            GrSwizzle swizzle = cachedCopy->textureSwizzleDoNotUse();
            return GrSurfaceProxyView(std::move(cachedCopy), originalView.origin(), swizzle);
        }
    }

    GrSurfaceProxyView copyView;
    if (copyForMipsOnly) {
        copyView = GrCopyBaseMipMapToTextureProxy(this->context(), originalView.proxy(),
                                                  originalView.origin(), this->colorType());
    } else {
        copyView = CopyOnGpu(this->context(), this->originalProxyViewRef(), this->colorType(),
                             copyParams, willBeMipped);
    }
    if (copyView.proxy()) {
        if (key.isValid()) {
            SkASSERT(copyView.origin() == originalView.origin());
            if (cachedCopy) {
                SkASSERT(GrMipMapped::kYes == copyView.asTextureProxy()->mipMapped() &&
                         GrMipMapped::kNo == cachedCopy->mipMapped());
                // If we had a cachedProxy, that means there already is a proxy in the cache which
                // matches the key, but it does not have mip levels and we require them. Thus we
                // must remove the unique key from that proxy.
                SkASSERT(cachedCopy->getUniqueKey() == key);
                proxyProvider->removeUniqueKeyFromProxy(cachedCopy.get());
            }
            proxyProvider->assignUniqueKeyToProxy(key, copyView.asTextureProxy());
            this->didCacheCopy(key, proxyProvider->contextID());
        }
    }
    return copyView;
}

GrSurfaceProxyView GrTextureAdjuster::onRefTextureProxyViewForParams(GrSamplerState params,
                                                                     bool willBeMipped,
                                                                     SkScalar scaleAdjust[2]) {
    if (this->context()->priv().abandoned()) {
        // The texture was abandoned.
        return {};
    }

    SkASSERT(this->width() <= this->context()->priv().caps()->maxTextureSize() &&
             this->height() <= this->context()->priv().caps()->maxTextureSize());

    GrSurfaceProxyView view = this->originalProxyViewRef();
    GrTextureProxy* texProxy = view.asTextureProxy();
    SkASSERT(texProxy);
    CopyParams copyParams;

    bool needsCopyForMipsOnly = false;
    if (!params.isRepeated() ||
        !GrGpu::IsACopyNeededForRepeatWrapMode(this->context()->priv().caps(), texProxy,
                                               texProxy->dimensions(), params.filter(), &copyParams,
                                               scaleAdjust)) {
        needsCopyForMipsOnly = GrGpu::IsACopyNeededForMips(this->context()->priv().caps(),
                                                           texProxy, params.filter(),
                                                           &copyParams);
        if (!needsCopyForMipsOnly) {
            return view;
        }
    }

    GrSurfaceProxyView result = this->copy(copyParams, willBeMipped, needsCopyForMipsOnly);
    if (!result.proxy() && needsCopyForMipsOnly) {
        // If we were unable to make a copy and we only needed a copy for mips, then we will return
        // the source texture here and require that the GPU backend is able to fall back to using
        // bilerp if mips are required.
        return view;
    }
    SkASSERT(result.asTextureProxy());
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
    GrSurfaceProxyView view = this->viewForParams(filterOrNullForBicubic, scaleAdjust);
    if (!view.proxy()) {
        return nullptr;
    }
    SkASSERT(view.asTextureProxy());
    // If we made a copy then we only copied the contentArea, in which case the new texture is all
    // content.
    if (view.proxy() != this->originalProxyView().proxy()) {
        textureMatrix.postScale(scaleAdjust[0], scaleAdjust[1]);
    }

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
