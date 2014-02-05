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
    GrGLProgram* program = SkNEW_ARGS(GrGLProgram, (gpu, desc, colorStages, coverageStages));
    if (!program->succeeded()) {
        delete program;
        program = NULL;
    }
    return program;
}

GrGLProgram::GrGLProgram(GrGpuGL* gpu,
                         const GrGLProgramDesc& desc,
                         const GrEffectStage* colorStages[],
                         const GrEffectStage* coverageStages[])
: fGpu(gpu)
, fUniformManager(gpu)
, fHasVertexShader(false)
, fNumTexCoordSets(0) {
    fDesc = desc;
    fProgramID = 0;

    fDstCopyTexUnit = -1;

    fColor = GrColor_ILLEGAL;

    if (fDesc.getHeader().fHasVertexCode ||
        !fGpu->shouldUseFixedFunctionTexturing()) {
        GrGLFullShaderBuilder fullBuilder(fGpu, fUniformManager, fDesc);
        if (this->genProgram(&fullBuilder, colorStages, coverageStages)) {
            fUniformHandles.fViewMatrixUni = fullBuilder.getViewMatrixUniform();
            fHasVertexShader = true;
        }
    } else {
        GrGLFragmentOnlyShaderBuilder fragmentOnlyBuilder(fGpu, fUniformManager, fDesc);
        if (this->genProgram(&fragmentOnlyBuilder, colorStages, coverageStages)) {
            fNumTexCoordSets = fragmentOnlyBuilder.getNumTexCoordSets();
        }
    }
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
            GrCrash("Unexpected coverage output");
            break;
    }
}

bool GrGLProgram::genProgram(GrGLShaderBuilder* builder,
                             const GrEffectStage* colorStages[],
                             const GrEffectStage* coverageStages[]) {
    SkASSERT(0 == fProgramID);

    const GrGLProgramDesc::KeyHeader& header = fDesc.getHeader();

    // incoming color to current stage being processed.
    GrGLSLExpr4 inColor = builder->getInputColor();

    fColorEffects.reset(
        builder->createAndEmitEffects(colorStages,
                                      fDesc.effectKeys(),
                                      fDesc.numColorEffects(),
                                      &inColor));

    ///////////////////////////////////////////////////////////////////////////
    // compute the partial coverage
    GrGLSLExpr4 inCoverage = builder->getInputCoverage();

    fCoverageEffects.reset(
        builder->createAndEmitEffects(coverageStages,
                                      fDesc.getEffectKeys() + fDesc.numColorEffects(),
                                      fDesc.numCoverageEffects(),
                                      &inCoverage));

    if (GrGLProgramDesc::CoverageOutputUsesSecondaryOutput(header.fCoverageOutput)) {
        const char* secondaryOutputName = builder->enableSecondaryOutput();

        // default coeff to ones for kCoverage_DualSrcOutput
        GrGLSLExpr4 coeff(1);
        if (GrGLProgramDesc::kSecondaryCoverageISA_CoverageOutput == header.fCoverageOutput) {
            // Get (1-A) into coeff
            coeff = GrGLSLExpr4::VectorCast(GrGLSLExpr1(1) - inColor.a());
        } else if (GrGLProgramDesc::kSecondaryCoverageISC_CoverageOutput == header.fCoverageOutput) {
            // Get (1-RGBA) into coeff
            coeff = GrGLSLExpr4(1) - inColor;
        }
        // Get coeff * coverage into modulate and then write that to the dual source output.
        builder->fsCodeAppendf("\t%s = %s;\n", secondaryOutputName, (coeff * inCoverage).c_str());
    }

    ///////////////////////////////////////////////////////////////////////////
    // combine color and coverage as frag color

    // Get "color * coverage" into fragColor
    GrGLSLExpr4 fragColor = inColor * inCoverage;
    // Now tack on "+(1-coverage)dst onto the frag color if we were asked to do so.
    if (GrGLProgramDesc::kCombineWithDst_CoverageOutput == header.fCoverageOutput) {
        GrGLSLExpr4 dstCoeff = GrGLSLExpr4(1) - inCoverage;

        GrGLSLExpr4 dstContribution = dstCoeff * GrGLSLExpr4(builder->dstColor());

        fragColor = fragColor + dstContribution;
    }
    builder->fsCodeAppendf("\t%s = %s;\n", builder->getColorOutputName(), fragColor.c_str());

    if (!builder->finish(&fProgramID)) {
        return false;
    }

    fUniformHandles.fRTHeightUni = builder->getRTHeightUniform();
    fUniformHandles.fDstCopyTopLeftUni = builder->getDstCopyTopLeftUniform();
    fUniformHandles.fDstCopyScaleUni = builder->getDstCopyScaleUniform();
    fUniformHandles.fColorUni = builder->getColorUniform();
    fUniformHandles.fCoverageUni = builder->getCoverageUniform();
    fUniformHandles.fDstCopySamplerUni = builder->getDstCopySamplerUniform();
    // This must be called after we set fDstCopySamplerUni above.
    this->initSamplerUniforms();

    return true;
}

void GrGLProgram::initSamplerUniforms() {
    GL_CALL(UseProgram(fProgramID));
    GrGLint texUnitIdx = 0;
    if (fUniformHandles.fDstCopySamplerUni.isValid()) {
        fUniformManager.setSampler(fUniformHandles.fDstCopySamplerUni, texUnitIdx);
        fDstCopyTexUnit = texUnitIdx++;
    }
    fColorEffects->initSamplers(fUniformManager, &texUnitIdx);
    fCoverageEffects->initSamplers(fUniformManager, &texUnitIdx);
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
        if (fUniformHandles.fDstCopyTopLeftUni.isValid()) {
            fUniformManager.set2f(fUniformHandles.fDstCopyTopLeftUni,
                                  static_cast<GrGLfloat>(dstCopy->offset().fX),
                                  static_cast<GrGLfloat>(dstCopy->offset().fY));
            fUniformManager.set2f(fUniformHandles.fDstCopyScaleUni,
                                  1.f / dstCopy->texture()->width(),
                                  1.f / dstCopy->texture()->height());
            GrGLTexture* texture = static_cast<GrGLTexture*>(dstCopy->texture());
            static GrTextureParams kParams; // the default is clamp, nearest filtering.
            fGpu->bindTexture(fDstCopyTexUnit, kParams, texture);
        } else {
            SkASSERT(!fUniformHandles.fDstCopyScaleUni.isValid());
            SkASSERT(!fUniformHandles.fDstCopySamplerUni.isValid());
        }
    } else {
        SkASSERT(!fUniformHandles.fDstCopyTopLeftUni.isValid());
        SkASSERT(!fUniformHandles.fDstCopyScaleUni.isValid());
        SkASSERT(!fUniformHandles.fDstCopySamplerUni.isValid());
    }

    fColorEffects->setData(fGpu, fUniformManager, colorStages);
    fCoverageEffects->setData(fGpu, fUniformManager, coverageStages);


    // TexGen state applies to the the fixed function vertex shader. For custom shaders, it's
    // ignored, so we don't need to change the texgen settings in that case.
    if (!fHasVertexShader) {
        fGpu->flushTexGenSettings(fNumTexCoordSets);
    }
}

void GrGLProgram::setColor(const GrDrawState& drawState,
                           GrColor color,
                           SharedGLState* sharedState) {
    const GrGLProgramDesc::KeyHeader& header = fDesc.getHeader();
    if (!drawState.hasColorVertexAttribute()) {
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
                if (fColor != color && fUniformHandles.fColorUni.isValid()) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    fUniformManager.set4fv(fUniformHandles.fColorUni, 1, c);
                    fColor = color;
                }
                sharedState->fConstAttribColorIndex = -1;
                break;
            case GrGLProgramDesc::kSolidWhite_ColorInput:
            case GrGLProgramDesc::kTransBlack_ColorInput:
                sharedState->fConstAttribColorIndex = -1;
                break;
            default:
                GrCrash("Unknown color type.");
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
                    fUniformManager.set4fv(fUniformHandles.fCoverageUni, 1, c);
                    fCoverage = coverage;
                }
                sharedState->fConstAttribCoverageIndex = -1;
                break;
            case GrGLProgramDesc::kSolidWhite_ColorInput:
            case GrGLProgramDesc::kTransBlack_ColorInput:
                sharedState->fConstAttribCoverageIndex = -1;
                break;
            default:
                GrCrash("Unknown coverage type.");
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
    if (fUniformHandles.fRTHeightUni.isValid() &&
        fMatrixState.fRenderTargetSize.fHeight != size.fHeight) {
        fUniformManager.set1f(fUniformHandles.fRTHeightUni, SkIntToScalar(size.fHeight));
    }

    if (!fHasVertexShader) {
        SkASSERT(!fUniformHandles.fViewMatrixUni.isValid());
        fGpu->setProjectionMatrix(drawState.getViewMatrix(), size, rt->origin());
    } else if (fMatrixState.fRenderTargetOrigin != rt->origin() ||
               fMatrixState.fRenderTargetSize != size ||
               !fMatrixState.fViewMatrix.cheapEqualTo(drawState.getViewMatrix())) {
        SkASSERT(fUniformHandles.fViewMatrixUni.isValid());

        fMatrixState.fViewMatrix = drawState.getViewMatrix();
        fMatrixState.fRenderTargetSize = size;
        fMatrixState.fRenderTargetOrigin = rt->origin();

        GrGLfloat viewMatrix[3 * 3];
        fMatrixState.getGLMatrix<3>(viewMatrix);
        fUniformManager.setMatrix3f(fUniformHandles.fViewMatrixUni, viewMatrix);
    }
}
