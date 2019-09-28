/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Should have height = 1px, horizontal axis represents t = 0 to 1
in uniform sampler2D gradient;

@samplerParams(gradient) {
    GrSamplerState::ClampBilerp()
}

void main() {
    half2 coord = half2(sk_InColor.x, 0.5);
    sk_OutColor = sample(gradient, coord);
}
