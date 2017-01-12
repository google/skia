/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m, bool bFoo)
    : fCoordTransform1(m, bFoo, texture, GrSamplerParams::FilterMode::kNone_FilterMode)
    , fTextureSampler(texture)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform1);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m, bool bFoo,
                                             GrSamplerParams::FilterMode filterMode)
    : fCoordTransform1(m, bFoo, texture, filterMode)
    , fTextureSampler(texture, filterMode)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform1);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m, bool bFoo,
                                             const GrSamplerParams& params)
    : fCoordTransform1(m, bFoo, texture, params.filterMode())
    , fTextureSampler(texture, params)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform1);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}
