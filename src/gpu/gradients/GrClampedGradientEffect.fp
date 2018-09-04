/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This master effect implements clamping on the layout coordinate and
// requires specifying the border colors that are used when outside the clamped
// boundary. Gradients with the SkShader::kClamp_TileMode should use the colors
// at their first and last stop (after adding dummy stops for t=0,t=1) as the
// border color. This will automatically replicate the edge color, even if
// when there is a hard stop.
//
// The SkShader::kDecal_TileMode can be produced by specifying transparent
// black as the border colors, regardless of the gradient's stop colors.

in fragmentProcessor colorizer;
in fragmentProcessor gradLayout;

layout(ctype=GrColor4f, tracked) in uniform half4 bottomBorderColor; // t < 0.0
layout(ctype=GrColor4f, tracked) in uniform half4 topBorderColor;    // t > 1.0

layout(key) in bool makePremul;

// If the border colors are not opaque, remove the opaque optimization;
// but otherwise preserve the colorizer's exposed optimizations
@optimizationFlags {
    (colorizer->compatibleWithCoverageAsAlpha() ? kCompatibleWithCoverageAsAlpha_OptimizationFlag
                                                : kNone_OptimizationFlags) |
    (colorizer->preservesOpaqueInput() && bottomBorderColor.isOpaque() && topBorderColor.isOpaque()
            ? kPreservesOpaqueInput_OptimizationFlag : kNone_OptimizationFlags)
}

void main() {
    half4 t = process(gradLayout);
    // If t.x is below 0, use the bottom border color without invoking the child
    // processor. If any t.x is above 1, use the top border color. Otherwise, t
    // is in the [0, 1] range assumed by the colorizer FP, so delegate to the
    // child processor.
    if (t.y < 0) {
        // layout has rejected this fragment
        // FIXME: only 2pt conic does this, can we add an optimization flag
        // that lets us assume t.y >= 0 in many cases?
        // FIXME: will this invalidate opaque preserving optimization?
        sk_OutColor = half4(0);
    } else if (t.x < 0) {
        sk_OutColor = bottomBorderColor;
    } else if (t.x > 1.0) {
        sk_OutColor = topBorderColor;
    } else {
        sk_OutColor = process(colorizer, t);
    }

    @if(makePremul) {
        sk_OutColor.xyz *= sk_OutColor.w;
    }
}
