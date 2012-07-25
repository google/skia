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

void GrGLProgramStage::setupVariables(GrGLShaderBuilder*, int stage) {

}

void GrGLProgramStage::setData(const GrGLUniformManager&,
                               const GrCustomStage&,
                               const GrRenderTarget*,
                               int stageNum) {
}

