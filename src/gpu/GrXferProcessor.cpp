/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrXferProcessor.h"
#include "gl/GrGLCaps.h"

GrXferProcessor::GrXferProcessor()
    : fWillReadDstColor(false), fReadsCoverage(true), fDstCopyTextureOffset() {
}

GrXferProcessor::GrXferProcessor(const GrDeviceCoordTexture* dstCopy, bool willReadDstColor)
    : fWillReadDstColor(willReadDstColor)
    , fReadsCoverage(true)
    , fDstCopyTextureOffset() {
    if (dstCopy && dstCopy->texture()) {
        fDstCopy.reset(dstCopy->texture());
        fDstCopyTextureOffset = dstCopy->offset();
        this->addTextureAccess(&fDstCopy);
        this->setWillReadFragmentPosition();
    }
}

GrXferProcessor::OptFlags GrXferProcessor::getOptimizations(const GrProcOptInfo& colorPOI,
                                                            const GrProcOptInfo& coveragePOI,
                                                            bool doesStencilWrite,
                                                            GrColor* overrideColor,
                                                            const GrDrawTargetCaps& caps) {
    GrXferProcessor::OptFlags flags = this->onGetOptimizations(colorPOI,
                                                               coveragePOI,
                                                               doesStencilWrite,
                                                               overrideColor,
                                                               caps);

    if (flags & GrXferProcessor::kIgnoreCoverage_OptFlag) {
        fReadsCoverage = false;
    }
    return flags;
}

void GrXferProcessor::getGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    uint32_t key = this->willReadDstColor() ? 0x1 : 0x0;
    if (this->getDstCopyTexture() &&
        kTopLeft_GrSurfaceOrigin == this->getDstCopyTexture()->origin()) {
        key |= 0x2;
    }
    b->add32(key);
    this->onGetGLProcessorKey(caps, b);
}

bool GrXferProcessor::willNeedXferBarrier(const GrRenderTarget* rt,
                                          const GrDrawTargetCaps& caps,
                                          GrXferBarrierType* outBarrierType) const {
    if (static_cast<const GrSurface*>(rt) == this->getDstCopyTexture()) {
        // Texture barriers are required when a shader reads and renders to the same texture.
        SkASSERT(rt);
        SkASSERT(caps.textureBarrierSupport());
        *outBarrierType = kTexture_GrXferBarrierType;
        return true;
    }
    return this->onWillNeedXferBarrier(rt, caps, outBarrierType);
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

GrXferProcessor* GrXPFactory::createXferProcessor(const GrProcOptInfo& colorPOI,
                                                  const GrProcOptInfo& coveragePOI,
                                                  const GrDeviceCoordTexture* dstCopy,
                                                  const GrDrawTargetCaps& caps) const {
#ifdef SK_DEBUG
    if (this->willReadDstColor(caps, colorPOI, coveragePOI)) {
        if (!caps.shaderCaps()->dstReadInShaderSupport()) {
            SkASSERT(dstCopy && dstCopy->texture());
        } else {
            SkASSERT(!dstCopy || !dstCopy->texture()); 
        }
    } else {
        SkASSERT(!dstCopy || !dstCopy->texture()); 
    }
#endif
    return this->onCreateXferProcessor(caps, colorPOI, coveragePOI, dstCopy);
}

bool GrXPFactory::willNeedDstCopy(const GrDrawTargetCaps& caps, const GrProcOptInfo& colorPOI,
                                  const GrProcOptInfo& coveragePOI) const {
    return (this->willReadDstColor(caps, colorPOI, coveragePOI) 
            && !caps.shaderCaps()->dstReadInShaderSupport());
}

