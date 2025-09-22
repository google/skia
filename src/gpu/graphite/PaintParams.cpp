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
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"

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

bool blendmode_depends_on_dst(SkBlendMode blendMode, bool srcIsOpaque) {
    if (blendMode == SkBlendMode::kSrc || blendMode == SkBlendMode::kClear) {
        // src and clear blending never depends on dst
        return false;
    }

    if (blendMode == SkBlendMode::kSrcOver || blendMode == SkBlendMode::kDstOut) {
        // src-over depends on dst if src is transparent (a != 1)
        // dst-out simplifies to kClear if a == 1
        return !srcIsOpaque;
    }

    return true;
}

std::optional<SkBlendMode> get_final_blendmode(SkBlender* blender) {
    return blender ? as_BB(blender)->asBlendMode() : SkBlendMode::kSrcOver;
}

Coverage get_renderer_coverage(Coverage coverage,
                               SkShader* clipShader,
                               const NonMSAAClip& nonMSAAClip) {
    return (clipShader || !nonMSAAClip.isEmpty()) && coverage == Coverage::kNone ?
            Coverage::kSingleChannel : coverage;
}

SkEnumBitMask<DstUsage> get_dst_usage(const Caps* caps,
                                      TextureFormat targetFormat,
                                      std::optional<SkBlendMode> finalBlendMode,
                                      Coverage rendererCoverage) {
    SkEnumBitMask<DstUsage> dstUsage =
            CanUseHardwareBlending(caps, targetFormat, finalBlendMode, rendererCoverage)
                            ? DstUsage::kNone
                            : DstUsage::kDstReadRequired;
    if (finalBlendMode.has_value() && finalBlendMode.value() > SkBlendMode::kLastCoeffMode) {
        dstUsage |= DstUsage::kAdvancedBlend;
    }
    return dstUsage;
}

} // anonymous namespace

PaintParams::PaintParams(const Caps* caps,
                         const SkPaint& paint,
                         sk_sp<SkBlender> primitiveBlender,
                         const NonMSAAClip& nonMSAAClip,
                         sk_sp<SkShader> clipShader,
                         Coverage coverage,
                         TextureFormat targetFormat,
                         bool skipColorXform)
        : fColor(paint.getColor4f())
        , fFinalBlender(paint.refBlender())
        , fFinalBlendMode(get_final_blendmode(fFinalBlender.get()))
        , fShader(paint.refShader())
        , fColorFilter(paint.refColorFilter())
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fNonMSAAClip(nonMSAAClip)
        , fClipShader(std::move(clipShader))
        , fRendererCoverage(get_renderer_coverage(coverage, fClipShader.get(), fNonMSAAClip))
        , fTargetFormat(targetFormat)
        , fSkipColorXform(skipColorXform)
        , fDither(paint.isDither())
        , fDstUsage(get_dst_usage(caps, fTargetFormat, fFinalBlendMode, fRendererCoverage)) {
    if (!fPrimitiveBlender) {
        SkColor4f constantColor;   // if filled in, will be un-premul sRGB
        // fColor is un-premul sRGB
        if (fShader && as_SB(fShader)->isConstant(&constantColor)) {
            float origA = fColor.fA;
            fColor = constantColor;
            fColor.fA *= origA;
            fShader = nullptr;
        }
        if (!fShader && fColorFilter) {
            fColor = fColorFilter->filterColor4f(fColor,
                                                 sk_srgb_singleton(),
                                                 sk_srgb_singleton());
            fColorFilter = nullptr;
        }
    }
}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

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

void AddFixedBlendMode(const KeyContext& keyContext, SkBlendMode bm) {
    SkASSERT(bm <= SkBlendMode::kLastMode);
    BuiltInCodeSnippetID id = static_cast<BuiltInCodeSnippetID>(kFixedBlendIDOffset +
                                                                static_cast<int>(bm));
    keyContext.paintParamsKeyBuilder()->addBlock(id);
}

void AddBlendMode(const KeyContext& keyContext, SkBlendMode bm) {
    // For non-fixed blends, coefficient blend modes are combined into the same shader snippet.
    // The same goes for the HSLC advanced blends. The remaining advanced blends are fairly unique
    // in their implementations. To avoid having to compile all of their SkSL, they are treated as
    // fixed blend modes.
    SkSpan<const float> coeffs = skgpu::GetPorterDuffBlendConstants(bm);
    if (!coeffs.empty()) {
        PorterDuffBlenderBlock::AddBlock(keyContext, coeffs);
    } else if (bm >= SkBlendMode::kHue) {
        ReducedBlendModeInfo blendInfo = GetReducedBlendModeInfo(bm);
        HSLCBlenderBlock::AddBlock(keyContext, blendInfo.fUniformData);
    } else {
        AddFixedBlendMode(keyContext, bm);
    }
}

void AddDitherBlock(const KeyContext& keyContext, SkColorType ct) {
    static const SkBitmap gLUT = skgpu::MakeDitherLUT();

    sk_sp<TextureProxy> proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(), gLUT,
                                                                "DitherLUT");
    if (keyContext.recorder() && !proxy) {
        SKGPU_LOG_W("Couldn't create dither shader's LUT");
        keyContext.paintParamsKeyBuilder()->addBlock(BuiltInCodeSnippetID::kPriorOutput);
        return;
    }

    DitherShaderBlock::DitherData data(skgpu::DitherRangeForConfig(ct), std::move(proxy));

    DitherShaderBlock::AddBlock(keyContext, data);
}

bool PaintParams::addPaintColorToKey(const KeyContext& keyContext) const {
    if (fShader) {
        AddToKey(keyContext, fShader.get());
        return fShader->isOpaque();
    } else {
        RGBPaintColorBlock::AddBlock(keyContext);
        return true; // rgb1, always opaque
    }
}

/**
 * Primitive blend blocks are used to blend either the paint color or the output of another shader
 * with a primitive color emitted by certain draw geometry calls (drawVertices, drawAtlas, etc.).
 * Dst: primitiveColor Src: Paint color/shader output
 */
bool PaintParams::handlePrimitiveColor(const KeyContext& keyContext) const {
    /**
     * If no primitive blending is required, simply add the paint color.
    */
    if (!fPrimitiveBlender) {
        return this->addPaintColorToKey(keyContext);
    }

    /**
     * If no color space conversion is required and the primitive blend mode is kDst, the src
     * branch of the blend does not matter and we can simply emit the primitive color.
    */
    const bool canSkipBlendStep =
        fSkipColorXform &&
        as_BB(fPrimitiveBlender.get())->asBlendMode().has_value() &&
        as_BB(fPrimitiveBlender.get())->asBlendMode().value() == SkBlendMode::kDst;

    if (canSkipBlendStep) {
        AddPrimitiveColor(keyContext, fSkipColorXform);
        return false;
    }

    bool srcIsOpaque = false;
    Blend(keyContext,
        /* addBlendToKey= */ [&] () -> void {
            AddToKey(keyContext, fPrimitiveBlender.get());
        },
        /* addSrcToKey= */ [&] () -> void {
            srcIsOpaque = this->addPaintColorToKey(keyContext);
        },
        /* addDstToKey= */ [&] () -> void {
            AddPrimitiveColor(keyContext, fSkipColorXform);
        });
    std::optional<SkBlendMode> primBlend = as_BB(fPrimitiveBlender.get())->asBlendMode();
    if (primBlend.has_value() && srcIsOpaque) {
        // If the input paint/shader is opaque, the result is only opaque if the primitive blend
        // mode is kSrc or kSrcOver. All other modes can introduce transparency.
        return primBlend.value() == SkBlendMode::kSrc || primBlend.value() == SkBlendMode::kSrcOver;
    }

    // If the input was already transparent, or if it's a runtime/complex blend mode,
    // the result cannot be considered opaque.
    return false;
}

// Apply the paint's alpha value.
bool PaintParams::handlePaintAlpha(const KeyContext& keyContext) const {
    if (!fShader && !fPrimitiveBlender) {
        // If there is no shader and no primitive blending the input to the colorFilter stage
        // is just the premultiplied paint color.
        SkPMColor4f paintColor = PaintParams::Color4fPrepForDst(fColor,
                                                                keyContext.dstColorInfo()).premul();
        SolidColorShaderBlock::AddBlock(keyContext, paintColor);
        return fColor.isOpaque();
    }

    if (!fColor.isOpaque()) {
        Blend(keyContext,
              /* addBlendToKey= */ [&] () -> void {
                  AddFixedBlendMode(keyContext, SkBlendMode::kSrcIn);
              },
              /* addSrcToKey= */ [&]() -> void {
                  this->handlePrimitiveColor(keyContext);
              },
              /* addDstToKey= */ [&]() -> void {
                  AlphaOnlyPaintColorBlock::AddBlock(keyContext);
              });
        // The result is guaranteed to be non-opaque because we're blending with fColor's alpha.
        return false;
    } else {
        return this->handlePrimitiveColor(keyContext);
    }
}

bool PaintParams::handleColorFilter(const KeyContext& keyContext) const {
    if (fColorFilter) {
        bool srcIsOpaque = false;
        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    srcIsOpaque = this->handlePaintAlpha(keyContext);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddToKey(keyContext, fColorFilter.get());
                });
        return srcIsOpaque && fColorFilter->isAlphaUnchanged();
    } else {
        return this->handlePaintAlpha(keyContext);
    }
}

bool PaintParams::handleDithering(const KeyContext& keyContext) const {

#ifndef SK_IGNORE_GPU_DITHER
    SkColorType ct = keyContext.dstColorInfo().colorType();
    if (should_dither(*this, ct)) {
        bool srcIsOpaque = false;
        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    srcIsOpaque = this->handleColorFilter(keyContext);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddDitherBlock(keyContext, ct);
                });
        return srcIsOpaque;
    } else
#endif
    {
        return this->handleColorFilter(keyContext);
    }
}

void PaintParams::handleClipping(const KeyContext& keyContext) const {
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
            Blend(keyContext,
                  /* addBlendToKey= */ [&]() -> void {
                      AddFixedBlendMode(keyContext, SkBlendMode::kModulate);
                  },
                  /* addSrcToKey= */ [&]() -> void {
                      NonMSAAClipBlock::AddBlock(keyContext, data);
                  },
                  /* addDstToKey= */ [&]() -> void {
                      AddToKey(keyContext, fClipShader.get());
                  });
        } else {
            // Without a clip shader, the analytic clip can be the clipping root node.
            NonMSAAClipBlock::AddBlock(keyContext, data);
        }
    } else if (fClipShader) {
        // Since there's no analytic clip, the clipping root node can be fClipShader directly.
        AddToKey(keyContext, fClipShader.get());
    }
}

std::optional<PaintParams::Result> PaintParams::toKey(const KeyContext& keyContext) const {
    // Root Node 0 is the source color, which is the output of all effects post dithering
    bool isOpaque = this->handleDithering(keyContext);

    // Root Node 1 is the final blender
    bool dependsOnDst = fRendererCoverage != Coverage::kNone;
    if (fFinalBlendMode.has_value()) {
        if (!(fDstUsage & DstUsage::kDstReadRequired)) {
            // With no shader blending, be as explicit as possible about the final blend
            AddFixedBlendMode(keyContext, fFinalBlendMode.value());
        } else {
            // With shader blending, use AddBlendMode() to select the more universal blend functions
            // when possible. Technically we could always use a fixed blend mode but would then
            // over-generate when encountering certain classes of blends. This is most problematic
            // on devices that wouldn't support dual-source blending, so help them out by at least
            // not requiring lots of pipelines.
            AddBlendMode(keyContext, fFinalBlendMode.value());
        }

        // Blend modes can be analyzed to determine if specific src colors still depend on the dst.
        dependsOnDst |= blendmode_depends_on_dst(fFinalBlendMode.value(), isOpaque);
    } else {
        AddToKey(keyContext, fFinalBlender.get());
        // Cannot inspect runtime blenders to pessimistically assume they will always use the dst.
        dependsOnDst = true;
    }

    // Optional Root Node 2 is the clip
    this->handleClipping(keyContext);

    UniquePaintParamsID paintID =
            keyContext.recorder()->priv().shaderCodeDictionary()->findOrCreate(
                    keyContext.paintParamsKeyBuilder());

    if (!paintID.isValid()) {
        return {};
    } else {
        return Result{paintID,
                      fDstUsage | (dependsOnDst ? DstUsage::kDependsOnDst : DstUsage::kNone)};
    }
}

} // namespace skgpu::graphite
