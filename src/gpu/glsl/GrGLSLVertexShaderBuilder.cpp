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

void GrGLSLVertexBuilder::transformToNormalizedDeviceSpace(const GrShaderVar& posVar,
                                                           const char* rtAdjustName) {
    // setup RT Uniform
    if (this->getProgramBuilder()->desc()->header().fSnapVerticesToPixelCenters) {
        if (kFloat3_GrSLType == posVar.getType()) {
            const char* p = posVar.c_str();
            this->codeAppendf("{float2 _posTmp = float2(%s.x/%s.z, %s.y/%s.z);", p, p, p, p);
        } else {
            SkASSERT(kFloat2_GrSLType == posVar.getType());
            this->codeAppendf("{float2 _posTmp = %s;", posVar.c_str());
        }
        this->codeAppendf("_posTmp = floor(_posTmp) + half2(0.5, 0.5);"
                          "gl_Position = float4(_posTmp.x * %s.x + %s.y,"
                                               "_posTmp.y * %s.z + %s.w, 0, 1);}",
                          rtAdjustName, rtAdjustName, rtAdjustName, rtAdjustName);
    } else if (kFloat3_GrSLType == posVar.getType()) {
        this->codeAppendf("gl_Position = float4(dot(%s.xz, %s.xy), dot(%s.yz, %s.zw), 0, %s.z);",
                          posVar.c_str(), rtAdjustName,
                          posVar.c_str(), rtAdjustName,
                          posVar.c_str());
    } else {
        SkASSERT(kFloat2_GrSLType == posVar.getType());
        this->codeAppendf("gl_Position = float4(%s.x * %s.x + %s.y, %s.y * %s.z + %s.w, 0, 1);",
                          posVar.c_str(), rtAdjustName, rtAdjustName,
                          posVar.c_str(), rtAdjustName, rtAdjustName);
    }
    // We could have the GrGeometryProcessor do this, but its just easier to have it performed
    // here. If we ever need to set variable pointsize, then we can reinvestigate.
    if (this->getProgramBuilder()->desc()->header().fHasPointSize) {
        this->codeAppend("gl_PointSize = 1.0;");
    }
}

void GrGLSLVertexBuilder::onFinalize() {
    fProgramBuilder->varyingHandler()->getVertexDecls(&this->inputs(), &this->outputs());
}
