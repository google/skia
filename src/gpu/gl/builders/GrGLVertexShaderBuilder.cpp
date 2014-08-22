/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLVertexShaderBuilder.h"
#include "GrGLProgramBuilder.h"
#include "GrGLShaderStringBuilder.h"
#include "../GrGpuGL.h"

#define GL_CALL(X) GR_GL_CALL(gpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(gpu->glInterface(), R, X)

namespace {
inline const char* color_attribute_name() { return "aColor"; }
inline const char* coverage_attribute_name() { return "aCoverage"; }
}

GrGLVertexShaderBuilder::GrGLVertexShaderBuilder(GrGLFullProgramBuilder* program)
    : INHERITED(program)
    , fPositionVar(NULL)
    , fLocalCoordsVar(NULL) {
}
bool GrGLVertexShaderBuilder::addAttribute(GrSLType type, const char* name) {
    for (int i = 0; i < fInputs.count(); ++i) {
        const GrGLShaderVar& attr = fInputs[i];
        // if attribute already added, don't add it again
        if (attr.getName().equals(name)) {
            return false;
        }
    }
    fInputs.push_back().set(type, GrGLShaderVar::kAttribute_TypeModifier, name);
    return true;
}

bool GrGLVertexShaderBuilder::addEffectAttribute(int attributeIndex,
                                               GrSLType type,
                                               const SkString& name) {
    if (!this->addAttribute(type, name.c_str())) {
        return false;
    }

    fEffectAttributes.push_back().set(attributeIndex, name);
    return true;
}

void GrGLVertexShaderBuilder::emitAttributes(const GrEffectStage& stage) {
    int numAttributes = stage.getVertexAttribIndexCount();
    const int* attributeIndices = stage.getVertexAttribIndices();
    for (int a = 0; a < numAttributes; ++a) {
        // TODO: Make addAttribute mangle the name.
        SkString attributeName("aAttr");
        attributeName.appendS32(attributeIndices[a]);
        this->addEffectAttribute(attributeIndices[a],
                                 stage.getEffect()->vertexAttribType(a),
                                 attributeName);
    }
}

const SkString* GrGLVertexShaderBuilder::getEffectAttributeName(int attributeIndex) const {
    const AttributePair* attribEnd = fEffectAttributes.end();
    for (const AttributePair* attrib = fEffectAttributes.begin(); attrib != attribEnd; ++attrib) {
        if (attrib->fIndex == attributeIndex) {
            return &attrib->fName;
        }
    }

    return NULL;
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

    const AttributePair* attribEnd = fEffectAttributes.end();
    for (const AttributePair* attrib = fEffectAttributes.begin(); attrib != attribEnd; ++attrib) {
         GL_CALL(BindAttribLocation(programId, attrib->fIndex, attrib->fName.c_str()));
    }
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
    vertShaderSrc.append("void main() {\n");
    vertShaderSrc.append(fCode);
    vertShaderSrc.append("}\n");
    GrGLuint vertShaderId = GrGLCompileAndAttachShader(glCtx, programId,
            GR_GL_VERTEX_SHADER, vertShaderSrc);
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
        "\tgl_Position = vec4(dot(pos3.xz, %s.xy), dot(pos3.yz, %s.zw), 0, pos3.z);\n",
        rtAdjustName, rtAdjustName);
}

void GrGLVertexShaderBuilder::emitCodeBeforeEffects(GrGLSLExpr4* color, GrGLSLExpr4* coverage) {
    const GrGLProgramDesc::KeyHeader& header = fProgramBuilder->desc().getHeader();

    fPositionVar = &fInputs.push_back();
    fPositionVar->set(kVec2f_GrSLType, GrGLShaderVar::kAttribute_TypeModifier, "aPosition");
    if (-1 != header.fLocalCoordAttributeIndex) {
        fLocalCoordsVar = &fInputs.push_back();
        fLocalCoordsVar->set(kVec2f_GrSLType,
                             GrGLShaderVar::kAttribute_TypeModifier,
                             "aLocalCoords");
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
    this->codeAppendf("\tvec3 pos3 = %s * vec3(%s, 1);\n",
                      viewMName, fPositionVar->c_str());

    // we output point size in the GS if present
    if (header.fEmitsPointSize
#if GR_GL_EXPERIMENTAL_GS
        && !header.fExperimentalGS
#endif
        ) {
        this->codeAppend("\tgl_PointSize = 1.0;\n");
    }

    if (GrGLProgramDesc::kAttribute_ColorInput == header.fColorInput) {
        this->addAttribute(kVec4f_GrSLType, color_attribute_name());
        const char *vsName, *fsName;
        fFullProgramBuilder->addVarying(kVec4f_GrSLType, "Color", &vsName, &fsName);
        this->codeAppendf("\t%s = %s;\n", vsName, color_attribute_name());
        *color = fsName;
    }

    if (GrGLProgramDesc::kAttribute_ColorInput == header.fCoverageInput) {
        this->addAttribute(kVec4f_GrSLType, coverage_attribute_name());
        const char *vsName, *fsName;
        fFullProgramBuilder->addVarying(kVec4f_GrSLType, "Coverage", &vsName, &fsName);
        this->codeAppendf("\t%s = %s;\n", vsName, coverage_attribute_name());
        *coverage = fsName;
    }
}
