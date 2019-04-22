/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrXferProcessor.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrPipeline.h"

GrXferProcessor::GrXferProcessor(ClassID classID)
        : INHERITED(classID)
        , fWillReadDstColor(false)
        , fDstReadUsesMixedSamples(false)
        , fIsLCD(false) {}

GrXferProcessor::GrXferProcessor(ClassID classID, bool willReadDstColor, bool hasMixedSamples,
                                 GrProcessorAnalysisCoverage coverage)
        : INHERITED(classID)
        , fWillReadDstColor(willReadDstColor)
        , fDstReadUsesMixedSamples(willReadDstColor && hasMixedSamples)
        , fIsLCD(GrProcessorAnalysisCoverage::kLCD == coverage) {}

bool GrXferProcessor::hasSecondaryOutput() const {
    if (!this->willReadDstColor()) {
        return this->onHasSecondaryOutput();
    }
    return this->dstReadUsesMixedSamples();
}

void GrXferProcessor::getBlendInfo(BlendInfo* blendInfo) const {
    blendInfo->reset();
    if (!this->willReadDstColor()) {
        this->onGetBlendInfo(blendInfo);
    } else if (this->dstReadUsesMixedSamples()) {
        blendInfo->fDstBlend = kIS2A_GrBlendCoeff;
    }
}

void GrXferProcessor::getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b,
                                          const GrSurfaceOrigin* originIfDstTexture) const {
    uint32_t key = this->willReadDstColor() ? 0x1 : 0x0;
    if (key) {
        if (originIfDstTexture) {
            key |= 0x2;
            if (kTopLeft_GrSurfaceOrigin == *originIfDstTexture) {
                key |= 0x4;
            }
        }
        if (this->dstReadUsesMixedSamples()) {
            key |= 0x8;
        }
    }
    if (fIsLCD) {
        key |= 0x10;
    }
    b->add32(key);
    this->onGetGLSLProcessorKey(caps, b);
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
        case kConstA_GrBlendCoeff:
            return "const_alpha";
        case kIConstA_GrBlendCoeff:
            return "inv_const_alpha";
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
        result = GrPorterDuffXPFactory::SrcOverAnalysisProperties(color, coverage, caps,
                                                                  clampType);
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
                                                            bool hasMixedSamples,
                                                            const GrCaps& caps,
                                                            GrClampType clampType) {
    SkASSERT(!hasMixedSamples || caps.shaderCaps()->dualSourceBlendingSupport());
    if (factory) {
        return factory->makeXferProcessor(color, coverage, hasMixedSamples, caps, clampType);
    } else {
        return GrPorterDuffXPFactory::MakeSrcOverXferProcessor(color, coverage, hasMixedSamples,
                                                               caps);
    }
}
