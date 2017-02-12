/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkBlendModePriv.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkArenaAlloc.h"
#include "SkModeColorFilter.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkUtils.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkValidationUtils.h"
#include "SkPM4f.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkModeColorFilter::toString(SkString* str) const {
    str->append("SkModeColorFilter: color: 0x");
    str->appendHex(fColor);
    str->append(" mode: ");
    str->append(SkXfermode::ModeName(fMode));
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

void SkModeColorFilter::filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const {
    SkPMColor       color = fPMColor;
    SkXfermodeProc  proc = fProc;

    for (int i = 0; i < count; i++) {
        result[i] = proc(color, shader[i]);
    }
}

void SkModeColorFilter::filterSpan4f(const SkPM4f shader[], int count, SkPM4f result[]) const {
    SkXfermodeProc4f  proc = SkXfermode::GetProc4f(fMode);
    auto pm4f = SkColor4f::FromColor(fColor).premul();
    for (int i = 0; i < count; i++) {
        result[i] = proc(pm4f, shader[i]);
    }
}

void SkModeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
    buffer.writeUInt((int)fMode);
}

void SkModeColorFilter::updateCache() {
    fPMColor = SkPreMultiplyColor(fColor);
    fProc = SkXfermode::GetProc(fMode);
}

sk_sp<SkFlattenable> SkModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkColor color = buffer.readColor();
    SkBlendMode mode = (SkBlendMode)buffer.readUInt();
    return SkColorFilter::MakeModeFilter(color, mode);
}

bool SkModeColorFilter::onAppendStages(SkRasterPipeline* p,
                                       SkColorSpace* dst,
                                       SkArenaAlloc* scratch,
                                       bool shaderIsOpaque) const {
    auto color = scratch->make<SkPM4f>(SkPM4f_from_SkColor(fColor, dst));

    p->append(SkRasterPipeline::move_src_dst);
    p->append(SkRasterPipeline::constant_color, color);
    auto mode = (SkBlendMode)fMode;
    if (!SkBlendMode_AppendStages(mode, p)) {
        return false;
    }
    if (SkBlendMode_CanOverflow(mode)) { p->append(SkRasterPipeline::clamp_a); }
    return true;
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

class Src_SkModeColorFilter final : public SkModeColorFilter {
public:
    Src_SkModeColorFilter(SkColor color) : INHERITED(color, SkBlendMode::kSrc) {}

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        sk_memset32(result, this->getPMColor(), count);
    }

private:
    typedef SkModeColorFilter INHERITED;
};

class SrcOver_SkModeColorFilter final : public SkModeColorFilter {
public:
    SrcOver_SkModeColorFilter(SkColor color) : INHERITED(color, SkBlendMode::kSrcOver) { }

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        SkBlitRow::Color32(result, shader, count, this->getPMColor());
    }

private:
    typedef SkModeColorFilter INHERITED;
};

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

    switch (mode) {
        case SkBlendMode::kSrc:
            return sk_make_sp<Src_SkModeColorFilter>(color);
        case SkBlendMode::kSrcOver:
            return sk_make_sp<SrcOver_SkModeColorFilter>(color);
        default:
            return SkModeColorFilter::Make(color, mode);
    }
}
