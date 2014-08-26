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
#include "GrGLPathRendering.h"
#include "GrGLShaderVar.h"
#include "GrGLSL.h"
#include "SkXfermode.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

GrGLProgram* GrGLProgram::Create(GrGpuGL* gpu,
                                 const GrGLProgramDesc& desc,
                                 const GrEffectStage* colorStages[],
                                 const GrEffectStage* coverageStages[]) {
    SkAutoTDelete<GrGLProgramBuilder> builder;
    if (!desc.getHeader().fRequiresVertexShader &&
        gpu->glCaps().pathRenderingSupport() &&
        gpu->glPathRendering()->texturingMode() == GrGLPathRendering::FixedFunction_TexturingMode) {
        builder.reset(SkNEW_ARGS(GrGLFragmentOnlyProgramBuilder, (gpu, desc)));
    } else {
        builder.reset(SkNEW_ARGS(GrGLFullProgramBuilder, (gpu, desc)));
    }
    if (builder->genProgram(colorStages, coverageStages)) {
        SkASSERT(0 != builder->getProgramID());
        return SkNEW_ARGS(GrGLProgram, (gpu, desc, *builder));
    }
    return NULL;
}

GrGLProgram::GrGLProgram(GrGpuGL* gpu,
                         const GrGLProgramDesc& desc,
                         const GrGLProgramBuilder& builder)
    : fColor(GrColor_ILLEGAL)
    , fCoverage(GrColor_ILLEGAL)
    , fDstCopyTexUnit(-1)
    , fBuiltinUniformHandles(builder.getBuiltinUniformHandles())
    , fColorEffects(SkRef(builder.getColorEffects()))
    , fCoverageEffects(SkRef(builder.getCoverageEffects()))
    , fProgramID(builder.getProgramID())
    , fHasVertexShader(builder.hasVertexShader())
    , fTexCoordSetCnt(builder.getTexCoordSetCount())
    , fDesc(desc)
    , fGpu(gpu)
    , fProgramDataManager(gpu, this, builder) {
    this->initSamplerUniforms();
}

GrGLProgram::~GrGLProgram() {
    if (fProgramID) {
        GL_CALL(DeleteProgram(fProgramID));
    }
}

void GrGLProgram::abandon() {
    fProgramID = 0;
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
    GL_CALL(UseProgram(fProgramID));
    GrGLint texUnitIdx = 0;
    if (fBuiltinUniformHandles.fDstCopySamplerUni.isValid()) {
        fProgramDataManager.setSampler(fBuiltinUniformHandles.fDstCopySamplerUni, texUnitIdx);
        fDstCopyTexUnit = texUnitIdx++;
    }
    fColorEffects->initSamplers(fProgramDataManager, &texUnitIdx);
    fCoverageEffects->initSamplers(fProgramDataManager, &texUnitIdx);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLProgram::setData(GrGpu::DrawType drawType,
                          GrDrawState::BlendOptFlags blendOpts,
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
    this->setMatrixAndRenderTargetHeight(drawType, drawState);

    if (NULL != dstCopy) {
        if (fBuiltinUniformHandles.fDstCopyTopLeftUni.isValid()) {
            fProgramDataManager.set2f(fBuiltinUniformHandles.fDstCopyTopLeftUni,
                                       static_cast<GrGLfloat>(dstCopy->offset().fX),
                                       static_cast<GrGLfloat>(dstCopy->offset().fY));
            fProgramDataManager.set2f(fBuiltinUniformHandles.fDstCopyScaleUni,
                                       1.f / dstCopy->texture()->width(),
                                       1.f / dstCopy->texture()->height());
            GrGLTexture* texture = static_cast<GrGLTexture*>(dstCopy->texture());
            static GrTextureParams kParams; // the default is clamp, nearest filtering.
            fGpu->bindTexture(fDstCopyTexUnit, kParams, texture);
        } else {
            SkASSERT(!fBuiltinUniformHandles.fDstCopyScaleUni.isValid());
            SkASSERT(!fBuiltinUniformHandles.fDstCopySamplerUni.isValid());
        }
    } else {
        SkASSERT(!fBuiltinUniformHandles.fDstCopyTopLeftUni.isValid());
        SkASSERT(!fBuiltinUniformHandles.fDstCopyScaleUni.isValid());
        SkASSERT(!fBuiltinUniformHandles.fDstCopySamplerUni.isValid());
    }

    fColorEffects->setData(fGpu, drawType,fProgramDataManager, colorStages);
    fCoverageEffects->setData(fGpu, drawType,fProgramDataManager, coverageStages);

    // PathTexGen state applies to the the fixed function vertex shader. For
    // custom shaders, it's ignored, so we don't need to change the texgen
    // settings in that case.
    if (!fHasVertexShader) {
        fGpu->glPathRendering()->flushPathTexGenSettings(fTexCoordSetCnt);
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
                if (fColor != color && fBuiltinUniformHandles.fColorUni.isValid()) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    fProgramDataManager.set4fv(fBuiltinUniformHandles.fColorUni, 1, c);
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
                    fProgramDataManager.set4fv(fBuiltinUniformHandles.fCoverageUni, 1, c);
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

void GrGLProgram::setMatrixAndRenderTargetHeight(GrGpu::DrawType drawType,
                                                 const GrDrawState& drawState) {
    const GrRenderTarget* rt = drawState.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fMatrixState.fRenderTargetSize.fHeight != size.fHeight) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni,
                                   SkIntToScalar(size.fHeight));
    }

    if (GrGpu::IsPathRenderingDrawType(drawType)) {
        fGpu->glPathRendering()->setProjectionMatrix(drawState.getViewMatrix(), size, rt->origin());
    } else if (fMatrixState.fRenderTargetOrigin != rt->origin() ||
               fMatrixState.fRenderTargetSize != size ||
               !fMatrixState.fViewMatrix.cheapEqualTo(drawState.getViewMatrix())) {
        SkASSERT(fBuiltinUniformHandles.fViewMatrixUni.isValid());

        fMatrixState.fViewMatrix = drawState.getViewMatrix();
        fMatrixState.fRenderTargetSize = size;
        fMatrixState.fRenderTargetOrigin = rt->origin();

        GrGLfloat viewMatrix[3 * 3];
        fMatrixState.getGLMatrix<3>(viewMatrix);
        fProgramDataManager.setMatrix3f(fBuiltinUniformHandles.fViewMatrixUni, viewMatrix);

        GrGLfloat rtAdjustmentVec[4];
        fMatrixState.getRTAdjustmentVec(rtAdjustmentVec);
        fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}
