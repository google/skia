/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkOpts.h"
#include "SkRandom.h"

struct FloatToHalfBench : public Benchmark {
    const char* onGetName() override { return "float_to_half"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRandom rand;
        float fs[1023];
        for (float& f : fs) {
            f = rand.nextF();
        }

        uint16_t hs[1023];
        while (loops --> 0) {
            SkOpts::float_to_half(hs, fs, 1023);
        }
    }
};
DEF_BENCH(return new FloatToHalfBench;)

struct HalfToFloatBench : public Benchmark {
    const char* onGetName() override { return "half_to_float"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRandom rand;
        uint16_t hs[1023];
        for (uint16_t& h : hs) {
            h = rand.nextU16();
        }

        float fs[1023];
        while (loops --> 0) {
            SkOpts::half_to_float(fs, hs, 1023);
        }
    }
};
DEF_BENCH(return new HalfToFloatBench;)
