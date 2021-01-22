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

GrSurfaceProxyView GrTextureMaker::onView(GrMipmapped mipMapped) {
    if (this->width() > this->context()->priv().caps()->maxTextureSize() ||
        this->height() > this->context()->priv().caps()->maxTextureSize()) {
        return {};
    }
    return this->refOriginalTextureProxyView(mipMapped);
}

std::unique_ptr<GrFragmentProcessor> GrTextureMaker::createFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect* subset,
        const SkRect* domain,
        GrSamplerState sampler) {
    GrSurfaceProxyView view;
    return this->createFragmentProcessorForView(
            this->view(sampler.mipmapped()), textureMatrix, subset, domain, sampler);
}

std::unique_ptr<GrFragmentProcessor> GrTextureMaker::createBicubicFragmentProcessor(
        const SkMatrix& textureMatrix,
        const SkRect* subset,
        const SkRect* domain,
        GrSamplerState::WrapMode wrapX,
        GrSamplerState::WrapMode wrapY,
        SkImage::CubicResampler kernel) {
    return this->createBicubicFragmentProcessorForView(
            this->view(GrMipmapped::kNo), textureMatrix, subset, domain, wrapX, wrapY, kernel);
}
