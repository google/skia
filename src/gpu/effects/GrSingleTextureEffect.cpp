/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m)
    : fCoordTransform(m, texture, GrSamplerParams::kNone_FilterMode)
    , fTextureSampler(texture)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             GrSamplerParams::FilterMode filterMode)
    : fCoordTransform(m, texture, filterMode)
    , fTextureSampler(texture, filterMode)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             const GrSamplerParams& params)
    : fCoordTransform(m, texture, params.filterMode())
    , fTextureSampler(texture, params)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}
