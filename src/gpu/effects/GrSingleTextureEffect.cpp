/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture, const SkMatrix& m)
    : fTextureAccess(texture)
    , fMatrix(m) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture, const SkMatrix& m, bool bilerp)
    : fTextureAccess(texture, bilerp)
    , fMatrix(m) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             const SkMatrix& m,
                                             const GrTextureParams& params)
    : fTextureAccess(texture, params)
    , fMatrix(m) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}
