/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrXferProcessor.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"

GrXferProcessor::GrXferProcessor(ClassID classID)
        : INHERITED(classID)
        , fWillReadDstColor(false)
        , fIsLCD(false) {}

GrXferProcessor::GrXferProcessor(ClassID classID, bool willReadDstColor,
                                 GrProcessorAnalysisCoverage coverage)
        : INHERITED(classID)
        , fWillReadDstColor(willReadDstColor)
        , fIsLCD(GrProcessorAnalysisCoverage::kLCD == coverage) {}

bool GrXferProcessor::hasSecondaryOutput() const {
    if (!this->willReadDstColor()) {
        return this->onHasSecondaryOutput();
    }
    return false;
}

void GrXferProcessor::addToKey(const GrShaderCaps& caps,
                               skgpu::KeyBuilder* b,
                               const GrSurfaceOrigin* originIfDstTexture,
                               bool usesInputAttachmentForDstRead) const {
    uint32_t key = this->willReadDstColor() ? 0x1 : 0x0;
    if (key) {
        if (originIfDstTexture) {
            key |= 0x2;
            if (kTopLeft_GrSurfaceOrigin == *originIfDstTexture) {
                key |= 0x4;
            }
            if (usesInputAttachmentForDstRead) {
                key |= 0x8;
            }
        }
    }
    if (fIsLCD) {
        key |= 0x10;
    }
    b->add32(key);
    this->onAddToKey(caps, b);
}

#ifdef SK_DEBUG
static const char* equation_string(GrBlendEquation eq) {
    switch (eq) {
        case kAdd_GrBlendEquation:
            return "add";
        case kSubtract_GrBlendEquation:
            return "subtract";
        case kReverseSubtract_GrBlendEquation:
            return "reverse_subtract";
        case kScreen_GrBlendEquation:
            return "screen";
        case kOverlay_GrBlendEquation:
            return "overlay";
        case kDarken_GrBlendEquation:
            return "darken";
        case kLighten_GrBlendEquation:
            return "lighten";
        case kColorDodge_GrBlendEquation:
            return "color_dodge";
        case kColorBurn_GrBlendEquation:
            return "color_burn";
        case kHardLight_GrBlendEquation:
            return "hard_light";
        case kSoftLight_GrBlendEquation:
            return "soft_light";
        case kDifference_GrBlendEquation:
            return "difference";
        case kExclusion_GrBlendEquation:
            return "exclusion";
        case kMultiply_GrBlendEquation:
            return "multiply";
        case kHSLHue_GrBlendEquation:
            return "hsl_hue";
        case kHSLSaturation_GrBlendEquation:
            return "hsl_saturation";
        case kHSLColor_GrBlendEquation:
            return "hsl_color";
        case kHSLLuminosity_GrBlendEquation:
            return "hsl_luminosity";
        case kIllegal_GrBlendEquation:
            SkASSERT(false);
            return "<illegal>";
    }
    return "";
}

static const char* coeff_string(GrBlendCoeff coeff) {
    switch (coeff) {
        case kZero_GrBlendCoeff:
            return "zero";
        case kOne_GrBlendCoeff:
            return "one";
        case kSC_GrBlendCoeff:
            return "src_color";
        case kISC_GrBlendCoeff:
            return "inv_src_color";
        case kDC_GrBlendCoeff:
            return "dst_color";
        case kIDC_GrBlendCoeff:
            return "inv_dst_color";
        case kSA_GrBlendCoeff:
            return "src_alpha";
        case kISA_GrBlendCoeff:
            return "inv_src_alpha";
        case kDA_GrBlendCoeff:
            return "dst_alpha";
        case kIDA_GrBlendCoeff:
            return "inv_dst_alpha";
        case kConstC_GrBlendCoeff:
            return "const_color";
        case kIConstC_GrBlendCoeff:
            return "inv_const_color";
        case kS2C_GrBlendCoeff:
            return "src2_color";
        case kIS2C_GrBlendCoeff:
            return "inv_src2_color";
        case kS2A_GrBlendCoeff:
            return "src2_alpha";
        case kIS2A_GrBlendCoeff:
            return "inv_src2_alpha";
        case kIllegal_GrBlendCoeff:
            SkASSERT(false);
            return "<illegal>";
    }
    return "";
}

SkString GrXferProcessor::BlendInfo::dump() const {
    SkString out;
    out.printf("write_color(%d) equation(%s) src_coeff(%s) dst_coeff:(%s) const(0x%08x)",
               fWriteColor, equation_string(fEquation), coeff_string(fSrcBlend),
               coeff_string(fDstBlend), fBlendConstant.toBytes_RGBA());
    return out;
}
#endif

///////////////////////////////////////////////////////////////////////////////

GrXPFactory::AnalysisProperties GrXPFactory::GetAnalysisProperties(
        const GrXPFactory* factory,
        const GrProcessorAnalysisColor& color,
        const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps,
        GrClampType clampType) {
    AnalysisProperties result;
    if (factory) {
        result = factory->analysisProperties(color, coverage, caps, clampType);
    } else {
        result = GrPorterDuffXPFactory::SrcOverAnalysisProperties(color, coverage, caps, clampType);
    }
    if (coverage == GrProcessorAnalysisCoverage::kNone) {
        result |= AnalysisProperties::kCompatibleWithCoverageAsAlpha;
    }
    SkASSERT(!(result & AnalysisProperties::kRequiresDstTexture));
    if ((result & AnalysisProperties::kReadsDstInShader) &&
        !caps.shaderCaps()->dstReadInShaderSupport()) {
        result |= AnalysisProperties::kRequiresDstTexture |
                  AnalysisProperties::kRequiresNonOverlappingDraws;
    }
    return result;
}

sk_sp<const GrXferProcessor> GrXPFactory::MakeXferProcessor(const GrXPFactory* factory,
                                                            const GrProcessorAnalysisColor& color,
                                                            GrProcessorAnalysisCoverage coverage,
                                                            const GrCaps& caps,
                                                            GrClampType clampType) {
    if (factory) {
        return factory->makeXferProcessor(color, coverage, caps, clampType);
    } else {
        return GrPorterDuffXPFactory::MakeSrcOverXferProcessor(color, coverage, caps);
    }
}

//////////////////////////////////////////////////////////////////////////////

using ProgramImpl = GrXferProcessor::ProgramImpl;

// This is only called for cases where we are doing LCD coverage and not using in shader blending.
// For these cases we assume the the src alpha is 1, thus we can just use the max for the alpha
// coverage since src alpha will always be greater than or equal to dst alpha.
static void adjust_for_lcd_coverage(GrGLSLXPFragmentBuilder* fragBuilder,
                                    const char* srcCoverage,
                                    const GrXferProcessor& proc) {
    if (srcCoverage && proc.isLCD()) {
        fragBuilder->codeAppendf("%s.a = max(max(%s.r, %s.g), %s.b);",
                                 srcCoverage,
                                 srcCoverage,
                                 srcCoverage,
                                 srcCoverage);
    }
}

void ProgramImpl::emitCode(const EmitArgs& args) {
    if (!args.fXP.willReadDstColor()) {
        adjust_for_lcd_coverage(args.fXPFragBuilder, args.fInputCoverage, args.fXP);
        this->emitOutputsForBlendState(args);
    } else {
        GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        const char* dstColor = fragBuilder->dstColor();

        bool needsLocalOutColor = false;

        if (args.fDstTextureSamplerHandle.isValid()) {
            if (args.fInputCoverage) {
                // We don't think any shaders actually output negative coverage, but just as a
                // safety check for floating point precision errors, we compare with <= here. We
                // just check the RGB values of the coverage, since the alpha may not have been set
                // when using LCD. If we are using single-channel coverage, alpha will be equal to
                // RGB anyway.
                //
                // The discard here also helps for batching text-draws together, which need to read
                // from a dst copy for blends. However, this only helps the case where the outer
                // bounding boxes of each letter overlap and not two actually parts of the text.
                fragBuilder->codeAppendf("if (all(lessThanEqual(%s.rgb, half3(0)))) {"
                                         "    discard;"
                                         "}",
                                         args.fInputCoverage);
            }
        } else {
            needsLocalOutColor = args.fShaderCaps->requiresLocalOutputColorForFBFetch();
        }

        const char* outColor = "_localColorOut";
        if (!needsLocalOutColor) {
            outColor = args.fOutputPrimary;
        } else {
            fragBuilder->codeAppendf("half4 %s;", outColor);
        }

        this->emitBlendCodeForDstRead(fragBuilder,
                                      uniformHandler,
                                      args.fInputColor,
                                      args.fInputCoverage,
                                      dstColor,
                                      outColor,
                                      args.fOutputSecondary,
                                      args.fXP);
        if (needsLocalOutColor) {
            fragBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, outColor);
        }
    }

    // Swizzle the fragment shader outputs if necessary.
    this->emitWriteSwizzle(args.fXPFragBuilder,
                           args.fWriteSwizzle,
                           args.fOutputPrimary,
                           args.fOutputSecondary);
}

void ProgramImpl::emitWriteSwizzle(GrGLSLXPFragmentBuilder* x,
                                   const GrSwizzle& swizzle,
                                   const char* outColor,
                                   const char* outColorSecondary) const {
    if (GrSwizzle::RGBA() != swizzle) {
        x->codeAppendf("%s = %s.%s;", outColor, outColor, swizzle.asString().c_str());
        if (outColorSecondary) {
            x->codeAppendf("%s = %s.%s;",
                           outColorSecondary,
                           outColorSecondary,
                           swizzle.asString().c_str());
        }
    }
}

void ProgramImpl::setData(const GrGLSLProgramDataManager& pdm, const GrXferProcessor& xp) {
    this->onSetData(pdm, xp);
}

void ProgramImpl::DefaultCoverageModulation(GrGLSLXPFragmentBuilder* fragBuilder,
                                            const char* srcCoverage,
                                            const char* dstColor,
                                            const char* outColor,
                                            const char* outColorSecondary,
                                            const GrXferProcessor& proc) {
    if (srcCoverage) {
        if (proc.isLCD()) {
            fragBuilder->codeAppendf("half3 lerpRGB = mix(%s.aaa, %s.aaa, %s.rgb);",
                                     dstColor,
                                     outColor,
                                     srcCoverage);
        }
        fragBuilder->codeAppendf("%s = %s * %s + (half4(1.0) - %s) * %s;",
                                 outColor,
                                 srcCoverage,
                                 outColor,
                                 srcCoverage,
                                 dstColor);
        if (proc.isLCD()) {
            fragBuilder->codeAppendf("%s.a = max(max(lerpRGB.r, lerpRGB.b), lerpRGB.g);", outColor);
        }
    }
}
