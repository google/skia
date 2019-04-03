/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGradientShader.h"

#include "GrClampedGradientEffect.h"
#include "GrTiledGradientEffect.h"

#include "GrLinearGradientLayout.h"
#include "GrRadialGradientLayout.h"
#include "GrSweepGradientLayout.h"
#include "GrTwoPointConicalGradientLayout.h"

#include "GrDualIntervalGradientColorizer.h"
#include "GrSingleIntervalGradientColorizer.h"
#include "GrTextureGradientColorizer.h"
#include "GrUnrolledBinaryGradientColorizer.h"
#include "GrGradientBitmapCache.h"

#include "GrCaps.h"
#include "GrColor.h"
#include "GrColorSpaceInfo.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "SkGr.h"

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
    if (kLow_GrSLPrecision != GrSLSamplerPrecision(args.fDstColorSpaceInfo->config()) &&
        args.fContext->priv().caps()->isConfigTexturable(kRGBA_half_GrPixelConfig)) {
        colorType = kRGBA_F16_SkColorType;
    }
    SkAlphaType alphaType = premul ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;

    SkBitmap bitmap;
    gCache.getGradient(colors, positions, count, colorType, alphaType, &bitmap);
    SkASSERT(1 == bitmap.height() && SkIsPow2(bitmap.width()));
    SkASSERT(bitmap.isImmutable());

    sk_sp<GrTextureProxy> proxy = GrMakeCachedBitmapProxy(
            args.fContext->priv().proxyProvider(), bitmap);
    if (proxy == nullptr) {
        SkDebugf("Gradient won't draw. Could not create texture.");
        return nullptr;
    }

    return GrTextureGradientColorizer::Make(std::move(proxy));
}

// Analyze the shader's color stops and positions and chooses an appropriate colorizer to represent
// the gradient.
static std::unique_ptr<GrFragmentProcessor> make_colorizer(const SkPMColor4f* colors,
        const SkScalar* positions, int count, bool premul, const GrFPArgs& args) {
    // If there are hard stops at the beginning or end, the first and/or last color should be
    // ignored by the colorizer since it should only be used in a clamped border color. By detecting
    // and removing these stops at the beginning, it makes optimizing the remaining color stops
    // simpler.

    // SkGradientShaderBase guarantees that pos[0] == 0 by adding a dummy
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
        return GrSingleIntervalGradientColorizer::Make(colors[offset], colors[offset + 1]);
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
            return GrDualIntervalGradientColorizer::Make(colors[offset], colors[offset + 1],
                                                         colors[offset + 1], colors[offset + 2],
                                                         positions[offset + 1]);
        } else if (count == 4 && SkScalarNearlyEqual(positions[offset + 1],
                                                     positions[offset + 2])) {
            // Two separate intervals that join at the same threshold position
            return GrDualIntervalGradientColorizer::Make(colors[offset], colors[offset + 1],
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

// Combines the colorizer and layout with an appropriately configured master effect based on the
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
            shader.fColorSpace.get(), args.fDstColorSpaceInfo->colorSpace());
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
        implicitPos.reserve(shader.fColorCount);
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

    // The master effect has to export premul colors, but under certain conditions it doesn't need
    // to do anything to achieve that: i.e. its interpolating already premul colors (inputPremul)
    // or all the colors have a = 1, in which case premul is a no op. Note that this allOpaque
    // check is more permissive than SkGradientShaderBase's isOpaque(), since we can optimize away
    // the make-premul op for two point conical gradients (which report false for isOpaque).
    bool makePremul = !inputPremul && !allOpaque;

    // All tile modes are supported (unless something was added to SkShader)
    std::unique_ptr<GrFragmentProcessor> master;
    switch(shader.getTileMode()) {
        case SkTileMode::kRepeat:
            master = GrTiledGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                 /* mirror */ false, makePremul, allOpaque);
            break;
        case SkTileMode::kMirror:
            master = GrTiledGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                 /* mirror */ true, makePremul, allOpaque);
            break;
        case SkTileMode::kClamp:
            // For the clamped mode, the border colors are the first and last colors, corresponding
            // to t=0 and t=1, because SkGradientShaderBase enforces that by adding color stops as
            // appropriate. If there is a hard stop, this grabs the expected outer colors for the
            // border.
            master = GrClampedGradientEffect::Make(std::move(colorizer), std::move(layout),
                    colors[0], colors[shader.fColorCount - 1], makePremul, allOpaque);
            break;
        case SkTileMode::kDecal:
            // Even if the gradient colors are opaque, the decal borders are transparent so
            // disable that optimization
            master = GrClampedGradientEffect::Make(std::move(colorizer), std::move(layout),
                    SK_PMColor4fTRANSPARENT, SK_PMColor4fTRANSPARENT,
                    makePremul, /* colorsAreOpaque */ false);
            break;
    }

    if (master == nullptr) {
        // Unexpected tile mode
        return nullptr;
    }

    return GrFragmentProcessor::MulChildByInputAlpha(std::move(master));
}

namespace GrGradientShader {

std::unique_ptr<GrFragmentProcessor> MakeLinear(const SkLinearGradient& shader,
                                                const GrFPArgs& args) {
    return make_gradient(shader, args, GrLinearGradientLayout::Make(shader, args));
}

std::unique_ptr<GrFragmentProcessor> MakeRadial(const SkRadialGradient& shader,
                                                const GrFPArgs& args) {
    return make_gradient(shader,args, GrRadialGradientLayout::Make(shader, args));
}

std::unique_ptr<GrFragmentProcessor> MakeSweep(const SkSweepGradient& shader,
                                               const GrFPArgs& args) {
    return make_gradient(shader,args, GrSweepGradientLayout::Make(shader, args));
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

}
