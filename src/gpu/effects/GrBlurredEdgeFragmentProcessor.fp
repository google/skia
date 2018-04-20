/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

enum class Mode {
    kGaussian   = 0,
    kSmoothStep = 1
};

layout(key) in Mode mode;

void main() {
    half factor = sk_InColor.a;
    @switch (mode) {
        case Mode::kGaussian:
            factor = factor*factor*factor*(4.0 - 3.0*factor);
            break;
        case Mode::kSmoothStep:
            factor = smoothstep(0.0, 1.0, factor);
            break;
    }
    sk_OutColor = half4(factor);
}
