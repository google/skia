/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkColor.h"
#include "SkNx.h"

// Used to prevent the compiler from optimizing away the whole loop.
volatile uint32_t blackhole = 0;

// Not a great random number generator, but it's very fast.
// The code we're measuring is quite fast, so low overhead is essential.
static uint32_t lcg_rand(uint32_t* seed) {
    *seed *= 1664525;
    *seed += 1013904223;
    return *seed;
}

struct Sk4fBytesRoundtripBench : public Benchmark {
    Sk4fBytesRoundtripBench() {}

    const char* onGetName() override { return "Sk4f_roundtrip"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas* canvas) override {
        // Unlike blackhole, junk can and probably will be a register.
        uint32_t junk = 0;
        uint32_t seed = 0;
        for (int i = 0; i < loops; i++) {
            uint32_t color = lcg_rand(&seed),
                     back;
            auto f = SkNx_cast<float>(Sk4b::Load((const uint8_t*)&color));
            SkNx_cast<uint8_t>(f).store((uint8_t*)&back);
            junk ^= back;
        }
        blackhole ^= junk;
    }
};
DEF_BENCH(return new Sk4fBytesRoundtripBench;)

struct Sk4fGradientBench : public Benchmark {
    const char* onGetName() override { return "Sk4f_gradient"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    SkPMColor fDevice[100];
    void onDraw(int loops, SkCanvas*) override {
        Sk4f c0(0,0,255,255),
             c1(255,0,0,255),
             dc = c1 - c0,
             fx(0.1f),
             dx(0.002f),
             dcdx(dc*dx),
             dcdx4(dcdx+dcdx+dcdx+dcdx);

        for (int n = 0; n < loops; n++) {
            Sk4f a = c0 + dc*fx + Sk4f(0.5f),  // add an extra 0.5f to get rounding for free.
                 b = a + dcdx,
                 c = b + dcdx,
                 d = c + dcdx;
            for (size_t i = 0; i < SK_ARRAY_COUNT(fDevice); i += 4) {
                Sk4f_ToBytes((uint8_t*)(fDevice+i), a, b, c, d);
                a = a + dcdx4;
                b = b + dcdx4;
                c = c + dcdx4;
                d = d + dcdx4;
            }
        }
    }
};
DEF_BENCH(return new Sk4fGradientBench;)
