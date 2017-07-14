/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@class {
    enum Mode {
        kGaussian_Mode = 0,
        kSmoothStep_Mode = 1
    };
}

layout(key) in int mode;

void main() {
    float factor = 1.0 - sk_InColor.a;
    @switch (mode) {
        case 0: // kGaussian_Mode
            factor = exp(-factor * factor * 4.0) - 0.018;
            break;
        case 1: // kSmoothstep_Mode
            factor = smoothstep(1.0, 0.0, factor);
            break;
    }
    sk_OutColor = vec4(factor);
}
