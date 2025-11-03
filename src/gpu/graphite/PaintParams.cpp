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
#include "src/core/SkImageInfoPriv.h"
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
    return p.imageShader() || (p.shader() && !as_SB(p.shader())->isConstant());
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

std::pair<const SkBlender*, SkBlendMode> get_final_blend(const SkBlender* blender) {
    if (!blender) {
        return {nullptr, SkBlendMode::kSrcOver};
    }

    auto optionalBlendMode = as_BB(blender)->asBlendMode();
    if (optionalBlendMode.has_value()) {
        return {nullptr, *optionalBlendMode};
    } else {
        return {blender, SkBlendMode::kSrc};
    }
}

Coverage get_renderer_coverage(Coverage coverage,
                               const SkShader* clipShader,
                               const NonMSAAClip& nonMSAAClip) {
    return (clipShader || !nonMSAAClip.isEmpty()) && coverage == Coverage::kNone ?
            Coverage::kSingleChannel : coverage;
}

SkEnumBitMask<DstUsage> get_dst_usage(const Caps* caps,
                                      TextureFormat targetFormat,
                                      const PaintParams& paint,
                                      Coverage rendererCoverage) {
    if (paint.finalBlender()) {
        return DstUsage::kDstReadRequired;
    }

    SkBlendMode finalBlendMode = paint.finalBlendMode();
    SkEnumBitMask<DstUsage> dstUsage =
            CanUseHardwareBlending(caps, targetFormat, finalBlendMode, rendererCoverage)
                            ? DstUsage::kNone
                            : DstUsage::kDstReadRequired;
    if (finalBlendMode > SkBlendMode::kLastCoeffMode) {
        dstUsage |= DstUsage::kAdvancedBlend;
    }
    return dstUsage;
}

} // anonymous namespace

PaintParams::PaintParams(const SkPaint& paint,
                         const SimpleImage* imageOverride,
                         const SkBlender* primitiveBlender,
                         bool skipColorXform,
                         bool ignoreShader)
        : fColor(paint.getColor4f())
        , fFinalBlend(get_final_blend(paint.getBlender()))
        , fShader(ignoreShader ? nullptr : paint.getShader())
        , fImageShader(imageOverride)
        , fColorFilter(paint.getColorFilter())
        , fPrimitiveBlender(primitiveBlender)
        , fSkipColorXform(skipColorXform)
        , fDither(paint.isDither()) {
    if (!fPrimitiveBlender) {
        // NOTE: We can still have an alpha-only fImageShader and still want to try simplifying the
        // paint's shader to a solid color for the alpha image's colorization.
        SkColor4f constantColor;
        if (fShader && as_SB(fShader)->isConstant(&constantColor)) {
            // The original fColor and `constantColor` are un-premul sRGB, but we need to preserve
            // the paint's alpha.
            float origA = fColor.fA;
            fColor = constantColor;
            fColor.fA *= origA;
            fShader = nullptr;
        }
        // We can't apply the color filter to the color if a shader modifies it, including when the
        // image shader is alpha-only. The image's per-pixel alpha modulates the paint color
        // *before* the color filter is evaluated.
        if (!fShader && !fImageShader && fColorFilter) {
            fColor = fColorFilter->filterColor4f(fColor,
                                                 sk_srgb_singleton(),
                                                 sk_srgb_singleton());
            fColorFilter = nullptr;
        }
    }
}

PaintParams::PaintParams(const SkPaint& paint,
                         const SkBlender* primitiveBlender,
                         bool skipColorXform,
                         bool ignoreShader)
        : PaintParams(paint,
                      /*imageOverride=*/nullptr,
                      primitiveBlender,
                      skipColorXform,
                      ignoreShader) {}

PaintParams::PaintParams(const SkPaint& paint, const SimpleImage& imageOverride, float xtraAlpha)
        : PaintParams(paint,
                      &imageOverride,
                      /*primitiveBlender=*/nullptr,
                      /*skipColorXform=*/false,
                      // For color images, the paint's original shader is ignored.
                      /*ignoreShader=*/!SkColorTypeIsAlphaOnly(imageOverride.fImage->colorType())) {
    // Multiply in the extra alpha that's allowed to be set on an ImageSetEntry. Accepting it here
    // avoids needing to modify the SkPaint providing the base color.
    fColor.fA *= xtraAlpha;
}

PaintParams::PaintParams(const SkColor4f& color, SkBlendMode finalBlendMode)
        : fColor(color)
        , fFinalBlend({nullptr, finalBlendMode})
        , fShader(nullptr)
        , fImageShader(nullptr)
        , fColorFilter(nullptr)
        , fPrimitiveBlender(nullptr)
        , fSkipColorXform(false)
        , fDither(false) {}

SkColor4f PaintParams::Color4fPrepForDst(SkColor4f srcColor, const SkColorInfo& dstColorInfo) {
    // xform from sRGB to the destination colorspace
    SkColorSpaceXformSteps steps(sk_srgb_singleton(),       kUnpremul_SkAlphaType,
                                 dstColorInfo.colorSpace(), kUnpremul_SkAlphaType);

    SkColor4f result = srcColor;
    steps.apply(result.vec());
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

ShadingParams::ShadingParams(const Caps* caps,
                             const PaintParams& paint,
                             const NonMSAAClip& nonMSAAClip,
                             const SkShader* clipShader,
                             Coverage coverage,
                             TextureFormat targetFormat)
        : fPaint(paint)
        , fNonMSAAClip(nonMSAAClip)
        , fClipShader(clipShader)
        , fRendererCoverage(get_renderer_coverage(coverage, fClipShader, fNonMSAAClip))
        , fTargetFormat(targetFormat)
        , fDstUsage(get_dst_usage(caps, fTargetFormat, paint, fRendererCoverage)) {}

bool ShadingParams::addPaintColorToKey(const KeyContext& keyContext) const {
    const auto& simpleImage = fPaint.imageShader();
    if (simpleImage) {
        // There is an implicit image shader, match handling of SkModifyPaintForDrawImageRect
        if (fPaint.shader()) {
            // Alpha-only images for drawImageRect() get colorized with the paint's shader. This
            // differs from alpha-only image shaders that might be encountered within an SkShader
            // graph, which get colorized by the paint's opaque color.
            SkASSERT(SkColorTypeIsAlphaOnly(simpleImage->fImage->colorType()));
            Blend(keyContext,
                  /* addBlendToKey */ [&] () -> void {
                      AddFixedBlendMode(keyContext, SkBlendMode::kDstIn);
                  },
                  /* addSrcToKey = */ [&] () -> void {
                      // Since colorization is handled here, disable paint color-colorization later.
                      AddToKey(keyContext.withExtraFlags(
                                       KeyGenFlags::kDisableAlphaOnlyImageColorization),
                               *simpleImage);
                  },
                  /* addDstToKey = */ [&] () -> void {
                      AddToKey(keyContext, fPaint.shader());
                  });
            return false; // Colorizing with an alpha-only texture probably isn't opaque
        } else {
            // Encode the image structure directly, which includes handling alpha-only images that
            // combine with the paint's color (RGB1) stored on `keyContext`.
            AddToKey(keyContext, *simpleImage);
            return simpleImage->fImage->isOpaque();
        }
    } else if (fPaint.shader()) {
        AddToKey(keyContext, fPaint.shader());
        return fPaint.shader()->isOpaque();
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
bool ShadingParams::handlePrimitiveColor(const KeyContext& keyContext) const {
    // If no primitive blending is required, simply add the paint color.
    if (!fPaint.primitiveBlender()) {
        return this->addPaintColorToKey(keyContext);
    }

    // If no color space conversion is required and the primitive blend mode is kDst, the src
    // branch of the blend does not matter and we can simply emit the primitive color.
    std::optional<SkBlendMode> primBlend = as_BB(fPaint.primitiveBlender())->asBlendMode();
    const bool canSkipBlendStep = fPaint.skipPrimitiveColorXform() &&
                                  primBlend == SkBlendMode::kDst;

    if (canSkipBlendStep) {
        AddPrimitiveColor(keyContext, fPaint.skipPrimitiveColorXform());
        return false;
    }

    bool srcIsOpaque = false;
    Blend(keyContext,
        /* addBlendToKey= */ [&] () -> void {
            AddToKey(keyContext, fPaint.primitiveBlender());
        },
        /* addSrcToKey= */ [&] () -> void {
            srcIsOpaque = this->addPaintColorToKey(keyContext);
        },
        /* addDstToKey= */ [&] () -> void {
            AddPrimitiveColor(keyContext, fPaint.skipPrimitiveColorXform());
        });
    if (primBlend.has_value() && srcIsOpaque) {
        // If the input paint/shader is opaque, the result is only opaque if the primitive blend
        // mode is kSrc or kSrcOver. All other modes can introduce transparency.
        return *primBlend == SkBlendMode::kSrc || *primBlend == SkBlendMode::kSrcOver;
    }

    // If the input was already transparent, or if it's a runtime/complex blend mode,
    // the result cannot be considered opaque.
    return false;
}

// Apply the paint's alpha value.
bool ShadingParams::handlePaintAlpha(const KeyContext& keyContext) const {
    if (!fPaint.shader() && !fPaint.imageShader() && !fPaint.primitiveBlender()) {
        // If there is no shader and no primitive blending the input to the colorFilter stage
        // is just the premultiplied paint color.
        SkPMColor4f paintColor = PaintParams::Color4fPrepForDst(fPaint.color(),
                                                                keyContext.dstColorInfo()).premul();
        SolidColorShaderBlock::AddBlock(keyContext, paintColor);
        return fPaint.color().isOpaque();
    }

    if (!fPaint.color().isOpaque()) {
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

bool ShadingParams::handleColorFilter(const KeyContext& keyContext) const {
    if (fPaint.colorFilter()) {
        bool srcIsOpaque = false;
        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    srcIsOpaque = this->handlePaintAlpha(keyContext);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddToKey(keyContext, fPaint.colorFilter());
                });
        return srcIsOpaque && fPaint.colorFilter()->isAlphaUnchanged();
    } else {
        return this->handlePaintAlpha(keyContext);
    }
}

bool ShadingParams::handleDithering(const KeyContext& keyContext) const {

#ifndef SK_IGNORE_GPU_DITHER
    SkColorType ct = keyContext.dstColorInfo().colorType();
    if (should_dither(fPaint, ct)) {
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

void ShadingParams::handleClipping(const KeyContext& keyContext) const {
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
                      AddToKey(keyContext, fClipShader);
                  });
        } else {
            // Without a clip shader, the analytic clip can be the clipping root node.
            NonMSAAClipBlock::AddBlock(keyContext, data);
        }
    } else if (fClipShader) {
        // Since there's no analytic clip, the clipping root node can be fClipShader directly.
        AddToKey(keyContext, fClipShader);
    }
}

std::optional<ShadingParams::Result> ShadingParams::toKey(const KeyContext& keyContext) const {
    // Root Node 0 is the source color, which is the output of all effects post dithering
    bool isOpaque = this->handleDithering(keyContext);

    // Root Node 1 is the final blender
    bool dependsOnDst = fRendererCoverage != Coverage::kNone;
    if (fPaint.finalBlender()) {
        AddToKey(keyContext, fPaint.finalBlender());
        // Cannot inspect runtime blenders to pessimistically assume they will always use the dst.
        dependsOnDst = true;
    } else {
        if (!(fDstUsage & DstUsage::kDstReadRequired)) {
            // With no shader blending, be as explicit as possible about the final blend
            AddFixedBlendMode(keyContext, fPaint.finalBlendMode());
        } else {
            // With shader blending, use AddBlendMode() to select the more universal blend functions
            // when possible. Technically we could always use a fixed blend mode but would then
            // over-generate when encountering certain classes of blends. This is most problematic
            // on devices that wouldn't support dual-source blending, so help them out by at least
            // not requiring lots of pipelines.
            AddBlendMode(keyContext, fPaint.finalBlendMode());
        }

        // Blend modes can be analyzed to determine if specific src colors still depend on the dst.
        dependsOnDst |= blendmode_depends_on_dst(fPaint.finalBlendMode(), isOpaque);
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
