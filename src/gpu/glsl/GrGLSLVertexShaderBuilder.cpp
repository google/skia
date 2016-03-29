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
    : INHERITED(program)
    , fRtAdjustName(nullptr) {
}

void GrGLSLVertexBuilder::transformToNormalizedDeviceSpace(const GrShaderVar& posVar) {
    SkASSERT(!fRtAdjustName);

    // setup RT Uniform
    fProgramBuilder->addRTAdjustmentUniform(kHigh_GrSLPrecision,
                                            fProgramBuilder->rtAdjustment(),
                                            &fRtAdjustName);
    if (this->getProgramBuilder()->desc().header().fSnapVerticesToPixelCenters) {
        if (kVec3f_GrSLType == posVar.getType()) {
            const char* p = posVar.c_str();
            this->codeAppendf("{vec2 _posTmp = vec2(%s.x/%s.z, %s.y/%s.z);", p, p, p, p);
        } else {
            SkASSERT(kVec2f_GrSLType == posVar.getType());
            this->codeAppendf("{vec2 _posTmp = %s;", posVar.c_str());
        }
        this->codeAppendf("_posTmp = floor(_posTmp) + vec2(0.5, 0.5);"
                          "gl_Position = vec4(_posTmp.x * %s.x + %s.y,"
                                             "_posTmp.y * %s.z + %s.w, 0, 1);}",
                          fRtAdjustName, fRtAdjustName, fRtAdjustName, fRtAdjustName);
    } else if (kVec3f_GrSLType == posVar.getType()) {
        this->codeAppendf("gl_Position = vec4(dot(%s.xz, %s.xy), dot(%s.yz, %s.zw), 0, %s.z);",
                          posVar.c_str(), fRtAdjustName,
                          posVar.c_str(), fRtAdjustName,
                          posVar.c_str());
    } else {
        SkASSERT(kVec2f_GrSLType == posVar.getType());
        this->codeAppendf("gl_Position = vec4(%s.x * %s.x + %s.y, %s.y * %s.z + %s.w, 0, 1);",
                          posVar.c_str(), fRtAdjustName, fRtAdjustName,
                          posVar.c_str(), fRtAdjustName, fRtAdjustName);
    }
    // We could have the GrGeometryProcessor do this, but its just easier to have it performed
    // here. If we ever need to set variable pointsize, then we can reinvestigate
    this->codeAppend("gl_PointSize = 1.0;");
}

void GrGLSLVertexBuilder::onFinalize() {
    fProgramBuilder->varyingHandler()->getVertexDecls(&this->inputs(), &this->outputs());
}
