/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkPMFloat.h"

// Used to prevent the compiler from optimizing away the whole loop.
volatile uint32_t blackhole = 0;

// Not a great random number generator, but it's very fast.
// The code we're measuring is quite fast, so low overhead is essential.
static uint32_t lcg_rand(uint32_t* seed) {
    *seed *= 1664525;
    *seed += 1013904223;
    return *seed;
}

// I'm having better luck getting these to constant-propagate away as template parameters.
struct PMFloatRoundtripBench : public Benchmark {
    PMFloatRoundtripBench() {}

    const char* onGetName() override { return "SkPMFloat_roundtrip"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(const int loops, SkCanvas* canvas) override {
        // Unlike blackhole, junk can and probably will be a register.
        uint32_t junk = 0;
        uint32_t seed = 0;
        for (int i = 0; i < loops; i++) {
            SkPMColor color;
        #ifdef SK_DEBUG
            // Our SkASSERTs will remind us that it's technically required that we premultiply.
            color = SkPreMultiplyColor(lcg_rand(&seed));
        #else
            // But it's a lot faster not to, and this code won't really mind the non-PM colors.
            color = lcg_rand(&seed);
        #endif

            auto f = SkPMFloat::FromPMColor(color);
            SkPMColor back = f.round();
            junk ^= back;
        }
        blackhole ^= junk;
    }
};
DEF_BENCH(return new PMFloatRoundtripBench;)

struct PMFloatGradientBench : public Benchmark {
    const char* onGetName() override { return "PMFloat_gradient"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    SkPMColor fDevice[100];
    void onDraw(const int loops, SkCanvas*) override {
        Sk4f c0 = SkPMFloat::FromARGB(1, 1, 0, 0),
             c1 = SkPMFloat::FromARGB(1, 0, 0, 1),
             dc = c1 - c0,
             fx(0.1f),
             dx(0.002f),
             dcdx(dc*dx),
             dcdx4(dcdx+dcdx+dcdx+dcdx);

        for (int n = 0; n < loops; n++) {
            Sk4f a = c0 + dc*fx,
                 b = a + dcdx,
                 c = b + dcdx,
                 d = c + dcdx;
            for (size_t i = 0; i < SK_ARRAY_COUNT(fDevice); i += 4) {
                fDevice[i+0] = SkPMFloat(a).round();
                fDevice[i+1] = SkPMFloat(b).round();
                fDevice[i+2] = SkPMFloat(c).round();
                fDevice[i+3] = SkPMFloat(d).round();
                a = a + dcdx4;
                b = b + dcdx4;
                c = c + dcdx4;
                d = d + dcdx4;
            }
        }
    }
};

DEF_BENCH(return new PMFloatGradientBench;)
