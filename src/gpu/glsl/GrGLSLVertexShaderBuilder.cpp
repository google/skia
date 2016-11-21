/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLVertexShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"

GrGLSLVertexBuilder::GrGLSLVertexBuilder(GrGLSLProgramBuilder* program)
    : INHERITED(program) {
}

void GrGLSLVertexBuilder::appendTransformToWindowSpace2D(const char* coord2D) {
    const char* rtAdjustName = fProgramBuilder->getRTAdjustmentUniform();
    this->codeAppendf("(%s * %s.xz + %s.yw)", coord2D, rtAdjustName, rtAdjustName);
}

void GrGLSLVertexBuilder::appendTransformToWindowSpace3D(const char* coord3D) {
    const char* rtAdjustName = fProgramBuilder->getRTAdjustmentUniform();
    this->codeAppendf("vec4(dot(%s.xz, %s.xy), dot(%s.yz, %s.zw), 0, %s.z);",
                      coord3D, rtAdjustName, coord3D, rtAdjustName, coord3D);
}

void GrGLSLVertexBuilder::emitVertexPosition(const GrShaderVar& posVar) {
    if (this->getProgramBuilder()->desc().header().fSnapVerticesToPixelCenters) {
        if (kVec3f_GrSLType == posVar.getType()) {
            const char* p = posVar.c_str();
            this->codeAppendf("{vec2 _posTmp = vec2(%s.x/%s.z, %s.y/%s.z);", p, p, p, p);
        } else {
            SkASSERT(kVec2f_GrSLType == posVar.getType());
            this->codeAppendf("{vec2 _posTmp = %s;", posVar.c_str());
        }
        this->codeAppend("_posTmp = floor(_posTmp) + vec2(0.5, 0.5);"
                         "gl_Position = vec4(");
        this->appendTransformToWindowSpace2D("_posTmp");
        this->codeAppend(", 0, 1);}");
    } else if (kVec3f_GrSLType == posVar.getType()) {
        this->codeAppend("gl_Position = ");
        this->appendTransformToWindowSpace3D(posVar.c_str());
        this->codeAppend(";");
    } else {
        SkASSERT(kVec2f_GrSLType == posVar.getType());
        this->codeAppend("gl_Position = vec4(");
        this->appendTransformToWindowSpace2D(posVar.c_str());
        this->codeAppend(", 0, 1);");
    }
    // We could have the GrGeometryProcessor do this, but its just easier to have it performed
    // here. If we ever need to set variable pointsize, then we can reinvestigate.
    if (this->getProgramBuilder()->desc().header().fHasPointSize) {
        this->codeAppend("gl_PointSize = 1.0;");
    }
}

void GrGLSLVertexBuilder::onFinalize() {
    fProgramBuilder->varyingHandler()->getVertexDecls(&this->inputs(), &this->outputs());
}
