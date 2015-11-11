/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLVertexShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"

GrGLVertexBuilder::GrGLVertexBuilder(GrGLSLProgramBuilder* program)
    : INHERITED(program)
    , fRtAdjustName(nullptr) {
}

void GrGLVertexBuilder::addVarying(const char* name, GrSLPrecision precision, GrGLSLVarying* v) {
    fOutputs.push_back();
    fOutputs.back().setType(v->fType);
    fOutputs.back().setTypeModifier(GrGLSLShaderVar::kVaryingOut_TypeModifier);
    fOutputs.back().setPrecision(precision);
    fProgramBuilder->nameVariable(fOutputs.back().accessName(), 'v', name);
    v->fVsOut = fOutputs.back().getName().c_str();
}

void GrGLVertexBuilder::emitAttributes(const GrGeometryProcessor& gp) {
    int vaCount = gp.numAttribs();
    for (int i = 0; i < vaCount; i++) {
        this->addAttribute(&gp.getAttrib(i));
    }
    return;
}

void GrGLVertexBuilder::transformToNormalizedDeviceSpace(const GrShaderVar& posVar) {
    SkASSERT(!fRtAdjustName);

    GrSLPrecision precision = kDefault_GrSLPrecision;
    if (fProgramBuilder->glslCaps()->forceHighPrecisionNDSTransform()) {
        precision = kHigh_GrSLPrecision;
    }

    // setup RT Uniform
    fProgramBuilder->fUniformHandles.fRTAdjustmentUni =
            fProgramBuilder->addUniform(GrGLSLProgramBuilder::kVertex_Visibility,
                                        kVec4f_GrSLType, precision,
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

bool GrGLVertexBuilder::addAttribute(const GrShaderVar& var) {
    SkASSERT(GrShaderVar::kAttribute_TypeModifier == var.getTypeModifier());
    for (int i = 0; i < fInputs.count(); ++i) {
        const GrGLSLShaderVar& attr = fInputs[i];
        // if attribute already added, don't add it again
        if (attr.getName().equals(var.getName())) {
            return false;
        }
    }
    fInputs.push_back(var);
    return true;
}

