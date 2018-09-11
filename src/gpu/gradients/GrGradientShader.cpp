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
#include "GrSingleIntervalGradientColorizer.h"

#include "SkGradientShaderPriv.h"
#include "GrColor.h"

// Analyze the shader's color stops and positions and chooses an appropriate colorizer to represent
// the gradient.
static std::unique_ptr<GrFragmentProcessor> make_colorizer(const SkGradientShaderBase& shader,
        const GrFPArgs& args, const GrColor4f* colors) {
    // If there are hard stops at the beginning or end, the first and/or last color should be
    // ignored by the colorizer since it should only be used in a clamped border color. By detecting
    // and removing these stops at the beginning, it makes optimizing the remaining color stops
    // simpler.

    // SkGradientShaderBase guarantees that fOrigPos[0] == 0 by adding a dummy
    bool bottomHardStop = shader.fOrigPos && SkScalarNearlyEqual(shader.fOrigPos[0],
                                                                 shader.fOrigPos[1]);
    // The same is true for fOrigPos[end] == 1
    bool topHardStop = shader.fOrigPos &&
            SkScalarNearlyEqual(shader.fOrigPos[shader.fColorCount - 2],
                                shader.fOrigPos[shader.fColorCount - 1]);

    int offset = 0;
    int count = shader.fColorCount;
    if (bottomHardStop) {
        offset += 1;
        count--;
    }
    if (topHardStop) {
        count--;
    }

    // Currently only supports 2-color single intervals. However, when the gradient has hard stops
    // and is clamped, certain 3 or 4 color gradients are equivalent to a two color interval
    if (count == 2) {
        return GrSingleIntervalGradientColorizer::Make(colors[offset], colors[offset + 1]);
    }

    return nullptr;
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

    // Convert all colors into destination space and into GrColor4fs, and handle
    // premul issues depending on the interpolation mode
    bool inputPremul = shader.getGradFlags() & SkGradientShader::kInterpolateColorsInPremul_Flag;
    SkAutoSTMalloc<4, GrColor4f> colors(shader.fColorCount);
    SkColor4fXformer xformedColors(shader.fOrigColors4f, shader.fColorCount,
            shader.fColorSpace.get(), args.fDstColorSpaceInfo->colorSpace());
    for (int i = 0; i < shader.fColorCount; i++) {
        colors[i] = GrColor4f::FromSkColor4f(xformedColors.fColors[i]);
        if (inputPremul) {
            colors[i] = colors[i].premul();
        }
    }

    // All gradients are colorized the same way, regardless of layout
    std::unique_ptr<GrFragmentProcessor> colorizer = make_colorizer(shader, args, colors.get());
    if (colorizer == nullptr) {
        return nullptr;
    }

    // All tile modes are supported (unless something was added to SkShader)
    std::unique_ptr<GrFragmentProcessor> master;
    switch(shader.getTileMode()) {
        case SkShader::kRepeat_TileMode:
            master = GrTiledGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                 /* mirror */ false);
            break;
        case SkShader::kMirror_TileMode:
            master = GrTiledGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                 /* mirror */ true);
            break;
        case SkShader::kClamp_TileMode:
            // For the clamped mode, the border colors are the first and last colors, corresponding
            // to t=0 and t=1, because SkGradientShaderBase enforces that by adding color stops as
            // appropriate. If there is a hard stop, this grabs the expected outer colors for the
            // border.
            master = GrClampedGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                   colors[0], colors[shader.fColorCount - 1]);
            break;
        case SkShader::kDecal_TileMode:
            master = GrClampedGradientEffect::Make(std::move(colorizer), std::move(layout),
                                                   GrColor4f::TransparentBlack(),
                                                   GrColor4f::TransparentBlack());
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

namespace GrGradientShader {

std::unique_ptr<GrFragmentProcessor> MakeLinear(const SkLinearGradient& shader,
                                                const GrFPArgs& args) {
    return make_gradient(shader, args, GrLinearGradientLayout::Make(shader, args));
}

}
