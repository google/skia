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

layout(ctype=GrColor4f) in half4 bottomBorderColor; // t < 0.0
layout(ctype=GrColor4f) in half4 topBorderColor;    // t > 1.0

layout(ctype=GrColor4f) half4 prevBottomColor;
layout(ctype=GrColor4f) half4 prevTopColor;

uniform half4 uBottomBorderColor;
uniform half4 uTopBorderColor;

@setData(pdman) {
    if (prevBottomColor != bottomBorderColor) {
        pdman.set4fv(uBottomBorderColor, 1, bottomBorderColor.fRGBA);
        prevBottomColor = bottomBorderColor;
    }
    if (prevTopColor != topBorderColor) {
        pdman.set4fv(uTopBorderColor, 1, topBorderColor.fRGBA);
        prevTopColor = topBorderColor;
    }
}

void main() {
    half4 t = process(gradLayout);

    // If t.x is below 0, use the bottom border color without invoking the child
    // processor. If any t.x is above 1, use the top border color. Otherwise, t
    // is in the [0, 1] range assumed by the colorizer FP, so delegate to the
    // child processor.
    if (t.x < 0) {
        sk_OutColor = 0.5 * uBottomBorderColor;
    } else if (t.x > 1.0) {
        sk_OutColor = 0.5 * uTopBorderColor;
    } else {
        sk_OutColor = 0.5 * process(colorizer, t);
    }
}
