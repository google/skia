/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkModeColorFilter.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkString.h"
#include "include/private/SkArenaAlloc.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkColorData.h"
#include "src/core/SkColorSpaceXformer.h"
#include "src/core/SkPM4f.h"
#include "src/core/SkPM4fPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"
#include "src/utils/SkUTF.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

SkModeColorFilter::SkModeColorFilter(SkColor color, SkBlendMode mode) {
    fColor = color;
    fMode = mode;
}

bool SkModeColorFilter::asColorMode(SkColor* color, SkBlendMode* mode) const {
    if (color) {
        *color = fColor;
    }
    if (mode) {
        *mode = fMode;
    }
    return true;
}

uint32_t SkModeColorFilter::getFlags() const {
    uint32_t flags = 0;
    switch (fMode) {
        case SkBlendMode::kDst:      //!< [Da, Dc]
        case SkBlendMode::kSrcATop:  //!< [Da, Sc * Da + (1 - Sa) * Dc]
            flags |= kAlphaUnchanged_Flag;
        default:
            break;
    }
    return flags;
}

void SkModeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
    buffer.writeUInt((int)fMode);
}

sk_sp<SkFlattenable> SkModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkColor color = buffer.readColor();
    SkBlendMode mode = (SkBlendMode)buffer.readUInt();
    return SkColorFilter::MakeModeFilter(color, mode);
}

void SkModeColorFilter::onAppendStages(SkRasterPipeline* p,
                                       SkColorSpace* dst,
                                       SkArenaAlloc* scratch,
                                       bool shaderIsOpaque) const {
    p->append(SkRasterPipeline::move_src_dst);
    p->append_constant_color(scratch, premul_in_dst_colorspace(fColor, dst));
    SkBlendMode_AppendStages(fMode, p);
}

sk_sp<SkColorFilter> SkModeColorFilter::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkColor color = xformer->apply(fColor);
    if (color != fColor) {
        return SkColorFilter::MakeModeFilter(color, fMode);
    }
    return this->INHERITED::onMakeColorSpace(xformer);
}

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "include/gpu/GrBlend.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrConstColorProcessor.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"

std::unique_ptr<GrFragmentProcessor> SkModeColorFilter::asFragmentProcessor(
        GrContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const {
    if (SkBlendMode::kDst == fMode) {
        return nullptr;
    }

    auto constFP = GrConstColorProcessor::Make(SkColorToPremulGrColor4f(fColor, dstColorSpaceInfo),
                                               GrConstColorProcessor::InputMode::kIgnore);
    auto fp = GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(constFP), fMode);
    if (!fp) {
        return nullptr;
    }
#ifdef SK_DEBUG
    // With a solid color input this should always be able to compute the blended color
    // (at least for coeff modes)
    if ((unsigned)fMode <= (unsigned)SkBlendMode::kLastCoeffMode) {
        SkASSERT(fp->hasConstantOutputForConstantInput());
    }
#endif
    return fp;
}

#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilter::MakeModeFilter(SkColor color, SkBlendMode mode) {
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
