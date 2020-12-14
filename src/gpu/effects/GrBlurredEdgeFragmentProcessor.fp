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

in fragmentProcessor? inputFP;
layout(key) in Mode mode;

half4 main() {
    half inputAlpha = sample(inputFP).a;
    half factor = 1.0 - inputAlpha;
    @switch (mode) {
        case Mode::kGaussian:
            factor = half(exp(-factor * factor * 4.0) - 0.018);
            break;
        case Mode::kSmoothStep:
            factor = smoothstep(1.0, 0.0, factor);
            break;
    }
    return half4(factor);
}
