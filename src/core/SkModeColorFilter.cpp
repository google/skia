/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBlendModePriv.h"
#include "SkBlitRow.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkColorSpaceXformer.h"
#include "SkModeColorFilter.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkRandom.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkUtils.h"
#include "SkValidationUtils.h"
#include "SkWriteBuffer.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

SkModeColorFilter::SkModeColorFilter(SkColor color, SkBlendMode mode) {
    fColor = color;
    fMode = mode;
    // cache
    fPMColor = SkPreMultiplyColor(fColor);
}

#ifndef SK_IGNORE_TO_STRING
void SkModeColorFilter::toString(SkString* str) const {
    str->append("SkModeColorFilter: color: 0x");
    str->appendHex(fColor);
    str->append(" mode: ");
    str->append(SkBlendMode_Name(fMode));
}
#endif

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
    p->append_uniform_color(scratch, SkPM4f_from_SkColor(fColor, dst));
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
#include "GrBlend.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#include "effects/GrConstColorProcessor.h"
#include "SkGr.h"

sk_sp<GrFragmentProcessor> SkModeColorFilter::asFragmentProcessor(
                                                    GrContext*, SkColorSpace* dstColorSpace) const {
    if (SkBlendMode::kDst == fMode) {
        return nullptr;
    }

    sk_sp<GrFragmentProcessor> constFP(
        GrConstColorProcessor::Make(SkColorToPremulGrColor4f(fColor, dstColorSpace),
                                    GrConstColorProcessor::kIgnore_InputMode));
    sk_sp<GrFragmentProcessor> fp(
        GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(constFP), fMode));
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
