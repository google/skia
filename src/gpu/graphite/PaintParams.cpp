/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParams.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkShader.h"
#include "src/core/SkBlendModeBlender.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/gpu/DitherUtils.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

namespace {

// This should be kept in sync w/ SkPaintPriv::ShouldDither
bool should_dither(const PaintParams& p, SkColorType dstCT) {
    // The paint dither flag can veto.
    if (!p.dither()) {
        return false;
    }

    if (dstCT == kUnknown_SkColorType) {
        return false;
    }

    // We always dither 565 or 4444 when requested.
    if (dstCT == kRGB_565_SkColorType || dstCT == kARGB_4444_SkColorType) {
        return true;
    }

    // Otherwise, dither is only needed for non-const paints.
    return p.shader() && !as_SB(p.shader())->isConstant();
}

} // anonymous namespace

PaintParams::PaintParams(const SkColor4f& color,
                         sk_sp<SkBlender> finalBlender,
                         sk_sp<SkShader> shader,
                         sk_sp<SkColorFilter> colorFilter,
                         sk_sp<SkBlender> primitiveBlender,
                         DstReadRequirement dstReadReq,
                         bool skipColorXform,
                         bool dither)
        : fColor(color)
        , fFinalBlender(std::move(finalBlender))
        , fShader(std::move(shader))
        , fColorFilter(std::move(colorFilter))
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fDstReadReq(dstReadReq)
        , fSkipColorXform(skipColorXform)
        , fDither(dither) {}

PaintParams::PaintParams(const SkPaint& paint,
                         sk_sp<SkBlender> primitiveBlender,
                         DstReadRequirement dstReadReq,
                         bool skipColorXform)
        : fColor(paint.getColor4f())
        , fFinalBlender(paint.refBlender())
        , fShader(paint.refShader())
        , fColorFilter(paint.refColorFilter())
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fDstReadReq(dstReadReq)
        , fSkipColorXform(skipColorXform)
        , fDither(paint.isDither()) {}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

std::optional<SkBlendMode> PaintParams::asFinalBlendMode() const {
    return fFinalBlender ? as_BB(fFinalBlender)->asBlendMode()
                         : SkBlendMode::kSrcOver;
}

sk_sp<SkBlender> PaintParams::refFinalBlender() const { return fFinalBlender; }

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

sk_sp<SkColorFilter> PaintParams::refColorFilter() const { return fColorFilter; }

sk_sp<SkBlender> PaintParams::refPrimitiveBlender() const { return fPrimitiveBlender; }

SkColor4f PaintParams::Color4fPrepForDst(SkColor4f srcColor, const SkColorInfo& dstColorInfo) {
    // xform from sRGB to the destination colorspace
    SkColorSpaceXformSteps steps(sk_srgb_singleton(),       kUnpremul_SkAlphaType,
                                 dstColorInfo.colorSpace(), kUnpremul_SkAlphaType);

    SkColor4f result = srcColor;
    steps.apply(result.vec());
    return result;
}

void PaintParams::toKey(const KeyContext& keyContext,
                        PaintParamsKeyBuilder* builder,
                        PipelineDataGatherer* gatherer) const {
    // TODO: figure out how we can omit this block when the Paint's color isn't used.
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, keyContext.paintColor());
    builder->endBlock();

    bool needsDstSample = fDstReadReq == DstReadRequirement::kTextureCopy ||
                          fDstReadReq == DstReadRequirement::kTextureSample;
    SkASSERT(needsDstSample == SkToBool(keyContext.dstTexture()));
    if (needsDstSample) {
        DstReadSampleBlock::BeginBlock(
                keyContext, builder, gatherer, keyContext.dstTexture(), keyContext.dstOffset());
        builder->endBlock();

    } else if (fDstReadReq == DstReadRequirement::kFramebufferFetch) {
        DstReadFetchBlock::BeginBlock(keyContext, builder, gatherer);
        builder->endBlock();
    }

    AddToKey(keyContext, builder, gatherer, fShader.get());

    if (fPrimitiveBlender) {
        AddPrimitiveBlendBlock(keyContext, builder, gatherer, fPrimitiveBlender.get());
    }

    // Apply the paint's alpha value.
    if (fColor.fA != 1.0f) {
        AddColorBlendBlock(
                keyContext, builder, gatherer, SkBlendMode::kDstIn, {0, 0, 0, fColor.fA});
    }

    AddToKey(keyContext, builder, gatherer, fColorFilter.get());

#ifndef SK_IGNORE_GPU_DITHER
    SkColorType ct = keyContext.dstColorInfo().colorType();
    if (should_dither(*this, ct)) {
        DitherShaderBlock::DitherData data(skgpu::DitherRangeForConfig(ct));

        DitherShaderBlock::BeginBlock(keyContext, builder, gatherer, &data);
        builder->endBlock();
    }
#endif

    std::optional<SkBlendMode> finalBlendMode = this->asFinalBlendMode();
    // If this draw needs a dst read, we are either doing custom blending or we cannot handle the
    // combination of blend mode and coverage using fixed function blend hardware.
    if (fDstReadReq != DstReadRequirement::kNone) {
        // Add a block to implement the blending in the shader.
        static const SkBlendModeBlender kDefaultBlender(SkBlendMode::kSrcOver);
        AddDstBlendBlock(keyContext,
                         builder,
                         gatherer,
                         fFinalBlender ? fFinalBlender.get() : &kDefaultBlender);
        finalBlendMode = SkBlendMode::kSrc;
    }

    // Set the hardware blend mode.
    SkASSERT(finalBlendMode);
    BuiltInCodeSnippetID fixedFuncBlendModeID = static_cast<BuiltInCodeSnippetID>(
            kFixedFunctionBlendModeIDOffset + static_cast<int>(*finalBlendMode));
    builder->beginBlock(fixedFuncBlendModeID);
    builder->endBlock();
}

} // namespace skgpu::graphite
