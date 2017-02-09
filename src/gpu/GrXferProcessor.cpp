/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrXferProcessor.h"
#include "GrPipeline.h"
#include "GrProcOptInfo.h"
#include "gl/GrGLCaps.h"

GrXferProcessor::GrXferProcessor()
    : fWillReadDstColor(false)
    , fDstReadUsesMixedSamples(false)
    , fDstTextureOffset() {
}

GrXferProcessor::GrXferProcessor(const DstTexture* dstTexture,
                                 bool willReadDstColor,
                                 bool hasMixedSamples)
    : fWillReadDstColor(willReadDstColor)
    , fDstReadUsesMixedSamples(willReadDstColor && hasMixedSamples)
    , fDstTextureOffset() {
    if (dstTexture && dstTexture->texture()) {
        SkASSERT(willReadDstColor);
        fDstTexture.reset(dstTexture->texture());
        fDstTextureOffset = dstTexture->offset();
        this->addTextureSampler(&fDstTexture);
    }
}

GrXferProcessor::OptFlags GrXferProcessor::getOptimizations(const GrPipelineAnalysis& analysis,
                                                            bool doesStencilWrite,
                                                            GrColor* overrideColor,
                                                            const GrCaps& caps) const {
    GrXferProcessor::OptFlags flags =
            this->onGetOptimizations(analysis, doesStencilWrite, overrideColor, caps);
    return flags;
}

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

void GrXferProcessor::getGLSLProcessorKey(const GrShaderCaps& caps,
                                          GrProcessorKeyBuilder* b) const {
    uint32_t key = this->willReadDstColor() ? 0x1 : 0x0;
    if (key) {
        if (const GrTexture* dstTexture = this->getDstTexture()) {
            key |= 0x2;
            if (kTopLeft_GrSurfaceOrigin == dstTexture->origin()) {
                key |= 0x4;
            }
        }
        if (this->dstReadUsesMixedSamples()) {
            key |= 0x8;
        }
    }
    b->add32(key);
    this->onGetGLSLProcessorKey(caps, b);
}

GrXferBarrierType GrXferProcessor::xferBarrierType(const GrRenderTarget* rt,
                                                   const GrCaps& caps) const {
    SkASSERT(rt);
    if (static_cast<const GrSurface*>(rt) == this->getDstTexture()) {
        // Texture barriers are required when a shader reads and renders to the same texture.
        SkASSERT(caps.textureBarrierSupport());
        return kTexture_GrXferBarrierType;
    }
    return this->onXferBarrier(rt, caps);
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
    };
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
    }
    return "";
}

SkString GrXferProcessor::BlendInfo::dump() const {
    SkString out;
    out.printf("write_color(%d) equation(%s) src_coeff(%s) dst_coeff:(%s) const(0x%08x)",
               fWriteColor, equation_string(fEquation), coeff_string(fSrcBlend),
               coeff_string(fDstBlend), fBlendConstant);
    return out;
}
#endif

///////////////////////////////////////////////////////////////////////////////

using ColorType = GrXPFactory::ColorType;
using CoverageType = GrXPFactory::CoverageType;

ColorType analysis_color_type(const GrPipelineAnalysis& analysis) {
    if (analysis.fColorPOI.validFlags() == kRGBA_GrColorComponentFlags) {
        return GrColorIsOpaque(analysis.fColorPOI.color()) ? ColorType::kOpaqueConstant
                                                           : ColorType::kConstant;
    }
    if ((analysis.fColorPOI.validFlags() & kA_GrColorComponentFlag) &&
        GrColorIsOpaque(analysis.fColorPOI.color())) {
        return ColorType::kOpaque;
    }
    return ColorType::kUnknown;
}

CoverageType analysis_coverage_type(const GrPipelineAnalysis& analysis) {
    if (analysis.fCoveragePOI.isSolidWhite()) {
        return CoverageType::kNone;
    }
    if (analysis.fCoveragePOI.isLCDCoverage()) {
        return CoverageType::kLCD;
    }
    return CoverageType::kSingleChannel;
}

bool GrXPFactory::willReadDstColor(const GrCaps& caps, const GrPipelineAnalysis& analysis) const {
    if (analysis.fUsesPLSDstRead) {
        return true;
    }
    ColorType colorType = analysis_color_type(analysis);
    CoverageType coverageType = analysis_coverage_type(analysis);
    return this->willReadDstColor(caps, colorType, coverageType);
}

GrXferProcessor* GrXPFactory::createXferProcessor(const GrPipelineAnalysis& analysis,
                                                  bool hasMixedSamples,
                                                  const DstTexture* dstTexture,
                                                  const GrCaps& caps) const {
#ifdef SK_DEBUG
    if (this->willReadDstColor(caps, analysis)) {
        if (!caps.shaderCaps()->dstReadInShaderSupport()) {
            SkASSERT(dstTexture && dstTexture->texture());
        } else {
            SkASSERT(!dstTexture || !dstTexture->texture());
        }
    } else {
        SkASSERT(!dstTexture || !dstTexture->texture());
    }
    SkASSERT(!hasMixedSamples || caps.shaderCaps()->dualSourceBlendingSupport());
#endif
    return this->onCreateXferProcessor(caps, analysis, hasMixedSamples, dstTexture);
}

bool GrXPFactory::willNeedDstTexture(const GrCaps& caps, const GrPipelineAnalysis& analysis) const {
    return !analysis.fUsesPLSDstRead && !caps.shaderCaps()->dstReadInShaderSupport() &&
           this->willReadDstColor(caps, analysis);
}
