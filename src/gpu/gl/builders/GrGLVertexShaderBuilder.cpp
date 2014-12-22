/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLVertexShaderBuilder.h"
#include "GrGLProgramBuilder.h"
#include "GrGLShaderStringBuilder.h"
#include "../GrGLGpu.h"

#define GL_CALL(X) GR_GL_CALL(fProgramBuilder->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fProgramBuilder->gpu()->glInterface(), R, X)

GrGLVertexBuilder::GrGLVertexBuilder(GrGLProgramBuilder* program)
    : INHERITED(program)
    , fRtAdjustName(NULL) {
}

void GrGLVertexBuilder::addVarying(const char* name, GrGLVarying* v) {
    fOutputs.push_back();
    fOutputs.back().setType(v->fType);
    fOutputs.back().setTypeModifier(GrGLShaderVar::kVaryingOut_TypeModifier);
    fProgramBuilder->nameVariable(fOutputs.back().accessName(), 'v', name);
    v->fVsOut = fOutputs.back().getName().c_str();
}

void GrGLVertexBuilder::setupUniformViewMatrix() {
    fProgramBuilder->fUniformHandles.fViewMatrixUni =
            fProgramBuilder->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                        kMat33f_GrSLType, kDefault_GrSLPrecision,
                                        this->uViewM());
}

void GrGLVertexBuilder::emitAttributes(const GrGeometryProcessor& gp) {
    const GrGeometryProcessor::VertexAttribArray& v = gp.getAttribs();
    int vaCount = v.count();
    for (int i = 0; i < vaCount; i++) {
        this->addAttribute(&v[i]);
    }
    return;
}

void GrGLVertexBuilder::transformToNormalizedDeviceSpace() {
    // setup RT Uniform
    fProgramBuilder->fUniformHandles.fRTAdjustmentUni =
            fProgramBuilder->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                        kVec4f_GrSLType, kDefault_GrSLPrecision,
                                        fProgramBuilder->rtAdjustment(),
                                        &fRtAdjustName);
    // Wire transforms
    SkTArray<GrGLProgramBuilder::TransformVarying, true>& transVs = fProgramBuilder->fCoordVaryings;
    int transformCount = transVs.count();
    for (int i = 0; i < transformCount; i++) {
        GrCoordSet coordSet = transVs[i].fCoordSet;
        const char* coords = NULL;
        switch (coordSet) {
            case kLocal_GrCoordSet:
                coords = this->localCoords();
                break;
            case kDevice_GrCoordSet:
                coords = this->glPosition();
                break;
        }

        // varying = matrix * coords (logically)
        const GrGLVarying& v = transVs[i].fV;
        if (kDevice_GrCoordSet == coordSet) {
            if (kVec2f_GrSLType == v.fType) {
                this->codeAppendf("%s = (%s * %s).xy;", v.fVsOut, transVs[i].fUniName.c_str(),
                                  coords);
            } else {
                this->codeAppendf("%s = %s * %s;", v.fVsOut, transVs[i].fUniName.c_str(), coords);
            }
        } else {
            if (kVec2f_GrSLType == v.fType) {
                this->codeAppendf("%s = (%s * vec3(%s, 1)).xy;", v.fVsOut, transVs[i].fUniName.c_str(),
                                  coords);
            } else {
                this->codeAppendf("%s = %s * vec3(%s, 1);", v.fVsOut, transVs[i].fUniName.c_str(),
                                  coords);
            }
        }
    }

    // Transform from Skia's device coords to GL's normalized device coords.
    this->codeAppendf("gl_Position = vec4(dot(%s.xz, %s.xy), dot(%s.yz, %s.zw), 0, %s.z);",
                      this->glPosition(), fRtAdjustName, this->glPosition(), fRtAdjustName,
                      this->glPosition());
}

void GrGLVertexBuilder::bindVertexAttributes(GrGLuint programID) {
    const GrGeometryProcessor* gp = fProgramBuilder->fOptState.getGeometryProcessor();

    const GrGeometryProcessor::VertexAttribArray& v = gp->getAttribs();
    int vaCount = v.count();
    for (int i = 0; i < vaCount; i++) {
        GL_CALL(BindAttribLocation(programID, i, v[i].fName));
    }
    return;
}

bool GrGLVertexBuilder::compileAndAttachShaders(GrGLuint programId,
        SkTDArray<GrGLuint>* shaderIds) const {
    GrGLGpu* gpu = fProgramBuilder->gpu();
    const GrGLContext& glCtx = gpu->glContext();
    const GrGLContextInfo& ctxInfo = gpu->ctxInfo();
    SkString vertShaderSrc(GrGetGLSLVersionDecl(ctxInfo));
    fProgramBuilder->appendUniformDecls(GrGLProgramBuilder::kVertex_Visibility, &vertShaderSrc);
    this->appendDecls(fInputs, &vertShaderSrc);
    this->appendDecls(fOutputs, &vertShaderSrc);
    vertShaderSrc.append("void main() {");
    vertShaderSrc.append(fCode);
    vertShaderSrc.append("}\n");
    GrGLuint vertShaderId = GrGLCompileAndAttachShader(glCtx, programId,
                                                       GR_GL_VERTEX_SHADER, vertShaderSrc,
                                                       gpu->gpuStats());
    if (!vertShaderId) {
        return false;
    }
    *shaderIds->append() = vertShaderId;
    return true;
}

bool GrGLVertexBuilder::addAttribute(const GrShaderVar& var) {
    SkASSERT(GrShaderVar::kAttribute_TypeModifier == var.getTypeModifier());
    for (int i = 0; i < fInputs.count(); ++i) {
        const GrGLShaderVar& attr = fInputs[i];
        // if attribute already added, don't add it again
        if (attr.getName().equals(var.getName())) {
            return false;
        }
    }
    fInputs.push_back(var);
    return true;
}
