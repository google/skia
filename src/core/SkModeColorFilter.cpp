/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/private/SkColorData.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

template <SkAlphaType kDstAT = kPremul_SkAlphaType>
static SkRGBA4f<kDstAT> map_color(const SkColor4f& c, SkColorSpace* src, SkColorSpace* dst) {
    SkRGBA4f<kDstAT> color = {c.fR, c.fG, c.fB, c.fA};
    SkColorSpaceXformSteps(src, kUnpremul_SkAlphaType,
                           dst, kDstAT).apply(color.vec());
    return color;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

class SkModeColorFilter final : public SkColorFilterBase {
public:
    SkModeColorFilter(const SkColor4f& color, SkBlendMode mode);

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    bool onIsAlphaUnchanged() const override;

#if defined(SK_GANESH)
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*,
                                   const GrColorInfo&,
                                   const SkSurfaceProps&) const override;
#endif
#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

private:
    friend void ::SkRegisterModeColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkModeColorFilter)

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMode(SkColor*, SkBlendMode*) const override;

    skvm::Color onProgram(skvm::Builder*, skvm::Color,
                          const SkColorInfo&, skvm::Uniforms*, SkArenaAlloc*) const override;

    SkColor4f   fColor; // always stored in sRGB
    SkBlendMode fMode;
};

SkModeColorFilter::SkModeColorFilter(const SkColor4f& color,
                                     SkBlendMode mode)
        : fColor(color)
        , fMode(mode) {}

bool SkModeColorFilter::onAsAColorMode(SkColor* color, SkBlendMode* mode) const {
    if (color) {
        *color = fColor.toSkColor();
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
    buffer.writeColor4f(fColor);
    buffer.writeUInt((int) fMode);
}

sk_sp<SkFlattenable> SkModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkPicturePriv::kBlend4fColorFilter)) {
        // Color is 8-bit, sRGB
        SkColor color = buffer.readColor();
        SkBlendMode mode = (SkBlendMode)buffer.readUInt();
        return SkColorFilters::Blend(SkColor4f::FromColor(color), /*sRGB*/nullptr, mode);
    } else {
        // Color is 32-bit, sRGB
        SkColor4f color;
        buffer.readColor4f(&color);
        SkBlendMode mode = (SkBlendMode)buffer.readUInt();
        return SkColorFilters::Blend(color, /*sRGB*/nullptr, mode);
    }
}

bool SkModeColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipelineOp::move_src_dst);
    SkPMColor4f color = map_color(fColor, sk_srgb_singleton(), rec.fDstCS);
    rec.fPipeline->append_constant_color(rec.fAlloc, color.vec());
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

skvm::Color SkModeColorFilter::onProgram(skvm::Builder* p, skvm::Color c,
                                         const SkColorInfo& dstInfo,
                                         skvm::Uniforms* uniforms, SkArenaAlloc*) const {
    SkPMColor4f color = map_color(fColor, sk_srgb_singleton(), dstInfo.colorSpace());
    // The blend program operates on this as if it were premul but the API takes an SkColor4f
    skvm::Color dst = c,
                src = p->uniformColor({color.fR, color.fG, color.fB, color.fA}, uniforms);
    return p->blend(fMode, src,dst);
}

///////////////////////////////////////////////////////////////////////////////
#if defined(SK_GANESH)
#include "src/gpu/Blend.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"

GrFPResult SkModeColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                  GrRecordingContext*,
                                                  const GrColorInfo& dstColorInfo,
                                                  const SkSurfaceProps& props) const {
    if (fMode == SkBlendMode::kDst) {
        // If the blend mode is "dest," the blend color won't factor into it at all.
        // We can return the input FP as-is.
        return GrFPSuccess(std::move(inputFP));
    }

    SkDEBUGCODE(const bool fpHasConstIO = !inputFP || inputFP->hasConstantOutputForConstantInput();)

    SkPMColor4f color = map_color(fColor, sk_srgb_singleton(), dstColorInfo.colorSpace());

    auto colorFP = GrFragmentProcessor::MakeColor(color);
    auto xferFP = GrBlendFragmentProcessor::Make(std::move(colorFP), std::move(inputFP), fMode);

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

#if defined(SK_GRAPHITE)
void SkModeColorFilter::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                 skgpu::graphite::PaintParamsKeyBuilder* builder,
                                 skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SkPMColor4f color = map_color(fColor, sk_srgb_singleton(),
                                  keyContext.dstColorInfo().colorSpace());
    BlendColorFilterBlock::BlendColorFilterData data(fMode, color);

    BlendColorFilterBlock::BeginBlock(keyContext, builder, gatherer, &data);
    builder->endBlock();
}

#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Blend(const SkColor4f& color,
                                           sk_sp<SkColorSpace> colorSpace,
                                           SkBlendMode mode) {
    if (!SkIsValidMode(mode)) {
        return nullptr;
    }

    // First map to sRGB to simplify storage in the actual SkColorFilter instance, staying unpremul
    // until the final dst color space is known when actually filtering.
    SkColor4f srgb = map_color<kUnpremul_SkAlphaType>(
            color, colorSpace.get(), sk_srgb_singleton());

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

    return sk_sp<SkColorFilter>(new SkModeColorFilter(srgb, mode));
}

sk_sp<SkColorFilter> SkColorFilters::Blend(SkColor color, SkBlendMode mode) {
    return Blend(SkColor4f::FromColor(color), /*sRGB*/nullptr, mode);
}

void SkRegisterModeColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkModeColorFilter);
}
