/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTextureMaker.h"

#include "include/gpu/GrRecordingContext.h"
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
    GrSurfaceProxyView view;
    if (filterOrNullForBicubic) {
        view = this->view(*filterOrNullForBicubic);
    } else {
        view = this->view(GrMipMapped::kNo);
    }
    if (!view) {
        return nullptr;
    }

    return this->createFragmentProcessorForSubsetAndFilter(std::move(view),
                                                           textureMatrix,
                                                           coordsLimitedToConstraintRect,
                                                           filterConstraint,
                                                           constraintRect,
                                                           wrapX,
                                                           wrapY,
                                                           filterOrNullForBicubic);
}
