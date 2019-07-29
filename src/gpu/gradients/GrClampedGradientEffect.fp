/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This master effect implements clamping on the layout coordinate and requires specifying the
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

void main() {
    half4 t = sample(gradLayout);
    // If t.x is below 0, use the left border color without invoking the child processor. If any t.x
    // is above 1, use the right border color. Otherwise, t is in the [0, 1] range assumed by the
    // colorizer FP, so delegate to the child processor.
    if (!gradLayout.preservesOpaqueInput && t.y < 0) {
        // layout has rejected this fragment (rely on sksl to remove this branch if the layout FP
        // preserves opacity is false)
        sk_OutColor = half4(0);
    } else if (t.x < 0) {
        sk_OutColor = leftBorderColor;
    } else if (t.x > 1.0) {
        sk_OutColor = rightBorderColor;
    } else {
        sk_OutColor = sample(colorizer, t);
    }

    @if(makePremul) {
        sk_OutColor.xyz *= sk_OutColor.w;
    }
}

//////////////////////////////////////////////////////////////////////////////

// If the layout does not preserve opacity, remove the opaque optimization,
// but otherwise respect the provided color opacity state (which should take
// into account the opacity of the border colors).
@optimizationFlags {
    kCompatibleWithCoverageAsAlpha_OptimizationFlag |
    (colorsAreOpaque && gradLayout->preservesOpaqueInput() ? kPreservesOpaqueInput_OptimizationFlag
                                                           : kNone_OptimizationFlags)
}
