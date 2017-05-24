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

in int mode;

void main() {
    float factor;
    if (!sk_Args.gpImplementsDistanceVector) {
        // assuming interpolant is set in vertex colors
        factor = 1.0 - sk_InColor.a;
    } else {
        // using distance to edge to compute interpolant
        float radius = sk_InColor.r * 256.0 * 64.0 + sk_InColor.g * 64.0;
        float pad = sk_InColor.b * 64.0;
        factor = 1.0 - clamp((sk_DistanceVector.z - pad) / radius, 0.0, 1.0);
    }
    switch (mode) {
        case 0: // kGaussian_Mode
            factor = exp(-factor * factor * 4.0) - 0.018;
            break;
        case 1: // kSmoothstep_Mode
            factor = smoothstep(1.0, 0.0, factor);
            break;
    }
    sk_OutColor = vec4(factor);
}