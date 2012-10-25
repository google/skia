/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLEffect.h"

GrGLEffect::GrGLEffect(const GrProgramStageFactory& factory)
    : fFactory(factory) {
}

GrGLEffect::~GrGLEffect() {
}

///////////////////////////////////////////////////////////////////////////////

void GrGLEffect::setData(const GrGLUniformManager&, const GrEffect&) {
}

GrGLEffect::StageKey GrGLEffect::GenTextureKey(const GrEffect& effect,
                                                           const GrGLCaps& caps) {
    StageKey key = 0;
    for (int index = 0; index < effect.numTextures(); ++index) {
        const GrTextureAccess& access = effect.textureAccess(index);
        StageKey value = GrGLShaderBuilder::KeyForTextureAccess(access, caps) << index;
        GrAssert(0 == (value & key)); // keys for each access ought not to overlap
        key |= value;
    }
    return key;
}
