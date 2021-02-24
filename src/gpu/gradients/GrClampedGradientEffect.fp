/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This top-level effect implements clamping on the layout coordinate and requires specifying the
// border colors that are used when outside the clamped boundary. Gradients with the
// SkShader::kClamp_TileMode should use the colors at their first and last stop (after adding dummy
// stops for t=0,t=1) as the border color. This will automatically replicate the edge color, even if
// when there is a hard stop.
//
// The SkShader::kDecal_TileMode can be produced by specifying transparent black as the border
// colors, regardless of the gradient's stop colors.

in fragmentProcessor colorizer;
in fragmentProcessor gradLayout;

layout(ctype=SkPMColor4f, tracked) in uniform half4 leftBorderColor;  // t < 0.0
layout(ctype=SkPMColor4f, tracked) in uniform half4 rightBorderColor; // t > 1.0

layout(key) in bool makePremul;
// Trust the creator that this matches the color spec of the gradient
in bool colorsAreOpaque;
layout(key) in bool layoutPreservesOpacity;

half4 main() {
    half4 t = sample(gradLayout);
    half4 outColor;

    // If t.x is below 0, use the left border color without invoking the child processor. If any t.x
    // is above 1, use the right border color. Otherwise, t is in the [0, 1] range assumed by the
    // colorizer FP, so delegate to the child processor.
    if (!layoutPreservesOpacity && t.y < 0) {
        // layout has rejected this fragment (rely on sksl to remove this branch if the layout FP
        // preserves opacity is false)
        outColor = half4(0);
    } else if (t.x < 0) {
        outColor = leftBorderColor;
    } else if (t.x > 1.0) {
        outColor = rightBorderColor;
    } else {
        // Always sample from (x, 0), discarding y, since the layout FP can use y as a side-channel.
        outColor = sample(colorizer, t.x0);
    }
    @if (makePremul) {
        outColor.rgb *= outColor.a;
    }
    return outColor;
}

//////////////////////////////////////////////////////////////////////////////

// If the layout does not preserve opacity, remove the opaque optimization,
// but otherwise respect the provided color opacity state (which should take
// into account the opacity of the border colors).
@optimizationFlags {
    kCompatibleWithCoverageAsAlpha_OptimizationFlag |
    (colorsAreOpaque && layoutPreservesOpacity ? kPreservesOpaqueInput_OptimizationFlag
                                               : kNone_OptimizationFlags)
}

@make{
    static std::unique_ptr<GrFragmentProcessor> Make(
            std::unique_ptr<GrFragmentProcessor> colorizer,
            std::unique_ptr<GrFragmentProcessor> gradLayout,
            SkPMColor4f leftBorderColor,
            SkPMColor4f rightBorderColor,
            bool makePremul,
            bool colorsAreOpaque) {
        bool layoutPreservesOpacity = gradLayout->preservesOpaqueInput();
        return std::unique_ptr<GrFragmentProcessor>(new GrClampedGradientEffect(
                std::move(colorizer), std::move(gradLayout), leftBorderColor, rightBorderColor,
                makePremul, colorsAreOpaque, layoutPreservesOpacity));
    }
}
