/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLGeometryShaderBuilder.h"
#include "GrGLSLProgramBuilder.h"
#include "GrGLSLVarying.h"

GrGLSLGeometryBuilder::GrGLSLGeometryBuilder(GrGLSLProgramBuilder* program)
    : INHERITED(program) {

}

void GrGLSLGeometryBuilder::onFinalize() {
    fProgramBuilder->varyingHandler()->getGeomDecls(&this->inputs(), &this->outputs());
}

