/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkColor.h"
#include "SkNx.h"

// Writing into this array prevents the loops from being compiled away.
static volatile float blackhole[4];

template <typename T>
struct Sk4fRoundtripBench : public Benchmark {
    Sk4fRoundtripBench() {}

    const char* onGetName() override {
        switch (sizeof(T)) {
            case 1: return "Sk4f_roundtrip_u8";
            case 2: return "Sk4f_roundtrip_u16";
            case 4: return "Sk4f_roundtrip_int";
        }
        SkASSERT(false);
        return "";
    }

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas* canvas) override {
        Sk4f fs(1,2,3,4);
        while (loops --> 0) {
            fs = SkNx_cast<float>(SkNx_cast<T>(fs));
        }
        fs.store((float*)blackhole);
    }
};
DEF_BENCH(return new Sk4fRoundtripBench<uint8_t>;)
DEF_BENCH(return new Sk4fRoundtripBench<uint16_t>;)
DEF_BENCH(return new Sk4fRoundtripBench<int>;)

struct Sk4fFloorBench : public Benchmark {
    Sk4fFloorBench() {}

    const char* onGetName() override { return "Sk4f_floor"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas* canvas) override {
        Sk4f fs(1,2,3,4);
        while (loops --> 0) {
            fs = fs.floor();
        }
        fs.store((float*)blackhole);
    }
};
DEF_BENCH(return new Sk4fFloorBench;)

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
