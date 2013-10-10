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

SK_DEFINE_INST_COUNT(GrGLProgram)

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
    fColorFilterColor = GrColor_ILLEGAL;

    if (fDesc.getHeader().fHasVertexCode ||
        !fGpu->glCaps().fixedFunctionSupport() ||
        !fGpu->glCaps().pathRenderingSupport()) {

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

namespace {
// given two blend coefficients determine whether the src
// and/or dst computation can be omitted.
inline void need_blend_inputs(SkXfermode::Coeff srcCoeff,
                              SkXfermode::Coeff dstCoeff,
                              bool* needSrcValue,
                              bool* needDstValue) {
    if (SkXfermode::kZero_Coeff == srcCoeff) {
        switch (dstCoeff) {
            // these all read the src
            case SkXfermode::kSC_Coeff:
            case SkXfermode::kISC_Coeff:
            case SkXfermode::kSA_Coeff:
            case SkXfermode::kISA_Coeff:
                *needSrcValue = true;
                break;
            default:
                *needSrcValue = false;
                break;
        }
    } else {
        *needSrcValue = true;
    }
    if (SkXfermode::kZero_Coeff == dstCoeff) {
        switch (srcCoeff) {
            // these all read the dst
            case SkXfermode::kDC_Coeff:
            case SkXfermode::kIDC_Coeff:
            case SkXfermode::kDA_Coeff:
            case SkXfermode::kIDA_Coeff:
                *needDstValue = true;
                break;
            default:
                *needDstValue = false;
                break;
        }
    } else {
        *needDstValue = true;
    }
}

/**
 * Create a blend_coeff * value string to be used in shader code. Sets empty
 * string if result is trivially zero.
 */
inline void blend_term_string(SkString* str, SkXfermode::Coeff coeff,
                       const char* src, const char* dst,
                       const char* value) {
    switch (coeff) {
    case SkXfermode::kZero_Coeff:    /** 0 */
        *str = "";
        break;
    case SkXfermode::kOne_Coeff:     /** 1 */
        *str = value;
        break;
    case SkXfermode::kSC_Coeff:
        str->printf("(%s * %s)", src, value);
        break;
    case SkXfermode::kISC_Coeff:
        str->printf("((vec4(1) - %s) * %s)", src, value);
        break;
    case SkXfermode::kDC_Coeff:
        str->printf("(%s * %s)", dst, value);
        break;
    case SkXfermode::kIDC_Coeff:
        str->printf("((vec4(1) - %s) * %s)", dst, value);
        break;
    case SkXfermode::kSA_Coeff:      /** src alpha */
        str->printf("(%s.a * %s)", src, value);
        break;
    case SkXfermode::kISA_Coeff:     /** inverse src alpha (i.e. 1 - sa) */
        str->printf("((1.0 - %s.a) * %s)", src, value);
        break;
    case SkXfermode::kDA_Coeff:      /** dst alpha */
        str->printf("(%s.a * %s)", dst, value);
        break;
    case SkXfermode::kIDA_Coeff:     /** inverse dst alpha (i.e. 1 - da) */
        str->printf("((1.0 - %s.a) * %s)", dst, value);
        break;
    default:
        GrCrash("Unexpected xfer coeff.");
        break;
    }
}
/**
 * Adds a line to the fragment shader code which modifies the color by
 * the specified color filter.
 */
void add_color_filter(GrGLShaderBuilder* builder,
                      const char * outputVar,
                      SkXfermode::Coeff uniformCoeff,
                      SkXfermode::Coeff colorCoeff,
                      const char* filterColor,
                      const char* inColor) {
    SkString colorStr, constStr;
    blend_term_string(&colorStr, colorCoeff, filterColor, inColor, inColor);
    blend_term_string(&constStr, uniformCoeff, filterColor, inColor, filterColor);
    GrGLSLExpr<4> sum;
    if (colorStr.isEmpty() && constStr.isEmpty()) {
        sum = GrGLSLExpr<4>(0);
    } else if (colorStr.isEmpty()) {
        sum = constStr;
    } else if (constStr.isEmpty()) {
        sum = colorStr;
    } else {
        sum = GrGLSLExpr<4>(colorStr) + GrGLSLExpr<4>(constStr);
    }
    builder->fsCodeAppendf("\t%s = %s;\n", outputVar, sum.c_str());
}
}

bool GrGLProgram::genProgram(GrGLShaderBuilder* builder,
                             const GrEffectStage* colorStages[],
                             const GrEffectStage* coverageStages[]) {
    SkASSERT(0 == fProgramID);

    const GrGLProgramDesc::KeyHeader& header = fDesc.getHeader();

    // incoming color to current stage being processed.
    GrGLSLExpr<4> inColor = builder->getInputColor();

    // Get the coeffs for the Mode-based color filter, determine if color is needed.
    SkXfermode::Coeff colorCoeff;
    SkXfermode::Coeff filterColorCoeff;
    SkAssertResult(
        SkXfermode::ModeAsCoeff(header.fColorFilterXfermode,
                                &filterColorCoeff,
                                &colorCoeff));
    bool needColor, needFilterColor;
    need_blend_inputs(filterColorCoeff, colorCoeff, &needFilterColor, &needColor);

    fColorEffects.reset(
        builder->createAndEmitEffects(colorStages,
                                      fDesc.effectKeys(),
                                      needColor ? fDesc.numColorEffects() : 0,
                                      &inColor));

    // Insert the color filter. This will soon be replaced by a color effect.
    if (SkXfermode::kDst_Mode != header.fColorFilterXfermode) {
        const char* colorFilterColorUniName = NULL;
        fUniformHandles.fColorFilterUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                              kVec4f_GrSLType, "FilterColor",
                                                              &colorFilterColorUniName);

        builder->fsCodeAppend("\tvec4 filteredColor;\n");
        add_color_filter(builder, "filteredColor", filterColorCoeff,
                         colorCoeff, colorFilterColorUniName, inColor.c_str());
        inColor = "filteredColor";
    }

    ///////////////////////////////////////////////////////////////////////////
    // compute the partial coverage
    GrGLSLExpr<4> inCoverage = builder->getInputCoverage();

    fCoverageEffects.reset(
        builder->createAndEmitEffects(coverageStages,
                                      fDesc.getEffectKeys() + fDesc.numColorEffects(),
                                      fDesc.numCoverageEffects(),
                                      &inCoverage));

    // discard if coverage is zero
    if (header.fDiscardIfZeroCoverage && !inCoverage.isOnes()) {
        if (inCoverage.isZeros()) {
            // This is unfortunate.
            builder->fsCodeAppend("\tdiscard;\n");
        } else {
            builder->fsCodeAppendf("\tif (all(lessThanEqual(%s, vec4(0.0)))) {\n\t\tdiscard;\n\t}\n",
                                   inCoverage.c_str());
        }
    }

    if (GrGLProgramDesc::CoverageOutputUsesSecondaryOutput(header.fCoverageOutput)) {
        const char* secondaryOutputName = builder->enableSecondaryOutput();

        // default coeff to ones for kCoverage_DualSrcOutput
        GrGLSLExpr<4> coeff(1);
        if (GrGLProgramDesc::kSecondaryCoverageISA_CoverageOutput == header.fCoverageOutput) {
            // Get (1-A) into coeff
            coeff = GrGLSLExprCast4(GrGLSLExpr<1>(1) - GrGLSLExprExtractAlpha(inColor));
        } else if (GrGLProgramDesc::kSecondaryCoverageISC_CoverageOutput == header.fCoverageOutput) {
            // Get (1-RGBA) into coeff
            coeff = GrGLSLExpr<4>(1) - inColor;
        }
        // Get coeff * coverage into modulate and then write that to the dual source output.
        builder->fsCodeAppendf("\t%s = %s;\n", secondaryOutputName, (coeff * inCoverage).c_str());
    }

    ///////////////////////////////////////////////////////////////////////////
    // combine color and coverage as frag color

    // Get "color * coverage" into fragColor
    GrGLSLExpr<4> fragColor = inColor * inCoverage;
    // Now tack on "+(1-coverage)dst onto the frag color if we were asked to do so.
    if (GrGLProgramDesc::kCombineWithDst_CoverageOutput == header.fCoverageOutput) {
        GrGLSLExpr<4> dstCoeff = GrGLSLExpr<4>(1) - inCoverage;

        GrGLSLExpr<4> dstContribution = dstCoeff * GrGLSLExpr<4>(builder->dstColor());

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
        coverage = drawState.getCoverage();
    } else {
        color = drawState.getColor();
        coverage = drawState.getCoverage();
    }

    this->setColor(drawState, color, sharedState);
    this->setCoverage(drawState, coverage, sharedState);
    this->setMatrixAndRenderTargetHeight(drawState);

    // Setup the SkXfermode::Mode-based colorfilter uniform if necessary
    if (fUniformHandles.fColorFilterUni.isValid() &&
        fColorFilterColor != drawState.getColorFilterColor()) {
        GrGLfloat c[4];
        GrColorToRGBAFloat(drawState.getColorFilterColor(), c);
        fUniformManager.set4fv(fUniformHandles.fColorFilterUni, 0, 1, c);
        fColorFilterColor = drawState.getColorFilterColor();
    }

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

    if (!fHasVertexShader) {
        fGpu->disableUnusedTexGen(fNumTexCoordSets);
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
                if (fColor != color) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    fUniformManager.set4fv(fUniformHandles.fColorUni, 0, 1, c);
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
                    fUniformManager.set4fv(fUniformHandles.fCoverageUni, 0, 1, c);
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
