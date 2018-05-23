/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

STRINGIFY(
// This controls the range of values added to color channels
layout(key) in int rangeType;

void main(int x, int y, inout half4 color) {
    half value;
    half range;
    @switch (rangeType) {
        case 0:
            range = 1.0 / 255.0;
            break;
        case 1:
            range = 1.0 / 63.0;
            break;
        default:
            // Experimentally this looks better than the expected value of 1/15.
            range = 1.0 / 15.0;
            break;
    }
    @if (sk_Caps.integerSupport) {
        // This ordered-dither code is lifted from the cpu backend.
        uint x = uint(x);
        uint y = uint(y);
        uint m = (y & 1) << 5 | (x & 1) << 4 |
                 (y & 2) << 2 | (x & 2) << 1 |
                 (y & 4) >> 1 | (x & 4) >> 2;
        value = half(m) * 1.0 / 64.0 - 63.0 / 128.0;
    } else {
        // Simulate the integer effect used above using step/mod. For speed, simulates a 4x4
        // dither pattern rather than an 8x8 one.
        half4 modValues = mod(float4(x, y, x, y), half4(2.0, 2.0, 4.0, 4.0));
        half4 stepValues = step(modValues, half4(1.0, 1.0, 2.0, 2.0));
        value = dot(stepValues, half4(8.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0)) - 15.0 / 32.0;
    }
    // For each color channel, add the random offset to the channel value and then clamp
    // between 0 and alpha to keep the color premultiplied.
    color = half4(clamp(color.rgb + value * range, 0.0, color.a), color.a);
}
)