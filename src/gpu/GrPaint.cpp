
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPaint.h"

#include "effects/GrSimpleTextureEffect.h"

void GrPaint::addColorTextureEffect(GrTexture* texture, const SkMatrix& matrix) {
    GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix);
    this->addColorEffect(effect)->unref();
}

void GrPaint::addCoverageTextureEffect(GrTexture* texture, const SkMatrix& matrix) {
    GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix);
    this->addCoverageEffect(effect)->unref();
}

void GrPaint::addColorTextureEffect(GrTexture* texture,
                                    const SkMatrix& matrix,
                                    const GrTextureParams& params) {
    GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix, params);
    this->addColorEffect(effect)->unref();
}

void GrPaint::addCoverageTextureEffect(GrTexture* texture,
                                       const SkMatrix& matrix,
                                       const GrTextureParams& params) {
    GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix, params);
    this->addCoverageEffect(effect)->unref();
}
