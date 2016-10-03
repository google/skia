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
    : fCoordTransform(m, texture, GrTextureParams::kNone_FilterMode)
    , fTextureAccess(texture)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             GrTextureParams::FilterMode filterMode)
    : fCoordTransform(m, texture, filterMode)
    , fTextureAccess(texture, filterMode)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             const GrTextureParams& params)
    : fCoordTransform(m, texture, params.filterMode())
    , fTextureAccess(texture, params)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}
