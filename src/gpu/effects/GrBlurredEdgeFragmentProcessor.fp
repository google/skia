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
    half factor = 1.0 - sk_InColor.a;
    @switch (mode) {
        case Mode::kGaussian:
            factor = half(exp(-factor * factor * 4.0) - 0.018);
            break;
        case Mode::kSmoothStep:
            factor = smoothstep(1.0, 0.0, factor);
            break;
    }
    sk_OutColor = half4(factor);
}
