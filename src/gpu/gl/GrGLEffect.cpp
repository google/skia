/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLEffect.h"
#include "GrCoordTransform.h"
#include "GrDrawEffect.h"

GrGLEffect::GrGLEffect(const GrBackendEffectFactory& factory)
    : fFactory(factory) {
}

GrGLEffect::~GrGLEffect() {
}

///////////////////////////////////////////////////////////////////////////////

void GrGLEffect::setData(const GrGLUniformManager&, const GrDrawEffect&) {
}

GrGLEffect::EffectKey GrGLEffect::GenTextureKey(const GrDrawEffect& drawEffect,
                                                const GrGLCaps& caps) {
    EffectKey key = 0;
    int numTextures = (*drawEffect.effect())->numTextures();
    for (int index = 0; index < numTextures; ++index) {
        const GrTextureAccess& access = (*drawEffect.effect())->textureAccess(index);
        EffectKey value = GrGLShaderBuilder::KeyForTextureAccess(access, caps) << index;
        SkASSERT(0 == (value & key)); // keys for each access ought not to overlap
        key |= value;
    }
    return key;
}

GrGLEffect::EffectKey GrGLEffect::GenTransformKey(const GrDrawEffect& drawEffect) {
    EffectKey key = 0;
    int numTransforms = (*drawEffect.effect())->numTransforms();
    for (int index = 0; index < numTransforms; ++index) {
        EffectKey value = GrGLCoordTransform::GenKey(drawEffect, index);
        value <<= index * GrGLCoordTransform::kKeyBits;
        SkASSERT(0 == (value & key)); // keys for each transform ought not to overlap
        key |= value;
    }
    return key;
}

GrGLEffect::EffectKey GrGLEffect::GenAttribKey(const GrDrawEffect& drawEffect) {
    EffectKey key = 0;

    int numAttributes = drawEffect.getVertexAttribIndexCount();
    SkASSERT(numAttributes <= 2);
    const int* attributeIndices = drawEffect.getVertexAttribIndices();
    for (int index = 0; index < numAttributes; ++index) {
        EffectKey value = attributeIndices[index] << 3*index;
        SkASSERT(0 == (value & key)); // keys for each attribute ought not to overlap
        key |= value;
    }

    return key;
}
