/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgram.h"

#include "GrAllocator.h"
#include "GrEffect.h"
#include "GrCoordTransform.h"
#include "GrDrawEffect.h"
#include "GrGLEffect.h"
#include "GrGpuGL.h"
#include "GrGLShaderVar.h"
#include "GrGLSL.h"
#include "SkXfermode.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

GrGLProgram* GrGLProgram::Create(GrGpuGL* gpu,
                                 const GrGLProgramDesc& desc,
                                 const GrEffectStage* colorStages[],
                                 const GrEffectStage* coverageStages[]) {
    GrGLShaderBuilder::GenProgramOutput output;
    SkAutoTUnref<GrGLUniformManager> uman(SkNEW_ARGS(GrGLUniformManager, (gpu)));
    if (GrGLShaderBuilder::GenProgram(gpu, uman, desc, colorStages, coverageStages,
                                      &output)) {
        SkASSERT(0 != output.fProgramID);
        return SkNEW_ARGS(GrGLProgram, (gpu, desc, uman, output));
    }
    return NULL;
}

GrGLProgram::GrGLProgram(GrGpuGL* gpu,
                         const GrGLProgramDesc& desc,
                         GrGLUniformManager* uman,
                         const GrGLShaderBuilder::GenProgramOutput& builderOutput)
    : fColor(GrColor_ILLEGAL)
    , fCoverage(GrColor_ILLEGAL)
    , fDstCopyTexUnit(-1)
    , fBuilderOutput(builderOutput)
    , fDesc(desc)
    , fGpu(gpu)
    , fUniformManager(SkRef(uman)) {
    this->initSamplerUniforms();
}

GrGLProgram::~GrGLProgram() {
    if (fBuilderOutput.fProgramID) {
        GL_CALL(DeleteProgram(fBuilderOutput.fProgramID));
    }
}

void GrGLProgram::abandon() {
    fBuilderOutput.fProgramID = 0;
}

void GrGLProgram::overrideBlend(GrBlendCoeff* srcCoeff,
                                GrBlendCoeff* dstCoeff) const {
    switch (fDesc.getHeader().fCoverageOutput) {
        case GrGLProgramDesc::kModulate_CoverageOutput:
            break;
        // The prog will write a coverage value to the secondary
        // output and the dst is blended by one minus that value.
        case GrGLProgramDesc::kSecondaryCoverage_CoverageOutput:
        case GrGLProgramDesc::kSecondaryCoverageISA_CoverageOutput:
        case GrGLProgramDesc::kSecondaryCoverageISC_CoverageOutput:
            *dstCoeff = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            break;
        case GrGLProgramDesc::kCombineWithDst_CoverageOutput:
            // We should only have set this if the blend was specified as (1, 0)
            SkASSERT(kOne_GrBlendCoeff == *srcCoeff && kZero_GrBlendCoeff == *dstCoeff);
            break;
        default:
            SkFAIL("Unexpected coverage output");
            break;
    }
}

void GrGLProgram::initSamplerUniforms() {
    GL_CALL(UseProgram(fBuilderOutput.fProgramID));
    GrGLint texUnitIdx = 0;
    if (fBuilderOutput.fUniformHandles.fDstCopySamplerUni.isValid()) {
        fUniformManager->setSampler(fBuilderOutput.fUniformHandles.fDstCopySamplerUni, texUnitIdx);
        fDstCopyTexUnit = texUnitIdx++;
    }
    fBuilderOutput.fColorEffects->initSamplers(*fUniformManager, &texUnitIdx);
    fBuilderOutput.fCoverageEffects->initSamplers(*fUniformManager, &texUnitIdx);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLProgram::setData(GrDrawState::BlendOptFlags blendOpts,
                          const GrEffectStage* colorStages[],
                          const GrEffectStage* coverageStages[],
                          const GrDeviceCoordTexture* dstCopy,
                          SharedGLState* sharedState) {
    const GrDrawState& drawState = fGpu->getDrawState();

    GrColor color;
    GrColor coverage;
    if (blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag) {
        color = 0;
        coverage = 0;
    } else if (blendOpts & GrDrawState::kEmitCoverage_BlendOptFlag) {
        color = 0xffffffff;
        coverage = drawState.getCoverageColor();
    } else {
        color = drawState.getColor();
        coverage = drawState.getCoverageColor();
    }

    this->setColor(drawState, color, sharedState);
    this->setCoverage(drawState, coverage, sharedState);
    this->setMatrixAndRenderTargetHeight(drawState);

    if (NULL != dstCopy) {
        if (fBuilderOutput.fUniformHandles.fDstCopyTopLeftUni.isValid()) {
            fUniformManager->set2f(fBuilderOutput.fUniformHandles.fDstCopyTopLeftUni,
                                   static_cast<GrGLfloat>(dstCopy->offset().fX),
                                   static_cast<GrGLfloat>(dstCopy->offset().fY));
            fUniformManager->set2f(fBuilderOutput.fUniformHandles.fDstCopyScaleUni,
                                   1.f / dstCopy->texture()->width(),
                                   1.f / dstCopy->texture()->height());
            GrGLTexture* texture = static_cast<GrGLTexture*>(dstCopy->texture());
            static GrTextureParams kParams; // the default is clamp, nearest filtering.
            fGpu->bindTexture(fDstCopyTexUnit, kParams, texture);
        } else {
            SkASSERT(!fBuilderOutput.fUniformHandles.fDstCopyScaleUni.isValid());
            SkASSERT(!fBuilderOutput.fUniformHandles.fDstCopySamplerUni.isValid());
        }
    } else {
        SkASSERT(!fBuilderOutput.fUniformHandles.fDstCopyTopLeftUni.isValid());
        SkASSERT(!fBuilderOutput.fUniformHandles.fDstCopyScaleUni.isValid());
        SkASSERT(!fBuilderOutput.fUniformHandles.fDstCopySamplerUni.isValid());
    }

    fBuilderOutput.fColorEffects->setData(fGpu, *fUniformManager, colorStages);
    fBuilderOutput.fCoverageEffects->setData(fGpu, *fUniformManager, coverageStages);


    // PathTexGen state applies to the the fixed function vertex shader. For
    // custom shaders, it's ignored, so we don't need to change the texgen
    // settings in that case.
    if (!fBuilderOutput.fHasVertexShader) {
        fGpu->flushPathTexGenSettings(fBuilderOutput.fTexCoordSetCnt);
    }
}

void GrGLProgram::setColor(const GrDrawState& drawState,
                           GrColor color,
                           SharedGLState* sharedState) {
    const GrGLProgramDesc::KeyHeader& header = fDesc.getHeader();
    if (!drawState.hasColorVertexAttribute() || drawState.canIgnoreColorAttribute()) {
        switch (header.fColorInput) {
            case GrGLProgramDesc::kAttribute_ColorInput:
                SkASSERT(-1 != header.fColorAttributeIndex);
                if (sharedState->fConstAttribColor != color ||
                    sharedState->fConstAttribColorIndex != header.fColorAttributeIndex) {
                    // OpenGL ES only supports the float varieties of glVertexAttrib
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    GL_CALL(VertexAttrib4fv(header.fColorAttributeIndex, c));
                    sharedState->fConstAttribColor = color;
                    sharedState->fConstAttribColorIndex = header.fColorAttributeIndex;
                }
                break;
            case GrGLProgramDesc::kUniform_ColorInput:
                if (fColor != color && fBuilderOutput.fUniformHandles.fColorUni.isValid()) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    fUniformManager->set4fv(fBuilderOutput.fUniformHandles.fColorUni, 1, c);
                    fColor = color;
                }
                sharedState->fConstAttribColorIndex = -1;
                break;
            default:
                SkFAIL("Unexpected color type.");
        }
    } else {
        sharedState->fConstAttribColorIndex = -1;
    }
}

void GrGLProgram::setCoverage(const GrDrawState& drawState,
                              GrColor coverage,
                              SharedGLState* sharedState) {
    const GrGLProgramDesc::KeyHeader& header = fDesc.getHeader();
    if (!drawState.hasCoverageVertexAttribute()) {
        switch (header.fCoverageInput) {
            case GrGLProgramDesc::kAttribute_ColorInput:
                if (sharedState->fConstAttribCoverage != coverage ||
                    sharedState->fConstAttribCoverageIndex != header.fCoverageAttributeIndex) {
                    // OpenGL ES only supports the float varieties of  glVertexAttrib
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(coverage, c);
                    GL_CALL(VertexAttrib4fv(header.fCoverageAttributeIndex, c));
                    sharedState->fConstAttribCoverage = coverage;
                    sharedState->fConstAttribCoverageIndex = header.fCoverageAttributeIndex;
                }
                break;
            case GrGLProgramDesc::kUniform_ColorInput:
                if (fCoverage != coverage) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(coverage, c);
                    fUniformManager->set4fv(fBuilderOutput.fUniformHandles.fCoverageUni, 1, c);
                    fCoverage = coverage;
                }
                sharedState->fConstAttribCoverageIndex = -1;
                break;
            case GrGLProgramDesc::kSolidWhite_ColorInput:
                sharedState->fConstAttribCoverageIndex = -1;
                break;
            default:
                SkFAIL("Unexpected coverage type.");
        }
    } else {
        sharedState->fConstAttribCoverageIndex = -1;
    }
}

void GrGLProgram::setMatrixAndRenderTargetHeight(const GrDrawState& drawState) {
    const GrRenderTarget* rt = drawState.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuilderOutput.fUniformHandles.fRTHeightUni.isValid() &&
        fMatrixState.fRenderTargetSize.fHeight != size.fHeight) {
        fUniformManager->set1f(fBuilderOutput.fUniformHandles.fRTHeightUni,
                               SkIntToScalar(size.fHeight));
    }

    if (!fBuilderOutput.fHasVertexShader) {
        SkASSERT(!fBuilderOutput.fUniformHandles.fViewMatrixUni.isValid());
        SkASSERT(!fBuilderOutput.fUniformHandles.fRTAdjustmentUni.isValid());
        fGpu->setProjectionMatrix(drawState.getViewMatrix(), size, rt->origin());
    } else if (fMatrixState.fRenderTargetOrigin != rt->origin() ||
               fMatrixState.fRenderTargetSize != size ||
               !fMatrixState.fViewMatrix.cheapEqualTo(drawState.getViewMatrix())) {
        SkASSERT(fBuilderOutput.fUniformHandles.fViewMatrixUni.isValid());

        fMatrixState.fViewMatrix = drawState.getViewMatrix();
        fMatrixState.fRenderTargetSize = size;
        fMatrixState.fRenderTargetOrigin = rt->origin();

        GrGLfloat viewMatrix[3 * 3];
        fMatrixState.getGLMatrix<3>(viewMatrix);
        fUniformManager->setMatrix3f(fBuilderOutput.fUniformHandles.fViewMatrixUni, viewMatrix);

        GrGLfloat rtAdjustmentVec[4];
        fMatrixState.getRTAdjustmentVec(rtAdjustmentVec);
        fUniformManager->set4fv(fBuilderOutput.fUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}
