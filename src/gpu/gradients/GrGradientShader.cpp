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

#include "GrSingleIntervalGradientColorizer.h"

#include "SkMatrix.h"
#include "SkGradientShaderPriv.h"
#include "GrColor.h"

namespace GrGradientShader {

// Analyze the shader's color stops and positions and chooses an appropriate
// colorizer to represent the gradient.
std::unique_ptr<GrFragmentProcessor> MakeColorizer(
        const SkGradientShaderBase& shader, const GrFPArgs& args,
        const SkTArray<GrColor4f, true>& colors) {
    // If there are hard stops at the beginning or end, the first and/or last color
    // should be ignored by the colorizer since it should only be used in a
    // clamped border color. By detecting and removing these stops at the
    // beginning, it makes optimizing the remaining color stops simpler.

    // SkGradientShaderBase guarantees that fOrigPos[0] == 0 by adding a dummy
    bool bottomHardStop = shader.fOrigPos && SkScalarNearlyEqual(shader.fOrigPos[0], shader.fOrigPos[1]);
    // The same is true for fOrigPos[end] == 1
    bool topHardStop = shader.fOrigPos && SkScalarNearlyEqual(shader.fOrigPos[shader.fColorCount - 2], shader.fOrigPos[shader.fColorCount - 1]);

    int offset = 0;
    int count = shader.fColorCount;
    if (bottomHardStop) {
        offset += 1;
        count--;
    }
    if (topHardStop) {
        count--;
    }

    // Currently only supports 2-color single intervals. However, when the
    // gradient has hard stops and is clamped, certain 3 or 4 color gradients
    // are equivalent to a two color interval
    if (count == 2) {
        return GrSingleIntervalGradientColorizer::Make(colors[offset], colors[offset + 1]);
    }

    return nullptr;
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

    // Convert all colors into destination space and into GrColor4fs
    SkTArray<GrColor4f, true> colors;
    colors.reserve(shader.fColorCount);
    SkColor4fXformer xformedColors(shader.fOrigColors4f, shader.fColorCount,
        shader.fColorSpace.get(), args.fDstColorSpaceInfo->colorSpace());
    for (int i = 0; i < shader.fColorCount; i++) {
        colors.push_back(GrColor4f::FromSkColor4f(xformedColors.fColors[i]));
    }

    // All gradients are colorized the same way, regardless of layout
    std::unique_ptr<GrFragmentProcessor> colorizer = MakeColorizer(shader, args, colors);
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
    // FIXME: support premultipled and postmultipled alpha interpolation
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

}
