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
                                     uint32_t uniqueID)
        : INHERITED(context, {colorInfo, original.proxy()->dimensions()})
        , fOriginal(std::move(original))
        , fUniqueID(uniqueID) {}

GrSurfaceProxyView GrTextureAdjuster::makeMippedCopy() {
    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();

    GrUniqueKey baseKey, mipMappedKey;
    GrMakeKeyFromImageID(&baseKey, fUniqueID, SkIRect::MakeSize(this->dimensions()));
    if (baseKey.isValid()) {
        static const GrUniqueKey::Domain kMipMappedDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&mipMappedKey, baseKey, kMipMappedDomain, 0);
    }
    sk_sp<GrTextureProxy> cachedCopy;
    if (mipMappedKey.isValid()) {
        cachedCopy = proxyProvider->findOrCreateProxyByUniqueKey(mipMappedKey);
        if (cachedCopy) {
            return {std::move(cachedCopy), fOriginal.origin(), fOriginal.swizzle()};
        }
    }

    GrSurfaceProxyView copyView = GrCopyBaseMipMapToTextureProxy(
            this->context(), fOriginal.proxy(), fOriginal.origin(), this->colorType());
    if (!copyView) {
        return {};
    }
    if (mipMappedKey.isValid()) {
        SkASSERT(copyView.origin() == fOriginal.origin());
        // TODO: If we move listeners up from SkImage_Lazy to SkImage_Base then add one here.
        proxyProvider->assignUniqueKeyToProxy(mipMappedKey, copyView.asTextureProxy());
    }
    return copyView;
}

GrSurfaceProxyView GrTextureAdjuster::onView(GrMipMapped mipMapped) {
    if (this->context()->priv().abandoned()) {
        // The texture was abandoned.
        return {};
    }

    SkASSERT(this->width() <= this->context()->priv().caps()->maxTextureSize() &&
             this->height() <= this->context()->priv().caps()->maxTextureSize());

    GrTextureProxy* texProxy = fOriginal.asTextureProxy();
    SkASSERT(texProxy);
    if (mipMapped == GrMipMapped::kNo || texProxy->mipMapped() == GrMipMapped::kYes) {
        return fOriginal;
    }

    GrSurfaceProxyView copy = this->makeMippedCopy();
    if (!copy) {
        // If we were unable to make a copy and we only needed a copy for mips, then we will return
        // the source texture here and require that the GPU backend is able to fall back to using
        // bilerp if mips are required.
        return fOriginal;
    }
    SkASSERT(copy.asTextureProxy());
    return copy;
}

std::unique_ptr<GrFragmentProcessor> GrTextureAdjuster::createFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        GrSamplerState::WrapMode wrapX,
        GrSamplerState::WrapMode wrapY,
        const GrSamplerState::Filter* filterOrNullForBicubic) {
    GrSurfaceProxyView view;
    if (filterOrNullForBicubic) {
        view = this->view(*filterOrNullForBicubic);
    } else {
        view = this->view(GrMipMapped::kNo);
    }
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
    return this->createFragmentProcessorForSubsetAndFilter(std::move(view), textureMatrix,
                                                           domainMode, domain, wrapX, wrapY,
                                                           filterOrNullForBicubic);
}
