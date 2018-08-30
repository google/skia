/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This only supports a 2-color single interval so it is a simple  linear
// interpolation between the two end points based on t. But it serves as a good
// test for connecting all of the plumbing into a functional gradient shader.

layout(ctype=GrColor4f) in half4 start;
layout(ctype=GrColor4f) half4 prevStart;
layout(ctype=GrColor4f) in half4 end;
layout(ctype=GrColor4f) half4 prevEnd;

uniform half4 uStart;
uniform half4 uEnd;

@setData(pdman) {
    if (start != prevStart) {
        pdman.set4fv(uStart, 1, start.fRGBA);
        prevStart = start;
    }

    if (end != prevEnd) {
        pdman.set4fv(uEnd, 1, end.fRGBA);
        prevEnd = end;
    }
}

void main() {
    half t = sk_InColor.x;

    // Clamping and/or wrapping was already handled by the parent shader
    // so the output color is a simple lerp.
    sk_OutColor = (1 - t) * uStart + t * uEnd;
}
