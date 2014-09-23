/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLVertexShaderBuilder.h"
#include "GrGLFullProgramBuilder.h"
#include "GrGLShaderStringBuilder.h"
#include "../GrGpuGL.h"
#include "../../GrOptDrawState.h"

#define GL_CALL(X) GR_GL_CALL(gpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(gpu->glInterface(), R, X)

namespace {
inline const char* color_attribute_name() { return "inColor"; }
inline const char* coverage_attribute_name() { return "inCoverage"; }
}

GrGLVertexShaderBuilder::GrGLVertexShaderBuilder(GrGLFullProgramBuilder* program)
    : INHERITED(program)
    , fPositionVar(NULL)
    , fLocalCoordsVar(NULL) {
}
bool GrGLVertexShaderBuilder::addAttribute(const GrShaderVar& var) {
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

void GrGLVertexShaderBuilder::emitAttributes(const GrGeometryProcessor& gp) {
    const GrGeometryProcessor::VertexAttribArray& vars = gp.getVertexAttribs();
    int numAttributes = vars.count();
    for (int a = 0; a < numAttributes; ++a) {
        this->addAttribute(vars[a]);
    }
}

void GrGLVertexShaderBuilder::addVarying(GrSLType type, const char* name, const char** vsOutName) {
    fOutputs.push_back();
    fOutputs.back().setType(type);
    fOutputs.back().setTypeModifier(GrGLShaderVar::kVaryingOut_TypeModifier);
    fProgramBuilder->nameVariable(fOutputs.back().accessName(), 'v', name);

    if (vsOutName) {
        *vsOutName = fOutputs.back().getName().c_str();
    }
}


void GrGLVertexShaderBuilder::bindProgramLocations(GrGLuint programId) {
    const GrGLProgramDesc::KeyHeader& header = fProgramBuilder->desc().getHeader();
    GrGpuGL* gpu = fProgramBuilder->gpu();

    // Bind the attrib locations to same values for all shaders
    SkASSERT(-1 != header.fPositionAttributeIndex);
    GL_CALL(BindAttribLocation(programId,
                               header.fPositionAttributeIndex,
                               fPositionVar->c_str()));
    if (-1 != header.fLocalCoordAttributeIndex) {
        GL_CALL(BindAttribLocation(programId,
                                   header.fLocalCoordAttributeIndex,
                                   fLocalCoordsVar->c_str()));
    }
    if (-1 != header.fColorAttributeIndex) {
        GL_CALL(BindAttribLocation(programId,
                                   header.fColorAttributeIndex,
                                   color_attribute_name()));
    }
    if (-1 != header.fCoverageAttributeIndex) {
        GL_CALL(BindAttribLocation(programId,
                                   header.fCoverageAttributeIndex,
                                   coverage_attribute_name()));
    }

    // We pull the current state of attributes off of drawstate's optimized state and bind them in
    // order. This assumes that the drawState has not changed since we called flushGraphicsState()
    // higher up in the stack.
    const GrDrawTargetCaps* caps = fProgramBuilder->gpu()->caps();
    const GrDrawState& drawState = *fProgramBuilder->gpu()->drawState();
    SkAutoTUnref<GrOptDrawState> optState(drawState.createOptState(*caps));
    const GrVertexAttrib* vaPtr = optState->getVertexAttribs();
    const int vaCount = optState->getVertexAttribCount();

    int i = fEffectAttribOffset;
    for (int index = 0; index < vaCount; index++) {
        if (kGeometryProcessor_GrVertexAttribBinding != vaPtr[index].fBinding) {
            continue;
        }
        SkASSERT(index != header.fPositionAttributeIndex &&
                 index != header.fLocalCoordAttributeIndex &&
                 index != header.fColorAttributeIndex &&
                 index != header.fCoverageAttributeIndex);
        // We should never find another effect attribute if we have bound everything
        SkASSERT(i < fInputs.count());
        GL_CALL(BindAttribLocation(programId, index, fInputs[i].c_str()));
        i++;
    }
    // Make sure we bound everything
    SkASSERT(fInputs.count() == i);
}

bool GrGLVertexShaderBuilder::compileAndAttachShaders(GrGLuint programId,
        SkTDArray<GrGLuint>* shaderIds) const {
    GrGpuGL* gpu = fProgramBuilder->gpu();
    const GrGLContext& glCtx = gpu->glContext();
    const GrGLContextInfo& ctxInfo = gpu->ctxInfo();
    SkString vertShaderSrc(GrGetGLSLVersionDecl(ctxInfo));
    fProgramBuilder->appendUniformDecls(GrGLProgramBuilder::kVertex_Visibility, &vertShaderSrc);
    fProgramBuilder->appendDecls(fInputs, &vertShaderSrc);
    fProgramBuilder->appendDecls(fOutputs, &vertShaderSrc);
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

void GrGLVertexShaderBuilder::emitCodeAfterEffects() {
    const char* rtAdjustName;
    fProgramBuilder->fUniformHandles.fRTAdjustmentUni =
        fProgramBuilder->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                             kVec4f_GrSLType,
                             "rtAdjustment",
                             &rtAdjustName);

    // Transform from Skia's device coords to GL's normalized device coords.
    this->codeAppendf(
        "gl_Position = vec4(dot(pos3.xz, %s.xy), dot(pos3.yz, %s.zw), 0, pos3.z);",
        rtAdjustName, rtAdjustName);
}

void GrGLVertexShaderBuilder::emitCodeBeforeEffects(GrGLSLExpr4* color, GrGLSLExpr4* coverage) {
    const GrGLProgramDesc::KeyHeader& header = fProgramBuilder->desc().getHeader();

    fPositionVar = &fInputs.push_back();
    fPositionVar->set(kVec2f_GrSLType, GrGLShaderVar::kAttribute_TypeModifier, "inPosition");
    if (-1 != header.fLocalCoordAttributeIndex) {
        fLocalCoordsVar = &fInputs.push_back();
        fLocalCoordsVar->set(kVec2f_GrSLType,
                             GrGLShaderVar::kAttribute_TypeModifier,
                             "inLocalCoords");
    } else {
        fLocalCoordsVar = fPositionVar;
    }

    const char* viewMName;
    fProgramBuilder->fUniformHandles.fViewMatrixUni =
            fProgramBuilder->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                 kMat33f_GrSLType,
                                 "ViewM",
                                 &viewMName);

    // Transform the position into Skia's device coords.
    this->codeAppendf("vec3 pos3 = %s * vec3(%s, 1);",
                      viewMName, fPositionVar->c_str());

    // we output point size in the GS if present
    if (header.fEmitsPointSize
#if GR_GL_EXPERIMENTAL_GS
        && !header.fExperimentalGS
#endif
        ) {
        this->codeAppend("gl_PointSize = 1.0;");
    }

    if (GrGLProgramDesc::kAttribute_ColorInput == header.fColorInput) {
        this->addAttribute(GrShaderVar(color_attribute_name(),
                                       kVec4f_GrSLType,
                                       GrShaderVar::kAttribute_TypeModifier));
        const char *vsName, *fsName;
        fFullProgramBuilder->addVarying(kVec4f_GrSLType, "Color", &vsName, &fsName);
        this->codeAppendf("%s = %s;", vsName, color_attribute_name());
        *color = fsName;
    }

    if (GrGLProgramDesc::kAttribute_ColorInput == header.fCoverageInput) {
        this->addAttribute(GrShaderVar(coverage_attribute_name(),
                                       kVec4f_GrSLType,
                                       GrShaderVar::kAttribute_TypeModifier));
        const char *vsName, *fsName;
        fFullProgramBuilder->addVarying(kVec4f_GrSLType, "Coverage", &vsName, &fsName);
        this->codeAppendf("%s = %s;", vsName, coverage_attribute_name());
        *coverage = fsName;
    }
    fEffectAttribOffset = fInputs.count();
}
