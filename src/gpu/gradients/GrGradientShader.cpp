/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gradients/GrGradientShader.h"

#include "src/gpu/gradients/generated/GrClampedGradientEffect.h"
#include "src/gpu/gradients/generated/GrTiledGradientEffect.h"

#include "src/gpu/gradients/generated/GrTwoPointConicalGradientLayout.h"

#include "src/gpu/gradients/GrGradientBitmapCache.h"
#include "src/gpu/gradients/generated/GrUnrolledBinaryGradientColorizer.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/GrTextureEffect.h"

namespace GrGradientColorizer {

std::unique_ptr<GrFragmentProcessor> SingleInterval(const SkPMColor4f& start,
                                                    const SkPMColor4f& end) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform half4 start;
        uniform half4 end;
        half4 main(float2 coord) {
            // Clamping and/or wrapping was already handled by the parent shader so the output
            // color is a simple lerp.
            return mix(start, end, half(coord.x));
        }
    )");
    return GrSkSLFP::Make(effect, "SingleIntervalColorizer", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kNone,
                          "start", start,
                          "end", end);
}

std::unique_ptr<GrFragmentProcessor> DualInterval(const SkPMColor4f& c0,
                                                  const SkPMColor4f& c1,
                                                  const SkPMColor4f& c2,
                                                  const SkPMColor4f& c3,
                                                  float threshold) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform float4 scale01;
        uniform float4 bias01;
        uniform float4 scale23;
        uniform float4 bias23;
        uniform half threshold;

        half4 main(float2 coord) {
            half t = half(coord.x);

            float4 scale, bias;
            if (t < threshold) {
                scale = scale01;
                bias = bias01;
            } else {
                scale = scale23;
                bias = bias23;
            }

            return half4(t * scale + bias);
        }
    )");

    using sk4f = skvx::Vec<4, float>;

    // Derive scale and biases from the 4 colors and threshold
    auto vc0 = sk4f::Load(c0.vec());
    auto vc1 = sk4f::Load(c1.vec());
    auto scale01 = (vc1 - vc0) / threshold;
    // bias01 = c0

    auto vc2 = sk4f::Load(c2.vec());
    auto vc3 = sk4f::Load(c3.vec());
    auto scale23 = (vc3 - vc2) / (1 - threshold);
    auto bias23 = vc2 - threshold * scale23;

    return GrSkSLFP::Make(effect, "DualIntervalColorizer", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kNone,
                          "scale01", scale01,
                          "bias01", c0,
                          "scale23", scale23,
                          "bias23", bias23,
                          "threshold", threshold);
}

}  // namespace GrGradientColorizer

// Intervals smaller than this (that aren't hard stops) on low-precision-only devices force us to
// use the textured gradient
static const SkScalar kLowPrecisionIntervalLimit = 0.01f;

// Each cache entry costs 1K or 2K of RAM. Each bitmap will be 1x256 at either 32bpp or 64bpp.
static const int kMaxNumCachedGradientBitmaps = 32;
static const int kGradientTextureSize = 256;

// NOTE: signature takes raw pointers to the color/pos arrays and a count to make it easy for
// MakeColorizer to transparently take care of hard stops at the end points of the gradient.
static std::unique_ptr<GrFragmentProcessor> make_textured_colorizer(const SkPMColor4f* colors,
        const SkScalar* positions, int count, bool premul, const GrFPArgs& args) {
    static GrGradientBitmapCache gCache(kMaxNumCachedGradientBitmaps, kGradientTextureSize);

    // Use 8888 or F16, depending on the destination config.
    // TODO: Use 1010102 for opaque gradients, at least if destination is 1010102?
    SkColorType colorType = kRGBA_8888_SkColorType;
    if (GrColorTypeIsWiderThan(args.fDstColorInfo->colorType(), 8)) {
        auto f16Format = args.fContext->priv().caps()->getDefaultBackendFormat(
                GrColorType::kRGBA_F16, GrRenderable::kNo);
        if (f16Format.isValid()) {
            colorType = kRGBA_F16_SkColorType;
        }
    }
    SkAlphaType alphaType = premul ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;

    SkBitmap bitmap;
    gCache.getGradient(colors, positions, count, colorType, alphaType, &bitmap);
    SkASSERT(1 == bitmap.height() && SkIsPow2(bitmap.width()));
    SkASSERT(bitmap.isImmutable());

    auto view = std::get<0>(GrMakeCachedBitmapProxyView(args.fContext, bitmap, GrMipmapped::kNo));
    if (!view) {
        SkDebugf("Gradient won't draw. Could not create texture.");
        return nullptr;
    }

    auto m = SkMatrix::Scale(view.width(), 1.f);
    return GrTextureEffect::Make(std::move(view), alphaType, m, GrSamplerState::Filter::kLinear);
}

// Analyze the shader's color stops and positions and chooses an appropriate colorizer to represent
// the gradient.
static std::unique_ptr<GrFragmentProcessor> make_colorizer(const SkPMColor4f* colors,
        const SkScalar* positions, int count, bool premul, const GrFPArgs& args) {
    // If there are hard stops at the beginning or end, the first and/or last color should be
    // ignored by the colorizer since it should only be used in a clamped border color. By detecting
    // and removing these stops at the beginning, it makes optimizing the remaining color stops
    // simpler.

    // SkGradientShaderBase guarantees that pos[0] == 0 by adding a default value.
    bool bottomHardStop = SkScalarNearlyEqual(positions[0], positions[1]);
    // The same is true for pos[end] == 1
    bool topHardStop = SkScalarNearlyEqual(positions[count - 2], positions[count - 1]);

    int offset = 0;
    if (bottomHardStop) {
        offset += 1;
        count--;
    }
    if (topHardStop) {
        count--;
    }

    // Two remaining colors means a single interval from 0 to 1
    // (but it may have originally been a 3 or 4 color gradient with 1-2 hard stops at the ends)
    if (count == 2) {
        return GrGradientColorizer::SingleInterval(colors[offset], colors[offset + 1]);
    }

    // Do an early test for the texture fallback to skip all of the other tests for specific
    // analytic support of the gradient (and compatibility with the hardware), when it's definitely
    // impossible to use an analytic solution.
    bool tryAnalyticColorizer = count <= GrUnrolledBinaryGradientColorizer::kMaxColorCount;

    // The remaining analytic colorizers use scale*t+bias, and the scale/bias values can become
    // quite large when thresholds are close (but still outside the hardstop limit). If float isn't
    // 32-bit, output can be incorrect if the thresholds are too close together. However, the
    // analytic shaders are higher quality, so they can be used with lower precision hardware when
    // the thresholds are not ill-conditioned.
    const GrShaderCaps* caps = args.fContext->priv().caps()->shaderCaps();
    if (!caps->floatIs32Bits() && tryAnalyticColorizer) {
        // Could run into problems, check if thresholds are close together (with a limit of .01, so
        // that scales will be less than 100, which leaves 4 decimals of precision on 16-bit).
        for (int i = offset; i < count - 1; i++) {
            SkScalar dt = SkScalarAbs(positions[i] - positions[i + 1]);
            if (dt <= kLowPrecisionIntervalLimit && dt > SK_ScalarNearlyZero) {
                tryAnalyticColorizer = false;
                break;
            }
        }
    }

    if (tryAnalyticColorizer) {
        if (count == 3) {
            // Must be a dual interval gradient, where the middle point is at offset+1 and the two
            // intervals share the middle color stop.
            return GrGradientColorizer::DualInterval(colors[offset], colors[offset + 1],
                                                     colors[offset + 1], colors[offset + 2],
                                                     positions[offset + 1]);
        } else if (count == 4 && SkScalarNearlyEqual(positions[offset + 1],
                                                     positions[offset + 2])) {
            // Two separate intervals that join at the same threshold position
            return GrGradientColorizer::DualInterval(colors[offset], colors[offset + 1],
                                                     colors[offset + 2], colors[offset + 3],
                                                     positions[offset + 1]);
        }

        // The single and dual intervals are a specialized case of the unrolled binary search
        // colorizer which can analytically render gradients of up to 8 intervals (up to 9 or 16
        // colors depending on how many hard stops are inserted).
        std::unique_ptr<GrFragmentProcessor> unrolled = GrUnrolledBinaryGradientColorizer::Make(
                colors + offset, positions + offset, count);
        if (unrolled) {
            return unrolled;
        }
    }

    // Otherwise fall back to a rasterized gradient sampled by a texture, which can handle
    // arbitrary gradients (the only downside being sampling resolution).
    return make_textured_colorizer(colors + offset, positions + offset, count, premul, args);
}

// Combines the colorizer and layout with an appropriately configured top-level effect based on the
// gradient's tile mode
static std::unique_ptr<GrFragmentProcessor> make_gradient(const SkGradientShaderBase& shader,
        const GrFPArgs& args, std::unique_ptr<GrFragmentProcessor> layout) {
    // No shader is possible if a layout couldn't be created, e.g. a layout-specific Make() returned
    // null.
    if (layout == nullptr) {
        return nullptr;
    }

    // Convert all colors into destination space and into SkPMColor4fs, and handle
    // premul issues depending on the interpolation mode
    bool inputPremul = shader.getGradFlags() & SkGradientShader::kInterpolateColorsInPremul_Flag;
    bool allOpaque = true;
    SkAutoSTMalloc<4, SkPMColor4f> colors(shader.fColorCount);
    SkColor4fXformer xformedColors(shader.fOrigColors4f, shader.fColorCount,
                                   shader.fColorSpace.get(), args.fDstColorInfo->colorSpace());
    for (int i = 0; i < shader.fColorCount; i++) {
        const SkColor4f& upmColor = xformedColors.fColors[i];
        colors[i] = inputPremul ? upmColor.premul()
                                : SkPMColor4f{ upmColor.fR, upmColor.fG, upmColor.fB, upmColor.fA };
        if (allOpaque && !SkScalarNearlyEqual(colors[i].fA, 1.0)) {
            allOpaque = false;
        }
    }

    // SkGradientShader stores positions implicitly when they are evenly spaced, but the getPos()
    // implementation performs a branch for every position index. Since the shader conversion
    // requires lots of position tests, calculate all of the positions up front if needed.
    SkTArray<SkScalar, true> implicitPos;
    SkScalar* positions;
    if (shader.fOrigPos) {
        positions = shader.fOrigPos;
    } else {
        implicitPos.reserve_back(shader.fColorCount);
        SkScalar posScale = SK_Scalar1 / (shader.fColorCount - 1);
        for (int i = 0 ; i < shader.fColorCount; i++) {
            implicitPos.push_back(SkIntToScalar(i) * posScale);
        }
        positions = implicitPos.begin();
    }

    // All gradients are colorized the same way, regardless of layout
    std::unique_ptr<GrFragmentProcessor> colorizer = make_colorizer(
            colors.get(), positions, shader.fColorCount, inputPremul, args);
    if (colorizer == nullptr) {
        return nullptr;
    }

    // The top-level effect has to export premul colors, but under certain conditions it doesn't
    // need to do anything to achieve that: i.e. its interpolating already premul colors
    // (inputPremul) or all the colors have a = 1, in which case premul is a no op. Note that this
    // allOpaque check is more permissive than SkGradientShaderBase's isOpaque(), since we can
    // optimize away the make-premul op for two point conical gradients (which report false for
    // isOpaque).
    bool makePremul = !inputPremul && !allOpaque;

    // All tile modes are supported (unless something was added to SkShader)
    std::unique_ptr<GrFragmentProcessor> gradient;
    switch(shader.getTileMode()) {
        case SkTileMode::kRepeat:
            gradient = GrTiledGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                   /* mirror */ false, makePremul, allOpaque);
            break;
        case SkTileMode::kMirror:
            gradient = GrTiledGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                   /* mirror */ true, makePremul, allOpaque);
            break;
        case SkTileMode::kClamp:
            // For the clamped mode, the border colors are the first and last colors, corresponding
            // to t=0 and t=1, because SkGradientShaderBase enforces that by adding color stops as
            // appropriate. If there is a hard stop, this grabs the expected outer colors for the
            // border.
            gradient = GrClampedGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                     colors[0], colors[shader.fColorCount - 1],
                                                     makePremul, allOpaque);
            break;
        case SkTileMode::kDecal:
            // Even if the gradient colors are opaque, the decal borders are transparent so
            // disable that optimization
            gradient = GrClampedGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                     SK_PMColor4fTRANSPARENT,
                                                     SK_PMColor4fTRANSPARENT,
                                                     makePremul, /* colorsAreOpaque */ false);
            break;
    }

    if (gradient == nullptr) {
        // Unexpected tile mode
        return nullptr;
    }
    if (args.fInputColorIsOpaque) {
        // If the input alpha is known to be 1, we don't need to take the kSrcIn path. This is
        // just an optimization. However, we can't just return 'gradient' here. We need to actually
        // inhibit the coverage-as-alpha optimization, or we'll fail to incorporate AA correctly.
        // The OverrideInput FP happens to do that, so wrap our fp in one of those. The gradient FP
        // doesn't actually use the input color at all, so the overridden input is irrelevant.
        return GrFragmentProcessor::OverrideInput(std::move(gradient), SK_PMColor4fWHITE, false);
    }
    return GrFragmentProcessor::MulChildByInputAlpha(std::move(gradient));
}

namespace GrGradientShader {

static std::unique_ptr<GrFragmentProcessor> apply_matrix(const SkGradientShaderBase& gradient,
                                                         const GrFPArgs& args,
                                                         std::unique_ptr<GrFragmentProcessor> fp) {
    SkMatrix matrix;
    if (!gradient.totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    matrix.postConcat(gradient.getGradientMatrix());
    return GrMatrixEffect::Make(matrix, std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> MakeLinear(const SkLinearGradient& shader,
                                                const GrFPArgs& args) {
    // We add a tiny delta to t. When gradient stops are set up so that a hard stop in a vertically
    // or horizontally oriented gradient falls exactly at a column or row of pixel centers we can
    // we can get slightly different interpolated t values along the column/row. By adding the delta
    // we will consistently get the color to the "right" of the stop. Of course if the hard stop
    // falls at X.5 - delta then we still could get inconsistent results, but that is much less
    // likely. crbug.com/938592
    // If/when we add filtering of the gradient this can be removed.
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        half4 main(float2 coord) {
            return half4(half(coord.x) + 0.00001, 1, 0, 0); // y = 1 for always valid
        }
    )");
    // The linear gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(effect, "LinearLayout", /*inputFP=*/nullptr,
                             GrSkSLFP::OptFlags::kPreservesOpaqueInput);
    return make_gradient(shader, args, apply_matrix(shader, args, std::move(fp)));
}

std::unique_ptr<GrFragmentProcessor> MakeRadial(const SkRadialGradient& shader,
                                                const GrFPArgs& args) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        half4 main(float2 coord) {
            return half4(half(length(coord)), 1, 0, 0); // y = 1 for always valid
        }
    )");
    // The radial gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(effect, "RadialLayout", /*inputFP=*/nullptr,
                             GrSkSLFP::OptFlags::kPreservesOpaqueInput);
    return make_gradient(shader,args, apply_matrix(shader, args, std::move(fp)));
}

std::unique_ptr<GrFragmentProcessor> MakeSweep(const SkSweepGradient& shader,
                                               const GrFPArgs& args) {
    // On some devices they incorrectly implement atan2(y,x) as atan(y/x). In actuality it is
    // atan2(y,x) = 2 * atan(y / (sqrt(x^2 + y^2) + x)). So to work around this we pass in (sqrt(x^2
    // + y^2) + x) as the second parameter to atan2 in these cases. We let the device handle the
    // undefined behavior of the second paramenter being 0 instead of doing the divide ourselves and
    // using atan instead.
    int useAtanWorkaround =
            args.fContext->priv().caps()->shaderCaps()->atan2ImplementedAsAtanYOverX();
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform half bias;
        uniform half scale;
        uniform int useAtanWorkaround;  // specialized

        half4 main(float2 coord) {
            half angle = bool(useAtanWorkaround)
                    ? half(2 * atan(-coord.y, length(coord) - coord.x))
                    : half(atan(-coord.y, -coord.x));

            // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
            half t = (angle * 0.1591549430918 + 0.5 + bias) * scale;
            return half4(t, 1, 0, 0); // y = 1 for always valid
        }
    )");
    // The sweep gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(effect, "SweepLayout", /*inputFP=*/nullptr,
                             GrSkSLFP::OptFlags::kPreservesOpaqueInput,
                             "bias", shader.getTBias(),
                             "scale", shader.getTScale(),
                             "useAtanWorkaround", GrSkSLFP::Specialize(useAtanWorkaround));
    return make_gradient(shader,args, apply_matrix(shader, args, std::move(fp)));
}

std::unique_ptr<GrFragmentProcessor> MakeConical(const SkTwoPointConicalGradient& shader,
                                                 const GrFPArgs& args) {
    return make_gradient(shader, args, GrTwoPointConicalGradientLayout::Make(shader, args));
}

#if GR_TEST_UTILS
RandomParams::RandomParams(SkRandom* random) {
    // Set color count to min of 2 so that we don't trigger the const color optimization and make
    // a non-gradient processor.
    fColorCount = random->nextRangeU(2, kMaxRandomGradientColors);
    fUseColors4f = random->nextBool();

    // if one color, omit stops, otherwise randomly decide whether or not to
    if (fColorCount == 1 || (fColorCount >= 2 && random->nextBool())) {
        fStops = nullptr;
    } else {
        fStops = fStopStorage;
    }

    // if using SkColor4f, attach a random (possibly null) color space (with linear gamma)
    if (fUseColors4f) {
        fColorSpace = GrTest::TestColorSpace(random);
    }

    SkScalar stop = 0.f;
    for (int i = 0; i < fColorCount; ++i) {
        if (fUseColors4f) {
            fColors4f[i].fR = random->nextUScalar1();
            fColors4f[i].fG = random->nextUScalar1();
            fColors4f[i].fB = random->nextUScalar1();
            fColors4f[i].fA = random->nextUScalar1();
        } else {
            fColors[i] = random->nextU();
        }
        if (fStops) {
            fStops[i] = stop;
            stop = i < fColorCount - 1 ? stop + random->nextUScalar1() * (1.f - stop) : 1.f;
        }
    }
    fTileMode = static_cast<SkTileMode>(random->nextULessThan(kSkTileModeCount));
}
#endif

}  // namespace GrGradientShader
