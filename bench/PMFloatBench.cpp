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
template <bool kClamp, bool kWide>
struct PMFloatGetSetBench : public Benchmark {
    PMFloatGetSetBench() {}

    const char* onGetName() override {
        switch (kClamp << 1 | kWide) {
            case 0: return "SkPMFloat_get_1x";
            case 1: return "SkPMFloat_get_4x";
            case 2: return "SkPMFloat_clamp_1x";
            case 3: return "SkPMFloat_clamp_4x";
        }
        SkFAIL("unreachable");
        return "oh bother";
    }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(const int loops, SkCanvas* canvas) override {
        // Unlike blackhole, junk can and probably will be a register.
        uint32_t junk = 0;
        uint32_t seed = 0;
        for (int i = 0; i < loops; i++) {
            SkPMColor colors[4];
        #ifdef SK_DEBUG
            for (int i = 0; i < 4; i++) {
                // Our SkASSERTs will remind us that it's technically required that we premultiply.
                colors[i] = SkPreMultiplyColor(lcg_rand(&seed));
            }
        #else
            // But it's a lot faster not to, and this code won't really mind the non-PM colors.
            (void)lcg_rand(&seed);
            colors[0] = seed + 0;
            colors[1] = seed + 1;
            colors[2] = seed + 2;
            colors[3] = seed + 3;
        #endif

            SkPMFloat fa,fb,fc,fd;
            if (kWide) {
                SkPMFloat::From4PMColors(colors, &fa, &fb, &fc, &fd);
            } else {
                fa = SkPMFloat::FromPMColor(colors[0]);
                fb = SkPMFloat::FromPMColor(colors[1]);
                fc = SkPMFloat::FromPMColor(colors[2]);
                fd = SkPMFloat::FromPMColor(colors[3]);
            }

            SkPMColor back[4];
            switch (kClamp << 1 | kWide) {
                case 0: {
                    back[0] = fa.round();
                    back[1] = fb.round();
                    back[2] = fc.round();
                    back[3] = fd.round();
                } break;
                case 1: SkPMFloat::RoundTo4PMColors(fa, fb, fc, fd, back); break;
                case 2: {
                    back[0] = fa.roundClamp();
                    back[1] = fb.roundClamp();
                    back[2] = fc.roundClamp();
                    back[3] = fd.roundClamp();
                } break;
                case 3: SkPMFloat::RoundClampTo4PMColors(fa, fb, fc, fd, back); break;
            }
            for (int i = 0; i < 4; i++) {
                junk ^= back[i];
            }
        }
        blackhole ^= junk;
    }
};

// Extra () help DEF_BENCH not get confused by the comma inside the <>.
DEF_BENCH(return (new PMFloatGetSetBench< true,  true>);)
DEF_BENCH(return (new PMFloatGetSetBench<false,  true>);)
DEF_BENCH(return (new PMFloatGetSetBench< true, false>);)
DEF_BENCH(return (new PMFloatGetSetBench<false, false>);)

struct PMFloatGradientBench : public Benchmark {
    const char* onGetName() override { return "PMFloat_gradient"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    SkPMColor fDevice[100];
    void onDraw(const int loops, SkCanvas*) override {
        Sk4f c0 = SkPMFloat::FromARGB(255, 255, 0, 0),
             c1 = SkPMFloat::FromARGB(255, 0, 0, 255),
             dc = c1 - c0,
             fx(0.1f),
             dx(0.002f),
             dcdx(dc*dx),
             dcdx4(dcdx+dcdx+dcdx+dcdx);

        for (int n = 0; n < loops; n++) {
            Sk4f a = c0 + dc*fx + Sk4f(0.5f),  // The +0.5f lets us call trunc() instead of get().
                 b = a + dcdx,
                 c = b + dcdx,
                 d = c + dcdx;
            for (size_t i = 0; i < SK_ARRAY_COUNT(fDevice); i += 4) {
                fDevice[i+0] = SkPMFloat(a).trunc();
                fDevice[i+1] = SkPMFloat(b).trunc();
                fDevice[i+2] = SkPMFloat(c).trunc();
                fDevice[i+3] = SkPMFloat(d).trunc();
                a += dcdx4;
                b += dcdx4;
                c += dcdx4;
                d += dcdx4;
            }
        }
    }
};

DEF_BENCH(return new PMFloatGradientBench;)
