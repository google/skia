/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This only supports a 2-color single interval so it is a simple linear interpolation between the
// two end points based on t. But it serves as a good test for connecting all of the plumbing into a
// functional gradient shader.

layout(ctype=SkPMColor4f, tracked) in uniform half4 start;
layout(ctype=SkPMColor4f, tracked) in uniform half4 end;

half4 main(float2 coord) {
    // Clamping and/or wrapping was already handled by the parent shader so the output color is a
    // simple lerp.
    return mix(start, end, half(coord.x));
}
