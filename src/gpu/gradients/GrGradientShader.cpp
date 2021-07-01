/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gradients/GrGradientShader.h"

#include "src/gpu/gradients/GrGradientBitmapCache.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/GrTextureEffect.h"

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


static std::unique_ptr<GrFragmentProcessor> make_single_interval_colorizer(const SkPMColor4f& start,
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

static std::unique_ptr<GrFragmentProcessor> make_dual_interval_colorizer(const SkPMColor4f& c0,
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

static constexpr int kMaxUnrolledColorCount    = 16;
static constexpr int kMaxUnrolledIntervalCount = 8;

static std::unique_ptr<GrFragmentProcessor> make_unrolled_colorizer(int intervalCount,
                                                                    const SkPMColor4f* scale,
                                                                    const SkPMColor4f* bias,
                                                                    SkRect thresholds1_7,
                                                                    SkRect thresholds9_13) {
    SkASSERT(intervalCount >= 1 && intervalCount <= 8);

    static SkOnce                 once[kMaxUnrolledIntervalCount];
    static sk_sp<SkRuntimeEffect> effects[kMaxUnrolledIntervalCount];

    once[intervalCount - 1]([intervalCount] {
        SkString sksl;

        // The 7 threshold positions that define the boundaries of the 8 intervals (excluding t = 0,
        // and t = 1) are packed into two half4s instead of having up to 7 separate scalar uniforms.
        // For low interval counts, the extra components are ignored in the shader, but the uniform
        // simplification is worth it. It is assumed thresholds are provided in increasing value,
        // mapped as:
        //  - thresholds1_7.x = boundary between (0,1) and (2,3) -> 1_2
        //  -              .y = boundary between (2,3) and (4,5) -> 3_4
        //  -              .z = boundary between (4,5) and (6,7) -> 5_6
        //  -              .w = boundary between (6,7) and (8,9) -> 7_8
        //  - thresholds9_13.x = boundary between (8,9) and (10,11) -> 9_10
        //  -               .y = boundary between (10,11) and (12,13) -> 11_12
        //  -               .z = boundary between (12,13) and (14,15) -> 13_14
        //  -               .w = unused
        sksl.append("uniform half4 thresholds1_7, thresholds9_13;");

        // With the current hardstop detection threshold of 0.00024, the maximum scale and bias
        // values will be on the order of 4k (since they divide by dt). That is well outside the
        // precision capabilities of half floats, which can lead to inaccurate gradient calculations
        for (int i = 0; i < intervalCount; ++i) {
            sksl.appendf("uniform float4 scale%d_%d;", 2 * i, 2 * i + 1);
            sksl.appendf("uniform float4 bias%d_%d;", 2 * i, 2 * i + 1);
        }

        sksl.append("half4 main(float2 coord) {");
        sksl.append("  half t = half(coord.x);");
        sksl.append("  float4 scale, bias;");

        // To ensure that the code below always compiles, inject local variables with the names of
        // the uniforms that we *didn't* emit above. These will all end up unused and removed.
        for (int i = intervalCount; i < kMaxUnrolledIntervalCount; i++) {
            sksl.appendf("float4 scale%d_%d, bias%d_%d;", i * 2, i * 2 + 1, i * 2, i * 2 + 1);
        }

        sksl.appendf(R"(
            // Explicit binary search for the proper interval that t falls within. The interval
            // count checks are constant expressions, which are then optimized to the minimal number
            // of branches for the specific interval count.

            // thresholds1_7.w is mid point for intervals (0,7) and (8,15)
            if (%d <= 4 || t < thresholds1_7.w) {
                // thresholds1_7.y is mid point for intervals (0,3) and (4,7)
                if (%d <= 2 || t < thresholds1_7.y) {
                    // thresholds1_7.x is mid point for intervals (0,1) and (2,3)
                    if (%d <= 1 || t < thresholds1_7.x) {
                        scale = scale0_1;
                        bias = bias0_1;
                    } else {
                        scale = scale2_3;
                        bias = bias2_3;
                    }
                } else {
                    // thresholds1_7.z is mid point for intervals (4,5) and (6,7)
                    if (%d <= 3 || t < thresholds1_7.z) {
                        scale = scale4_5;
                        bias = bias4_5;
                    } else {
                        scale = scale6_7;
                        bias = bias6_7;
                    }
                }
            } else {
                // thresholds9_13.y is mid point for intervals (8,11) and (12,15)
                if (%d <= 6 || t < thresholds9_13.y) {
                    // thresholds9_13.x is mid point for intervals (8,9) and (10,11)
                    if (%d <= 5 || t < thresholds9_13.x) {
                        // interval 8-9
                        scale = scale8_9;
                        bias = bias8_9;
                    } else {
                        // interval 10-11
                        scale = scale10_11;
                        bias = bias10_11;
                    }
                } else {
                    // thresholds9_13.z is mid point for intervals (12,13) and (14,15)
                    if (%d <= 7 || t < thresholds9_13.z) {
                        // interval 12-13
                        scale = scale12_13;
                        bias = bias12_13;
                    } else {
                        // interval 14-15
                        scale = scale14_15;
                        bias = bias14_15;
                    }
                }
            }

        )", intervalCount, intervalCount, intervalCount, intervalCount, intervalCount,
            intervalCount, intervalCount);

        sksl.append("return half4(t * scale + bias); }");

        auto result = SkRuntimeEffect::MakeForShader(std::move(sksl));
        SkASSERTF(result.effect, "%s", result.errorText.c_str());
        effects[intervalCount - 1] = std::move(result.effect);
    });

#define ADD_STOP(N, suffix) \
    "scale" #suffix, GrSkSLFP::When(intervalCount > N, scale[N]), \
    "bias"  #suffix, GrSkSLFP::When(intervalCount > N, bias[N])

    return GrSkSLFP::Make(effects[intervalCount - 1], "UnrolledBinaryColorizer",
                          /*inputFP=*/nullptr, GrSkSLFP::OptFlags::kNone,
                          "thresholds1_7", thresholds1_7,
                          "thresholds9_13", thresholds9_13,
                          "scale0_1", scale[0], "bias0_1", bias[0],
                          ADD_STOP(1, 2_3),
                          ADD_STOP(2, 4_5),
                          ADD_STOP(3, 6_7),
                          ADD_STOP(4, 8_9),
                          ADD_STOP(5, 10_11),
                          ADD_STOP(6, 12_13),
                          ADD_STOP(7, 14_15));
#undef ADD_STOP
}

static std::unique_ptr<GrFragmentProcessor> make_unrolled_binary_colorizer(
        const SkPMColor4f* colors, const SkScalar* positions, int count) {
    // Depending on how the positions resolve into hard stops or regular stops, the number of
    // intervals specified by the number of colors/positions can change. For instance, a plain
    // 3 color gradient is two intervals, but a 4 color gradient with a hard stop is also
    // two intervals. At the most extreme end, an 8 interval gradient made entirely of hard
    // stops has 16 colors.

    if (count > kMaxUnrolledColorCount) {
        // Definitely cannot represent this gradient configuration
        return nullptr;
    }

    // The raster implementation also uses scales and biases, but since they must be calculated
    // after the dst color space is applied, it limits our ability to cache their values.
    SkPMColor4f scales[kMaxUnrolledIntervalCount];
    SkPMColor4f biases[kMaxUnrolledIntervalCount];
    SkScalar thresholds[kMaxUnrolledIntervalCount] = { 0 };

    int intervalCount = 0;

    for (int i = 0; i < count - 1; i++) {
        if (intervalCount >= kMaxUnrolledIntervalCount) {
            // Already reached kMaxUnrolledIntervalCount, and haven't run out of color stops so this
            // gradient cannot be represented by this shader.
            return nullptr;
        }

        SkScalar t0 = positions[i];
        SkScalar t1 = positions[i + 1];
        SkScalar dt = t1 - t0;
        // If the interval is empty, skip to the next interval. This will automatically create
        // distinct hard stop intervals as needed. It also protects against malformed gradients
        // that have repeated hard stops at the very beginning that are effectively unreachable.
        if (SkScalarNearlyZero(dt)) {
            continue;
        }

        auto c0 = Sk4f::Load(colors[i].vec());
        auto c1 = Sk4f::Load(colors[i + 1].vec());

        auto scale = (c1 - c0) / dt;
        auto bias = c0 - t0 * scale;

        scale.store(scales + intervalCount);
        bias.store(biases + intervalCount);
        thresholds[intervalCount] = t1;
        intervalCount++;
    }

    SkRect thresholds1_7  = {thresholds[0], thresholds[1], thresholds[2], thresholds[3]},
           thresholds9_13 = {thresholds[4], thresholds[5], thresholds[6], 0.0};

    return make_unrolled_colorizer(intervalCount, scales, biases, thresholds1_7, thresholds9_13);
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
        return make_single_interval_colorizer(colors[offset], colors[offset + 1]);
    }

    // Do an early test for the texture fallback to skip all of the other tests for specific
    // analytic support of the gradient (and compatibility with the hardware), when it's definitely
    // impossible to use an analytic solution.
    bool tryAnalyticColorizer = count <= kMaxUnrolledColorCount;

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
            return make_dual_interval_colorizer(colors[offset], colors[offset + 1],
                                                colors[offset + 1], colors[offset + 2],
                                                positions[offset + 1]);
        } else if (count == 4 && SkScalarNearlyEqual(positions[offset + 1],
                                                     positions[offset + 2])) {
            // Two separate intervals that join at the same threshold position
            return make_dual_interval_colorizer(colors[offset], colors[offset + 1],
                                                colors[offset + 2], colors[offset + 3],
                                                positions[offset + 1]);
        }

        // The single and dual intervals are a specialized case of the unrolled binary search
        // colorizer which can analytically render gradients of up to 8 intervals (up to 9 or 16
        // colors depending on how many hard stops are inserted).
        std::unique_ptr<GrFragmentProcessor> unrolled =
                make_unrolled_binary_colorizer(colors + offset, positions + offset, count);
        if (unrolled) {
            return unrolled;
        }
    }

    // Otherwise fall back to a rasterized gradient sampled by a texture, which can handle
    // arbitrary gradients (the only downside being sampling resolution).
    return make_textured_colorizer(colors + offset, positions + offset, count, premul, args);
}

// This top-level effect implements clamping on the layout coordinate and requires specifying the
// border colors that are used when outside the clamped boundary. Gradients with the
// SkTileMode::kClamp should use the colors at their first and last stop (after adding default stops
// for t=0,t=1) as the border color. This will automatically replicate the edge color, even when
// there is a hard stop.
//
// The SkTileMode::kDecal can be produced by specifying transparent black as the border colors,
// regardless of the gradient's stop colors.
static std::unique_ptr<GrFragmentProcessor> make_clamped_gradient(
        std::unique_ptr<GrFragmentProcessor> colorizer,
        std::unique_ptr<GrFragmentProcessor> gradLayout,
        SkPMColor4f leftBorderColor,
        SkPMColor4f rightBorderColor,
        bool makePremul,
        bool colorsAreOpaque) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform shader colorizer;
        uniform shader gradLayout;

        uniform half4 leftBorderColor;  // t < 0.0
        uniform half4 rightBorderColor; // t > 1.0

        uniform int makePremul;              // specialized
        uniform int layoutPreservesOpacity;  // specialized

        half4 main(float2 coord) {
            half4 t = sample(gradLayout, coord);
            half4 outColor;

            // If t.x is below 0, use the left border color without invoking the child processor.
            // If any t.x is above 1, use the right border color. Otherwise, t is in the [0, 1]
            // range assumed by the colorizer FP, so delegate to the child processor.
            if (!bool(layoutPreservesOpacity) && t.y < 0) {
                // layout has rejected this fragment (rely on sksl to remove this branch if the
                // layout FP preserves opacity is false)
                outColor = half4(0);
            } else if (t.x < 0) {
                outColor = leftBorderColor;
            } else if (t.x > 1.0) {
                outColor = rightBorderColor;
            } else {
                // Always sample from (x, 0), discarding y, since the layout FP can use y as a
                // side-channel.
                outColor = sample(colorizer, t.x0);
            }
            if (bool(makePremul)) {
                outColor.rgb *= outColor.a;
            }
            return outColor;
        }
    )");

    // If the layout does not preserve opacity, remove the opaque optimization,
    // but otherwise respect the provided color opacity state (which should take
    // into account the opacity of the border colors).
    bool layoutPreservesOpacity = gradLayout->preservesOpaqueInput();
    GrSkSLFP::OptFlags optFlags = GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha;
    if (colorsAreOpaque && layoutPreservesOpacity) {
        optFlags |= GrSkSLFP::OptFlags::kPreservesOpaqueInput;
    }

    return GrSkSLFP::Make(effect, "ClampedGradient", /*inputFP=*/nullptr, optFlags,
                          "colorizer", GrSkSLFP::IgnoreOptFlags(std::move(colorizer)),
                          "gradLayout", GrSkSLFP::IgnoreOptFlags(std::move(gradLayout)),
                          "leftBorderColor", leftBorderColor,
                          "rightBorderColor", rightBorderColor,
                          "makePremul", GrSkSLFP::Specialize<int>(makePremul),
                          "layoutPreservesOpacity",
                              GrSkSLFP::Specialize<int>(layoutPreservesOpacity));
}

static std::unique_ptr<GrFragmentProcessor> make_tiled_gradient(
        const GrFPArgs& args,
        std::unique_ptr<GrFragmentProcessor> colorizer,
        std::unique_ptr<GrFragmentProcessor> gradLayout,
        bool mirror,
        bool makePremul,
        bool colorsAreOpaque) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform shader colorizer;
        uniform shader gradLayout;

        uniform int mirror;                  // specialized
        uniform int makePremul;              // specialized
        uniform int layoutPreservesOpacity;  // specialized
        uniform int useFloorAbsWorkaround;   // specialized

        half4 main(float2 coord) {
            half4 t = sample(gradLayout, coord);

            if (!bool(layoutPreservesOpacity) && t.y < 0) {
                // layout has rejected this fragment (rely on sksl to remove this branch if the
                // layout FP preserves opacity is false)
                return half4(0);
            } else {
                if (bool(mirror)) {
                    half t_1 = t.x - 1;
                    half tiled_t = t_1 - 2 * floor(t_1 * 0.5) - 1;
                    if (bool(useFloorAbsWorkaround)) {
                        // At this point the expected value of tiled_t should between -1 and 1, so
                        // this clamp has no effect other than to break up the floor and abs calls
                        // and make sure the compiler doesn't merge them back together.
                        tiled_t = clamp(tiled_t, -1, 1);
                    }
                    t.x = abs(tiled_t);
                } else {
                    // Simple repeat mode
                    t.x = fract(t.x);
                }

                // Always sample from (x, 0), discarding y, since the layout FP can use y as a
                // side-channel.
                half4 outColor = sample(colorizer, t.x0);
                if (bool(makePremul)) {
                    outColor.rgb *= outColor.a;
                }
                return outColor;
            }
        }
    )");

    // If the layout does not preserve opacity, remove the opaque optimization,
    // but otherwise respect the provided color opacity state (which should take
    // into account the opacity of the border colors).
    bool layoutPreservesOpacity = gradLayout->preservesOpaqueInput();
    GrSkSLFP::OptFlags optFlags = GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha;
    if (colorsAreOpaque && layoutPreservesOpacity) {
        optFlags |= GrSkSLFP::OptFlags::kPreservesOpaqueInput;
    }
    const bool useFloorAbsWorkaround =
            args.fContext->priv().caps()->shaderCaps()->mustDoOpBetweenFloorAndAbs();

    return GrSkSLFP::Make(effect, "TiledGradient", /*inputFP=*/nullptr, optFlags,
                          "colorizer", GrSkSLFP::IgnoreOptFlags(std::move(colorizer)),
                          "gradLayout", GrSkSLFP::IgnoreOptFlags(std::move(gradLayout)),
                          "mirror", GrSkSLFP::Specialize<int>(mirror),
                          "makePremul", GrSkSLFP::Specialize<int>(makePremul),
                          "layoutPreservesOpacity",
                                GrSkSLFP::Specialize<int>(layoutPreservesOpacity),
                          "useFloorAbsWorkaround",
                                GrSkSLFP::Specialize<int>(useFloorAbsWorkaround));
}

// Combines the colorizer and layout with an appropriately configured top-level effect based on the
// gradient's tile mode
static std::unique_ptr<GrFragmentProcessor> make_gradient(
        const SkGradientShaderBase& shader,
        const GrFPArgs& args,
        std::unique_ptr<GrFragmentProcessor> layout,
        const SkMatrix* overrideMatrix = nullptr) {
    // No shader is possible if a layout couldn't be created, e.g. a layout-specific Make() returned
    // null.
    if (layout == nullptr) {
        return nullptr;
    }

    // Wrap the layout in a matrix effect to apply the gradient's matrix:
    SkMatrix matrix;
    if (!shader.totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    // Some two-point conical gradients use a custom matrix here
    matrix.postConcat(overrideMatrix ? *overrideMatrix : shader.getGradientMatrix());
    layout = GrMatrixEffect::Make(matrix, std::move(layout));

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
            gradient = make_tiled_gradient(args, std::move(colorizer), std::move(layout),
                                           /* mirror */ false, makePremul, allOpaque);
            break;
        case SkTileMode::kMirror:
            gradient = make_tiled_gradient(args, std::move(colorizer), std::move(layout),
                                           /* mirror */ true, makePremul, allOpaque);
            break;
        case SkTileMode::kClamp:
            // For the clamped mode, the border colors are the first and last colors, corresponding
            // to t=0 and t=1, because SkGradientShaderBase enforces that by adding color stops as
            // appropriate. If there is a hard stop, this grabs the expected outer colors for the
            // border.
            gradient = make_clamped_gradient(std::move(colorizer), std::move(layout),
                                             colors[0], colors[shader.fColorCount - 1],
                                             makePremul, allOpaque);
            break;
        case SkTileMode::kDecal:
            // Even if the gradient colors are opaque, the decal borders are transparent so
            // disable that optimization
            gradient = make_clamped_gradient(std::move(colorizer), std::move(layout),
                                             SK_PMColor4fTRANSPARENT, SK_PMColor4fTRANSPARENT,
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
    return make_gradient(shader, args, std::move(fp));
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
    return make_gradient(shader, args, std::move(fp));
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
    return make_gradient(shader, args, std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> MakeConical(const SkTwoPointConicalGradient& shader,
                                                 const GrFPArgs& args) {
    // The 2 point conical gradient can reject a pixel so it does change opacity even if the input
    // was opaque. Thus, all of these layout FPs disable that optimization.
    std::unique_ptr<GrFragmentProcessor> fp;
    SkTLazy<SkMatrix> matrix;
    switch (shader.getType()) {
        case SkTwoPointConicalGradient::Type::kStrip: {
            static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
                uniform half r0_2;
                half4 main(float2 p) {
                    half v = 1; // validation flag, set to negative to discard fragment later
                    float t = r0_2 - p.y * p.y;
                    if (t >= 0) {
                        t = p.x + sqrt(t);
                    } else {
                        v = -1;
                    }
                    return half4(half(t), v, 0, 0);
                }
            )");
            float r0 = shader.getStartRadius() / shader.getCenterX1();
            fp = GrSkSLFP::Make(effect, "TwoPointConicalStripLayout", /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "r0_2", r0 * r0);
        } break;

        case SkTwoPointConicalGradient::Type::kRadial: {
            static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
                uniform half r0;
                uniform half lengthScale;
                half4 main(float2 p) {
                    half v = 1; // validation flag, set to negative to discard fragment later
                    float t = length(p) * lengthScale - r0;
                    return half4(half(t), v, 0, 0);
                }
            )");
            float dr = shader.getDiffRadius();
            float r0 = shader.getStartRadius() / dr;
            bool isRadiusIncreasing = dr >= 0;
            fp = GrSkSLFP::Make(effect, "TwoPointConicalRadialLayout", /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "r0", r0,
                                "lengthScale", isRadiusIncreasing ? 1.0f : -1.0f);

            // GPU radial matrix is different from the original matrix, since we map the diff radius
            // to have |dr| = 1, so manually compute the final gradient matrix here.

            // Map center to (0, 0)
            matrix.set(SkMatrix::Translate(-shader.getStartCenter().fX,
                                           -shader.getStartCenter().fY));
            // scale |diffRadius| to 1
            matrix->postScale(1 / dr, 1 / dr);
        } break;

        case SkTwoPointConicalGradient::Type::kFocal: {
            static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
                // Optimization flags, all specialized:
                uniform int isRadiusIncreasing;
                uniform int isFocalOnCircle;
                uniform int isWellBehaved;
                uniform int isSwapped;
                uniform int isNativelyFocal;

                uniform half invR1;  // 1/r1
                uniform half fx;     // focalX = r0/(r0-r1)

                half4 main(float2 p) {
                    float t = -1;
                    half v = 1; // validation flag, set to negative to discard fragment later

                    float x_t = -1;
                    if (bool(isFocalOnCircle)) {
                        x_t = dot(p, p) / p.x;
                    } else if (bool(isWellBehaved)) {
                        x_t = length(p) - p.x * invR1;
                    } else {
                        float temp = p.x * p.x - p.y * p.y;

                        // Only do sqrt if temp >= 0; this is significantly slower than checking
                        // temp >= 0 in the if statement that checks r(t) >= 0. But GPU may break if
                        // we sqrt a negative float. (Although I havevn't observed that on any
                        // devices so far, and the old approach also does sqrt negative value
                        // without a check.) If the performance is really critical, maybe we should
                        // just compute the area where temp and x_t are always valid and drop all
                        // these ifs.
                        if (temp >= 0) {
                            if (bool(isSwapped) || !bool(isRadiusIncreasing)) {
                                x_t = -sqrt(temp) - p.x * invR1;
                            } else {
                                x_t = sqrt(temp) - p.x * invR1;
                            }
                        }
                    }

                    // The final calculation of t from x_t has lots of static optimizations but only
                    // do them when x_t is positive (which can be assumed true if isWellBehaved is
                    // true)
                    if (!bool(isWellBehaved)) {
                        // This will still calculate t even though it will be ignored later in the
                        // pipeline to avoid a branch
                        if (x_t <= 0.0) {
                            v = -1;
                        }
                    }
                    if (bool(isRadiusIncreasing)) {
                        if (bool(isNativelyFocal)) {
                            t = x_t;
                        } else {
                            t = x_t + fx;
                        }
                    } else {
                        if (bool(isNativelyFocal)) {
                            t = -x_t;
                        } else {
                            t = -x_t + fx;
                        }
                    }

                    if (bool(isSwapped)) {
                        t = 1 - t;
                    }

                    return half4(half(t), v, 0, 0);
                }
            )");

            const SkTwoPointConicalGradient::FocalData& focalData = shader.getFocalData();
            bool isRadiusIncreasing = (1 - focalData.fFocalX) > 0,
                 isFocalOnCircle    = focalData.isFocalOnCircle(),
                 isWellBehaved      = focalData.isWellBehaved(),
                 isSwapped          = focalData.isSwapped(),
                 isNativelyFocal    = focalData.isNativelyFocal();

            fp = GrSkSLFP::Make(effect, "TwoPointConicalFocalLayout", /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "isRadiusIncreasing", GrSkSLFP::Specialize<int>(isRadiusIncreasing),
                                "isFocalOnCircle",    GrSkSLFP::Specialize<int>(isFocalOnCircle),
                                "isWellBehaved",      GrSkSLFP::Specialize<int>(isWellBehaved),
                                "isSwapped",          GrSkSLFP::Specialize<int>(isSwapped),
                                "isNativelyFocal",    GrSkSLFP::Specialize<int>(isNativelyFocal),
                                "invR1", 1.0f / focalData.fR1,
                                "fx", focalData.fFocalX);
        } break;
    }
    return make_gradient(shader, args, std::move(fp), matrix.getMaybeNull());
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
