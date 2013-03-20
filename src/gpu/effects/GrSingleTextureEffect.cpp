/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             const SkMatrix& m,
                                             CoordsType coordsType)
    : fTextureAccess(texture)
    , fMatrix(m)
    , fCoordsType(coordsType) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             const SkMatrix& m,
                                             bool bilerp,
                                             CoordsType coordsType)
    : fTextureAccess(texture, bilerp)
    , fMatrix(m)
    , fCoordsType(coordsType) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             const SkMatrix& m,
                                             const GrTextureParams& params,
                                             CoordsType coordsType)
    : fTextureAccess(texture, params)
    , fMatrix(m)
    , fCoordsType(coordsType) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}
