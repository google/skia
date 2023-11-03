/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/colorfilters/SkBlendModeColorFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkColorData.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

template <SkAlphaType kDstAT = kPremul_SkAlphaType>
static SkRGBA4f<kDstAT> map_color(const SkColor4f& c, SkColorSpace* src, SkColorSpace* dst) {
    SkRGBA4f<kDstAT> color = {c.fR, c.fG, c.fB, c.fA};
    SkColorSpaceXformSteps(src, kUnpremul_SkAlphaType, dst, kDstAT).apply(color.vec());
    return color;
}

SkBlendModeColorFilter::SkBlendModeColorFilter(const SkColor4f& color, SkBlendMode mode)
        : fColor(color), fMode(mode) {}

bool SkBlendModeColorFilter::onAsAColorMode(SkColor* color, SkBlendMode* mode) const {
    if (color) {
        *color = fColor.toSkColor();
    }
    if (mode) {
        *mode = fMode;
    }
    return true;
}

bool SkBlendModeColorFilter::onIsAlphaUnchanged() const {
    switch (fMode) {
        case SkBlendMode::kDst:      //!< [Da, Dc]
        case SkBlendMode::kSrcATop:  //!< [Da, Sc * Da + (1 - Sa) * Dc]
            return true;
        default:
            break;
    }
    return false;
}

void SkBlendModeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor4f(fColor);
    buffer.writeUInt((int)fMode);
}

sk_sp<SkFlattenable> SkBlendModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkPicturePriv::kBlend4fColorFilter)) {
        // Color is 8-bit, sRGB
        SkColor color = buffer.readColor();
        SkBlendMode mode = (SkBlendMode)buffer.readUInt();
        return SkColorFilters::Blend(SkColor4f::FromColor(color), /*sRGB*/ nullptr, mode);
    } else {
        // Color is 32-bit, sRGB
        SkColor4f color;
        buffer.readColor4f(&color);
        SkBlendMode mode = (SkBlendMode)buffer.readUInt();
        return SkColorFilters::Blend(color, /*sRGB*/ nullptr, mode);
    }
}

bool SkBlendModeColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipelineOp::move_src_dst);
    SkPMColor4f color = map_color(fColor, sk_srgb_singleton(), rec.fDstCS);
    rec.fPipeline->appendConstantColor(rec.fAlloc, color.vec());
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Blend(const SkColor4f& color,
                                           sk_sp<SkColorSpace> colorSpace,
                                           SkBlendMode mode) {
    if (!SkIsValidMode(mode)) {
        return nullptr;
    }

    // First map to sRGB to simplify storage in the actual SkColorFilter instance, staying unpremul
    // until the final dst color space is known when actually filtering.
    SkColor4f srgb = map_color<kUnpremul_SkAlphaType>(color, colorSpace.get(), sk_srgb_singleton());

    // Next collapse some modes if possible
    float alpha = srgb.fA;
    if (SkBlendMode::kClear == mode) {
        srgb = SkColors::kTransparent;
        mode = SkBlendMode::kSrc;
    } else if (SkBlendMode::kSrcOver == mode) {
        if (0.f == alpha) {
            mode = SkBlendMode::kDst;
        } else if (1.f == alpha) {
            mode = SkBlendMode::kSrc;
        }
        // else just stay srcover
    }

    // Finally weed out combinations that are noops, and just return null
    if (SkBlendMode::kDst == mode ||
        (0.f == alpha && (SkBlendMode::kSrcOver == mode ||
                          SkBlendMode::kDstOver == mode ||
                          SkBlendMode::kDstOut == mode ||
                          SkBlendMode::kSrcATop == mode ||
                          SkBlendMode::kXor == mode ||
                          SkBlendMode::kDarken == mode)) ||
            (1.f == alpha && SkBlendMode::kDstIn == mode)) {
        return nullptr;
    }

    return sk_sp<SkColorFilter>(new SkBlendModeColorFilter(srgb, mode));
}

sk_sp<SkColorFilter> SkColorFilters::Blend(SkColor color, SkBlendMode mode) {
    return Blend(SkColor4f::FromColor(color), /*sRGB*/ nullptr, mode);
}

void SkRegisterModeColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlendModeColorFilter);
    // Previous name
    SkFlattenable::Register("SkModeColorFilter", SkBlendModeColorFilter::CreateProc);
}
