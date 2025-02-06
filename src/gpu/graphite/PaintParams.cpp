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
#include "src/gpu/Blend.h"
#include "src/gpu/DitherUtils.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

namespace {

// This should be kept in sync w/ SkPaintPriv::ShouldDither and PaintOption::shouldDither
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

PaintParams::PaintParams(const SkPaint& paint,
                         sk_sp<SkBlender> primitiveBlender,
                         const NonMSAAClip& nonMSAAClip,
                         sk_sp<SkShader> clipShader,
                         bool dstReadRequired,
                         bool skipColorXform)
        : fColor(paint.getColor4f())
        , fFinalBlender(paint.refBlender())
        , fShader(paint.refShader())
        , fColorFilter(paint.refColorFilter())
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fNonMSAAClip(nonMSAAClip)
        , fClipShader(std::move(clipShader))
        , fDstReadRequired(dstReadRequired)
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

void AddFixedBlendMode(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       SkBlendMode bm) {
    SkASSERT(bm <= SkBlendMode::kLastMode);
    BuiltInCodeSnippetID id = static_cast<BuiltInCodeSnippetID>(kFixedBlendIDOffset +
                                                                static_cast<int>(bm));
    builder->addBlock(id);
}

void AddBlendMode(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  SkBlendMode bm) {
    // For non-fixed blends, coefficient blend modes are combined into the same shader snippet.
    // The same goes for the HSLC advanced blends. The remaining advanced blends are fairly unique
    // in their implementations. To avoid having to compile all of their SkSL, they are treated as
    // fixed blend modes.
    SkSpan<const float> coeffs = skgpu::GetPorterDuffBlendConstants(bm);
    if (!coeffs.empty()) {
        PorterDuffBlenderBlock::AddBlock(keyContext, builder, gatherer, coeffs);
    } else if (bm >= SkBlendMode::kHue) {
        ReducedBlendModeInfo blendInfo = GetReducedBlendModeInfo(bm);
        HSLCBlenderBlock::AddBlock(keyContext, builder, gatherer, blendInfo.fUniformData);
    } else {
        AddFixedBlendMode(keyContext, builder, gatherer, bm);
    }
}

void AddDitherBlock(const KeyContext& keyContext,
                    PaintParamsKeyBuilder* builder,
                    PipelineDataGatherer* gatherer,
                    SkColorType ct) {
    static const SkBitmap gLUT = skgpu::MakeDitherLUT();

    sk_sp<TextureProxy> proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(), gLUT,
                                                                "DitherLUT");
    if (keyContext.recorder() && !proxy) {
        SKGPU_LOG_W("Couldn't create dither shader's LUT");
        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
        return;
    }

    DitherShaderBlock::DitherData data(skgpu::DitherRangeForConfig(ct), std::move(proxy));

    DitherShaderBlock::AddBlock(keyContext, builder, gatherer, data);
}

void PaintParams::addPaintColorToKey(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* keyBuilder,
                                     PipelineDataGatherer* gatherer) const {
    if (fShader) {
        AddToKey(keyContext, keyBuilder, gatherer, fShader.get());
    } else {
        RGBPaintColorBlock::AddBlock(keyContext, keyBuilder, gatherer);
    }
}

/**
 * Primitive blend blocks are used to blend either the paint color or the output of another shader
 * with a primitive color emitted by certain draw geometry calls (drawVertices, drawAtlas, etc.).
 * Dst: primitiveColor Src: Paint color/shader output
 */
void PaintParams::handlePrimitiveColor(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* keyBuilder,
                                       PipelineDataGatherer* gatherer) const {
    if (fPrimitiveBlender) {
        Blend(keyContext, keyBuilder, gatherer,
              /* addBlendToKey= */ [&] () -> void {
                  AddToKey(keyContext, keyBuilder, gatherer, fPrimitiveBlender.get());
              },
              /* addSrcToKey= */ [&]() -> void {
                  this->addPaintColorToKey(keyContext, keyBuilder, gatherer);
              },
              /* addDstToKey= */ [&]() -> void {
                  // When fSkipColorXform is true, it's assumed that the primitive color is
                  // already in the dst color space. We could change the paint key to not have
                  // any colorspace block wrapping the primitive color block, but for now just
                  // use the dst color space as the src color space to produce an identity CS
                  // transform.
                  //
                  // When fSkipColorXform is false (most cases), it's assumed to be in sRGB.
                  const SkColorSpace* primitiveCS =
                        fSkipColorXform ? keyContext.dstColorInfo().colorSpace()
                                        : sk_srgb_singleton();
                  AddPrimitiveColor(keyContext, keyBuilder, gatherer, primitiveCS);
              });
    } else {
        this->addPaintColorToKey(keyContext, keyBuilder, gatherer);
    }
}

// Apply the paint's alpha value.
void PaintParams::handlePaintAlpha(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* keyBuilder,
                                   PipelineDataGatherer* gatherer) const {

    if (!fShader && !fPrimitiveBlender) {
        // If there is no shader and no primitive blending the input to the colorFilter stage
        // is just the premultiplied paint color.
        SkPMColor4f paintColor = PaintParams::Color4fPrepForDst(fColor,
                                                                keyContext.dstColorInfo()).premul();
        SolidColorShaderBlock::AddBlock(keyContext, keyBuilder, gatherer, paintColor);
        return;
    }

    if (fColor.fA != 1.0f) {
        Blend(keyContext, keyBuilder, gatherer,
              /* addBlendToKey= */ [&] () -> void {
                  AddFixedBlendMode(keyContext, keyBuilder, gatherer, SkBlendMode::kSrcIn);
              },
              /* addSrcToKey= */ [&]() -> void {
                  this->handlePrimitiveColor(keyContext, keyBuilder, gatherer);
              },
              /* addDstToKey= */ [&]() -> void {
                  AlphaOnlyPaintColorBlock::AddBlock(keyContext, keyBuilder, gatherer);
              });
    } else {
        this->handlePrimitiveColor(keyContext, keyBuilder, gatherer);
    }
}

void PaintParams::handleColorFilter(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
                                    PipelineDataGatherer* gatherer) const {
    if (fColorFilter) {
        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    this->handlePaintAlpha(keyContext, builder, gatherer);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddToKey(keyContext, builder, gatherer, fColorFilter.get());
                });
    } else {
        this->handlePaintAlpha(keyContext, builder, gatherer);
    }
}

void PaintParams::handleDithering(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer) const {

#ifndef SK_IGNORE_GPU_DITHER
    SkColorType ct = keyContext.dstColorInfo().colorType();
    if (should_dither(*this, ct)) {
        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    this->handleColorFilter(keyContext, builder, gatherer);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddDitherBlock(keyContext, builder, gatherer, ct);
                });
    } else
#endif
    {
        this->handleColorFilter(keyContext, builder, gatherer);
    }
}

void PaintParams::handleClipping(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* builder,
                                 PipelineDataGatherer* gatherer) const {
    if (!fNonMSAAClip.isEmpty()) {
        const AnalyticClip& analyticClip = fNonMSAAClip.fAnalyticClip;
        SkPoint radiusPair;
        SkRect analyticBounds;
        if (!analyticClip.isEmpty()) {
            float radius = analyticClip.fRadius + 0.5f;
            // N.B.: Because the clip data is normally used with depth-based clipping,
            // the shape is inverted from its usual state. We re-invert here to
            // match what the shader snippet expects.
            radiusPair = {(analyticClip.fInverted) ? radius : -radius, 1.0f/radius};
            analyticBounds = analyticClip.fBounds.makeOutset(0.5f).asSkRect();
        } else {
            // This will generate no analytic clip.
            radiusPair = { -0.5f, 1.f };
            analyticBounds = { 0, 0, 0, 0 };
        }

        const AtlasClip& atlasClip = fNonMSAAClip.fAtlasClip;
        SkISize maskSize = atlasClip.fMaskBounds.size();
        SkRect texMaskBounds = SkRect::MakeXYWH(atlasClip.fOutPos.x(), atlasClip.fOutPos.y(),
                                                maskSize.width(), maskSize.height());
        // Outset bounds to capture some of the padding (necessary for inverse clip)
        texMaskBounds.outset(0.5f, 0.5f);
        SkPoint texCoordOffset = SkPoint::Make(atlasClip.fOutPos.x() - atlasClip.fMaskBounds.left(),
                                               atlasClip.fOutPos.y() - atlasClip.fMaskBounds.top());

        NonMSAAClipBlock::NonMSAAClipData data(
                analyticBounds,
                radiusPair,
                analyticClip.edgeSelectRect(),
                texCoordOffset,
                texMaskBounds,
                atlasClip.fAtlasTexture);
        if (fClipShader) {
            // For both an analytic clip and clip shader, we need to compose them together into
            // a single clipping root node.
            Blend(keyContext, builder, gatherer,
                  /* addBlendToKey= */ [&]() -> void {
                      AddFixedBlendMode(keyContext, builder, gatherer, SkBlendMode::kModulate);
                  },
                  /* addSrcToKey= */ [&]() -> void {
                      NonMSAAClipBlock::AddBlock(keyContext, builder, gatherer, data);
                  },
                  /* addDstToKey= */ [&]() -> void {
                      AddToKey(keyContext, builder, gatherer, fClipShader.get());
                  });
        } else {
            // Without a clip shader, the analytic clip can be the clipping root node.
            NonMSAAClipBlock::AddBlock(keyContext, builder, gatherer, data);
        }
    } else if (fClipShader) {
        // Since there's no analytic clip, the clipping root node can be fClipShader directly.
        AddToKey(keyContext, builder, gatherer, fClipShader.get());
    }
}

void PaintParams::toKey(const KeyContext& keyContext,
                        PaintParamsKeyBuilder* builder,
                        PipelineDataGatherer* gatherer) const {
    // Root Node 0 is the source color, which is the output of all effects post dithering
    this->handleDithering(keyContext, builder, gatherer);

    // Root Node 1 is the final blender
    std::optional<SkBlendMode> finalBlendMode = this->asFinalBlendMode();
    if (finalBlendMode) {
        if (!fDstReadRequired) {
            // With no shader blending, be as explicit as possible about the final blend
            AddFixedBlendMode(keyContext, builder, gatherer, *finalBlendMode);
        } else {
            // With shader blending, use AddBlendMode() to select the more universal blend functions
            // when possible. Technically we could always use a fixed blend mode but would then
            // over-generate when encountering certain classes of blends. This is most problematic
            // on devices that wouldn't support dual-source blending, so help them out by at least
            // not requiring lots of pipelines.
            AddBlendMode(keyContext, builder, gatherer, *finalBlendMode);
        }
    } else {
        AddToKey(keyContext, builder, gatherer, fFinalBlender.get());
    }

    // Optional Root Node 2 is the clip
    this->handleClipping(keyContext, builder, gatherer);
}

// TODO(b/330864257): Can be deleted once keys are determined by the Device draw.
void PaintParams::notifyImagesInUse(Recorder* recorder,
                                    DrawContext* drawContext) const {
    if (fShader) {
        NotifyImagesInUse(recorder, drawContext, fShader.get());
    }
    if (fPrimitiveBlender) {
        NotifyImagesInUse(recorder, drawContext, fPrimitiveBlender.get());
    }
    if (fColorFilter) {
        NotifyImagesInUse(recorder, drawContext, fColorFilter.get());
    }
    if (fFinalBlender) {
        NotifyImagesInUse(recorder, drawContext, fFinalBlender.get());
    }
    if (fClipShader) {
        NotifyImagesInUse(recorder, drawContext, fClipShader.get());
    }
}

} // namespace skgpu::graphite
