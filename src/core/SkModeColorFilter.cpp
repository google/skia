/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkModeColorFilter.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

SkModeColorFilter::SkModeColorFilter(SkColor color, SkBlendMode mode) {
    fColor = color;
    fMode = mode;
}

bool SkModeColorFilter::onAsAColorMode(SkColor* color, SkBlendMode* mode) const {
    if (color) {
        *color = fColor;
    }
    if (mode) {
        *mode = fMode;
    }
    return true;
}

bool SkModeColorFilter::onIsAlphaUnchanged() const {
    switch (fMode) {
        case SkBlendMode::kDst:      //!< [Da, Dc]
        case SkBlendMode::kSrcATop:  //!< [Da, Sc * Da + (1 - Sa) * Dc]
            return true;
        default:
            break;
    }
    return false;
}

void SkModeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
    buffer.writeUInt((int)fMode);
}

sk_sp<SkFlattenable> SkModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkColor color = buffer.readColor();
    SkBlendMode mode = (SkBlendMode)buffer.readUInt();
    return SkColorFilters::Blend(color, mode);
}

bool SkModeColorFilter::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipeline::move_src_dst);
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           rec.fDstCS,          kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

skvm::Color SkModeColorFilter::onProgram(skvm::Builder* p, skvm::Color c,
                                         SkColorSpace* dstCS,
                                         skvm::Uniforms* uniforms, SkArenaAlloc*) const {
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                         dstCS,   kPremul_SkAlphaType).apply(color.vec());
    skvm::Color dst = c,
                src = p->uniformColor(color, uniforms);
    return p->blend(fMode, src,dst);
}

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "src/gpu/GrBlend.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"

GrFPResult SkModeColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                  GrRecordingContext*,
                                                  const GrColorInfo& dstColorInfo) const {
    if (fMode == SkBlendMode::kDst) {
        // If the blend mode is "dest," the blend color won't factor into it at all.
        // We can return the input FP as-is.
        return GrFPSuccess(std::move(inputFP));
    }

    SkDEBUGCODE(const bool fpHasConstIO = !inputFP || inputFP->hasConstantOutputForConstantInput();)

    auto colorFP = GrConstColorProcessor::Make(SkColorToPMColor4f(fColor, dstColorInfo));
    auto xferFP = GrBlendFragmentProcessor::Make(
            std::move(colorFP), std::move(inputFP), fMode,
            GrBlendFragmentProcessor::BlendBehavior::kSkModeBehavior);

    if (xferFP == nullptr) {
        // This is only expected to happen if the blend mode is "dest" and the input FP is null.
        // Since we already did an early-out in the "dest" blend mode case, we shouldn't get here.
        SkDEBUGFAIL("GrBlendFragmentProcessor::Make returned null unexpectedly");
        return GrFPFailure(nullptr);
    }

    // With a solid color input this should always be able to compute the blended color
    // (at least for coeff modes).
    // Occasionally, we even do better than we started; specifically, in "src" blend mode, we end up
    // ditching the input FP entirely, which turns a non-constant operation into a constant one.
    SkASSERT(fMode > SkBlendMode::kLastCoeffMode ||
             xferFP->hasConstantOutputForConstantInput() >= fpHasConstIO);

    return GrFPSuccess(std::move(xferFP));
}

#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Blend(SkColor color, SkBlendMode mode) {
    if (!SkIsValidMode(mode)) {
        return nullptr;
    }

    unsigned alpha = SkColorGetA(color);

    // first collaps some modes if possible

    if (SkBlendMode::kClear == mode) {
        color = 0;
        mode = SkBlendMode::kSrc;
    } else if (SkBlendMode::kSrcOver == mode) {
        if (0 == alpha) {
            mode = SkBlendMode::kDst;
        } else if (255 == alpha) {
            mode = SkBlendMode::kSrc;
        }
        // else just stay srcover
    }

    // weed out combinations that are noops, and just return null
    if (SkBlendMode::kDst == mode ||
        (0 == alpha && (SkBlendMode::kSrcOver == mode ||
                        SkBlendMode::kDstOver == mode ||
                        SkBlendMode::kDstOut == mode ||
                        SkBlendMode::kSrcATop == mode ||
                        SkBlendMode::kXor == mode ||
                        SkBlendMode::kDarken == mode)) ||
            (0xFF == alpha && SkBlendMode::kDstIn == mode)) {
        return nullptr;
    }

    return SkModeColorFilter::Make(color, mode);
}
