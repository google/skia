/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLProgramStage.h"

GrGLProgramStage::GrGLProgramStage(const GrProgramStageFactory& factory)
    : fFactory(factory) {
}

GrGLProgramStage::~GrGLProgramStage() {
}

///////////////////////////////////////////////////////////////////////////////

void GrGLProgramStage::setupVariables(GrGLShaderBuilder*) {

}

void GrGLProgramStage::setData(const GrGLUniformManager&,
                               const GrCustomStage&,
                               const GrRenderTarget*,
                               int stageNum) {
}

GrGLProgramStage::StageKey GrGLProgramStage::GenTextureKey(const GrCustomStage& stage,
                                                           const GrGLCaps& caps) {
    StageKey key = 0;
    for (unsigned int index = 0; index < stage.numTextures(); ++index) {
        if (stage.textureAccess(index)) {
            key = (key << index) |
                GrGLShaderBuilder::KeyForTextureAccess(*stage.textureAccess(index), caps);
        }
    }
    return key;
}
