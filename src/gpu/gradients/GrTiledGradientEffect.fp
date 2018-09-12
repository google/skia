/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Provides tiling for the repeat or mirror modes.

in fragmentProcessor colorizer;
in fragmentProcessor gradLayout;

layout(key) in bool mirror;

void main() {
    half4 t = process(gradLayout);

    if (t.y < 0) {
        // layout has rejected this fragment
        // FIXME: only 2pt conic does this, can we add an optimization flag
        // that lets us assume t.y >= 0 in many cases?
        sk_OutColor = half4(0);
    } else {
        @if(mirror) {
            half t_1 = t.x - 1;
            half tiled_t = t_1 - 2 * floor(t_1 * 0.5) - 1;
            if (sk_Caps.mustDoOpBetweenFloorAndAbs) {
                // At this point the expected value of tiled_t should between -1 and 1, so this
                // clamp has no effect other than to break up the floor and abs calls and make sure
                // the compiler doesn't merge them back together.
                tiled_t = clamp(tiled_t, -1, 1);
            }
            t.x = abs(tiled_t);
        } else {
            // Simple repeat mode
            t.x = fract(t.x);
        }

        // t.x has been tiled (repeat or mirrored), but pass through remaining 3 components
        // unmodified.
        sk_OutColor = process(colorizer, t);
    }
}
