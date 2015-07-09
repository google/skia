/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"

GrSingleTextureEffect::GrSingleTextureEffect(GrProcessorDataManager* procDataManager,
                                             GrTexture* texture,
                                             const SkMatrix& m,
                                             GrCoordSet coordSet)
    : fCoordTransform(coordSet, m, texture, GrTextureParams::kNone_FilterMode)
    , fTextureAccess(texture) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrProcessorDataManager* procDataManager,
                                             GrTexture* texture,
                                             const SkMatrix& m,
                                             GrTextureParams::FilterMode filterMode,
                                             GrCoordSet coordSet)
    : fCoordTransform(coordSet, m, texture, filterMode)
    , fTextureAccess(texture, filterMode) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrProcessorDataManager* procDataManager,
                                             GrTexture* texture,
                                             const SkMatrix& m,
                                             const GrTextureParams& params,
                                             GrCoordSet coordSet)
    : fCoordTransform(coordSet, m, texture, params.filterMode())
    , fTextureAccess(texture, params) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}
