/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;

// Larger values increase the strength of the dithering effect.
in uniform half range;

half4 main() {
    half4 color = sample(inputFP);
    half value;
    @if (sk_Caps.integerSupport)
    {
        // This ordered-dither code is lifted from the cpu backend.
        uint x = uint(sk_FragCoord.x);
        uint y = uint(sk_FragCoord.y) ^ x;
        uint m = (y & 1) << 5 | (x & 1) << 4 |
                 (y & 2) << 2 | (x & 2) << 1 |
                 (y & 4) >> 1 | (x & 4) >> 2;
        value = half(m) * 1.0 / 64.0 - 63.0 / 128.0;
    } else {
        // Simulate the integer effect used above using step/mod/abs. For speed, simulates a 4x4
        // dither pattern rather than an 8x8 one. Since it's 4x4, this is effectively computing:
        // uint m = (y & 1) << 3 | (x & 1) << 2 |
        //          (y & 2) << 0 | (x & 2) >> 1;
        // where 'y' has already been XOR'ed with 'x' as in the integer-supported case.

        // To get the low bit of p.x and p.y, we compute mod 2.0; for the high bit, we mod 4.0
        half4 bits = mod(half4(sk_FragCoord.yxyx), half4(2.0, 2.0, 4.0, 4.0));
        // Use step to convert the 0-3 value in bits.zw into a 0|1 value. bits.xy is already 0|1.
        bits.zw = step(2.0, bits.zw);
        // bits was constructed such that the p.x bits were already in the right place for
        // interleaving (in bits.yw). We just need to update the other bits from p.y to (p.x ^ p.y).
        // These are in bits.xz. Since the values are 0|1, we can simulate ^ as abs(y - x).
        bits.xz = abs(bits.xz - bits.yw);

        // Manual binary sum, divide by N^2, and offset
        value = dot(bits, half4(8.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0)) - 15.0 / 32.0;
    }
    // For each color channel, add the random offset to the channel value and then clamp
    // between 0 and alpha to keep the color premultiplied.
    return half4(clamp(color.rgb + value * range, 0.0, color.a), color.a);
}

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags)
    & kPreservesOpaqueInput_OptimizationFlag
}

@test(d) {
    float range = 1.0f - d->fRandom->nextRangeF(0.0f, 1.0f);
    return GrDitherEffect::Make(GrProcessorUnitTest::MakeChildFP(d), range);
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     float range) {
        if (range == 0.0 || inputFP == nullptr) {
            return inputFP;
        }
        return std::unique_ptr<GrFragmentProcessor>(new GrDitherEffect(std::move(inputFP), range));
    }
}
