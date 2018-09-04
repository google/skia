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
#include "GrGradientBitmapCache.h"

#include "SkMatrix.h"
#include "SkGr.h"

#include "GrColor.h"
#include "GrContext.h"
#include "GrContextPriv.h"


namespace GrGradientShader {

// Each cache entry costs 1K or 2K of RAM. Each bitmap will be 1x256 at either 32bpp or 64bpp.
static const int MAX_NUM_CACHED_GRADIENT_BITMAPS = 32;
static const int GRADIENT_TEXTURE_SIZE = 256;

// NOTE: signature takes raw pointers to the color/pos arrays and a count to make it easy for
// MakeColorizer to transparently take care of hard stops at the end points of the gradient.
std::unique_ptr<GrFragmentProcessor> MakeTexturedColorizer(const GrColor4f* colors,
                                                           const SkScalar* positions, int count,
                                                           bool premul, const GrFPArgs& args) {
    static GrGradientBitmapCache gCache(MAX_NUM_CACHED_GRADIENT_BITMAPS, GRADIENT_TEXTURE_SIZE);

    // Use 8888 or F16, depending on the destination config.
    // TODO: Use 1010102 for opaque gradients, at least if destination is 1010102?
    SkColorType colorType = kRGBA_8888_SkColorType;
    if (kLow_GrSLPrecision != GrSLSamplerPrecision(args.fDstColorSpaceInfo->config()) &&
        args.fContext->contextPriv().caps()->isConfigTexturable(kRGBA_half_GrPixelConfig)) {
        colorType = kRGBA_F16_SkColorType;
    }
    SkAlphaType alphaType = premul ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;

    SkBitmap bitmap;
    gCache.getGradient(colors, positions, count, colorType, alphaType, &bitmap);
    SkASSERT(1 == bitmap.height() && SkIsPow2(bitmap.width()));
    SkASSERT(bitmap.isImmutable());

    sk_sp<GrTextureProxy> proxy = GrMakeCachedBitmapProxy(
            args.fContext->contextPriv().proxyProvider(), bitmap);
    if (proxy == nullptr) {
        SkDebugf("Gradient won't draw. Could not create texture.");
        return nullptr;
    }

    return GrTextureGradientColorizer::Make(std::move(proxy));
}

// Analyze the shader's color stops and positions and chooses an appropriate
// colorizer to represent the gradient.
std::unique_ptr<GrFragmentProcessor> MakeColorizer(const SkTArray<GrColor4f, true>& colors,
                                                   const SkScalar* positions, bool premul,
                                                   const GrFPArgs& args) {
    // If there are hard stops at the beginning or end, the first and/or last color
    // should be ignored by the colorizer since it should only be used in a
    // clamped border color. By detecting and removing these stops at the
    // beginning, it makes optimizing the remaining color stops simpler.

    // SkGradientShaderBase guarantees that pos[0] == 0 by adding a dummy
    bool bottomHardStop = SkScalarNearlyEqual(positions[0], positions[1]);
    // The same is true for pos[end] == 1
    bool topHardStop = SkScalarNearlyEqual(positions[colors.count() - 2],
                                           positions[colors.count() - 1]);

    int offset = 0;
    int count = colors.count();
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
    } else if (count == 3) {
        // Must be a dual interval gradient, where the middle point is at offset+1 and the two
        // intervals share the middle color stop.
        return GrDualIntervalGradientColorizer::Make(colors[offset], colors[offset + 1],
                                                     colors[offset + 1], colors[offset + 2],
                                                     positions[offset + 1]);
    } else if (count == 4 && SkScalarNearlyEqual(positions[offset + 1], positions[offset + 2])) {
        // Two separate intervals that join at the same threshold position
        return GrDualIntervalGradientColorizer::Make(colors[offset], colors[offset + 1],
                                                     colors[offset + 2], colors[offset + 3],
                                                     positions[offset + 1]);
    }

    // return MakeTexturedColorizer(colors.begin() + offset, positions + offset, count, premul, args);
    return MakeTexturedColorizer(colors.begin(), positions, count, premul, args);
}

// Combines the colorizer and layout with an appropriately configured master
// effect based on the gradient's tile mode
std::unique_ptr<GrFragmentProcessor> MakeGradient(
        const SkGradientShaderBase& shader, const GrFPArgs& args,
        std::unique_ptr<GrFragmentProcessor> layout) {
    // No shader is possible if a layout couldn't be created, e.g. a layout-specific
    // Make() returned null.
    if (layout == nullptr) {
        return nullptr;
    }

    // Convert all colors into destination space and into GrColor4fs, and handle
    // premul issues depending on the interpolation mode
    bool inputPremul = shader.getGradFlags() & SkGradientShader::kInterpolateColorsInPremul_Flag;
    SkTArray<GrColor4f, true> colors;
    colors.reserve(shader.fColorCount);
    SkColor4fXformer xformedColors(shader.fOrigColors4f, shader.fColorCount,
            shader.fColorSpace.get(), args.fDstColorSpaceInfo->colorSpace());
    for (int i = 0; i < shader.fColorCount; i++) {
        GrColor4f c = GrColor4f::FromSkColor4f(xformedColors.fColors[i]);
        if (inputPremul) {
            c = c.premul();
        }
        colors.push_back(c);
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
        SkScalar posScale = SkScalarFastInvert(shader.fColorCount - 1);
        for (int i = 0 ; i < shader.fColorCount; i++) {
            implicitPos.push_back(SkIntToScalar(i) * posScale);
        }
        positions = implicitPos.begin();
    }

    // All gradients are colorized the same way, regardless of layout
    std::unique_ptr<GrFragmentProcessor> colorizer = MakeColorizer(
            colors, positions, inputPremul, args);
    if (colorizer == nullptr) {
        return nullptr;
    }

    // All tile modes are supported (unless something was added to SkShader)
    std::unique_ptr<GrFragmentProcessor> master;
    switch(shader.getTileMode()) {
        case SkShader::kRepeat_TileMode:
            master = GrTiledGradientEffect::Make(
                std::move(colorizer), std::move(layout), /* mirror */ false);
            break;
        case SkShader::kMirror_TileMode:
            master = GrTiledGradientEffect::Make(
                std::move(colorizer), std::move(layout), /* mirror */ true);
            break;
        case SkShader::kClamp_TileMode: {
            // For the clamped mode, the border colors are the first and last
            // colors, corresponding to t=0 and t=1, because SkGradientShaderBase
            // enforces that by adding color stops as appropriate. If there is
            // a hard stop, this grabs the expected outer colors for the border.
            master = GrClampedGradientEffect::Make(
                std::move(colorizer), std::move(layout),
                colors[0], colors[shader.fColorCount - 1]);
            break;
        }
        case SkShader::kDecal_TileMode:
            master = GrClampedGradientEffect::Make(
                std::move(colorizer), std::move(layout),
                GrColor4f::TransparentBlack(), GrColor4f::TransparentBlack());
            break;
    }

    if (master == nullptr) {
        // Unexpected tile mode
        return nullptr;
    }

    if (!inputPremul) {
        // When interpolating unpremul colors, the output of the gradient
        // effect fp's will also be unpremul, so wrap it to ensure its premul.
        // - this is unnecessary when interpolating premul colors since the
        //   output color is premul by nature
        master = GrFragmentProcessor::PremulOutput(std::move(master));
    }

    return GrFragmentProcessor::MulChildByInputAlpha(std::move(master));
}

std::unique_ptr<GrFragmentProcessor> MakeLinear(
    const SkGradientShaderBase& shader, const GrFPArgs& args,
    const SkPoint& start, const SkPoint& end) {
    SkMatrix invLocalMatrix;
    if (!shader.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&invLocalMatrix)) {
        return nullptr;
    }

    return MakeGradient(shader,args, GrLinearGradientLayout::Make(
            invLocalMatrix, start, end));
}

std::unique_ptr<GrFragmentProcessor> MakeRadial(
    const SkGradientShaderBase& shader, const GrFPArgs& args,
    const SkPoint& center, SkScalar radius) {
    SkMatrix invLocalMatrix;
    if (!shader.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&invLocalMatrix)) {
        return nullptr;
    }

    return MakeGradient(shader,args, GrRadialGradientLayout::Make(
        invLocalMatrix, center, radius));
}

std::unique_ptr<GrFragmentProcessor> MakeSweep(
    const SkGradientShaderBase& shader, const GrFPArgs& args,
    const SkPoint& center, SkScalar bias, SkScalar scale) {
    SkMatrix invLocalMatrix;
    if (!shader.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&invLocalMatrix)) {
        return nullptr;
    }

    return MakeGradient(shader,args, GrSweepGradientLayout::Make(
        invLocalMatrix, center, bias, scale));
}

std::unique_ptr<GrFragmentProcessor> MakeConical(
        const SkTwoPointConicalGradient& shader, const GrFPArgs& args) {
    return MakeGradient(shader, args, GrTwoPointConicalGradientLayout::Make(shader, args));
}

}
