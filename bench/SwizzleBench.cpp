/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkOpts.h"

class SwizzleBench : public Benchmark {
public:
    SwizzleBench(const char* name, SkOpts::Swizzle_8888_8888 fn) : fName(name), fFn(fn) {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return fName; }
    void onDraw(int loops, SkCanvas*) override {
        static const int K = 1023; // Arbitrary, but nice to be a non-power-of-two to trip up SIMD.
        uint32_t dst[K], src[K];
        while (loops --> 0) {
            fFn(dst, src, K);
        }
    }
private:
    const char* fName;
    SkOpts::Swizzle_8888_8888 fFn;
};


DEF_BENCH(return new SwizzleBench("SkOpts::premul_xxxa", SkOpts::premul_xxxa));
DEF_BENCH(return new SwizzleBench("SkOpts::swaprb_xxxa", SkOpts::swaprb_xxxa));
DEF_BENCH(return new SwizzleBench("SkOpts::premul_swaprb_xxxa", SkOpts::premul_swaprb_xxxa));
