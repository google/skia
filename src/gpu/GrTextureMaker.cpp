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
#include "src/gpu/SkGr.h"

GrSurfaceProxyView GrTextureMaker::onView(GrMipMapped mipMapped) {
    if (this->width() > this->context()->priv().caps()->maxTextureSize() ||
        this->height() > this->context()->priv().caps()->maxTextureSize()) {
        return {};
    }
    return this->refOriginalTextureProxyView(mipMapped);
}

std::unique_ptr<GrFragmentProcessor> GrTextureMaker::createFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        GrSamplerState::WrapMode wrapX,
        GrSamplerState::WrapMode wrapY,
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

    GrSurfaceProxyView view;
    if (filterOrNullForBicubic) {
        view = this->view(*filterOrNullForBicubic);
    } else {
        view = this->view(GrMipMapped::kNo);
    }
    if (!view) {
        return nullptr;
    }

    SkRect domain;
    DomainMode domainMode =
        DetermineDomainMode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            view.proxy(), fmForDetermineDomain, &domain);
    SkASSERT(kTightCopy_DomainMode != domainMode);
    return this->createFragmentProcessorForSubsetAndFilter(std::move(view), textureMatrix,
                                                           domainMode, domain, wrapX, wrapY,
                                                           filterOrNullForBicubic);
}
