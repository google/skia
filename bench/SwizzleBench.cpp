/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkSwizzlePriv.h"

class SwizzleBench : public Benchmark {
public:

    SwizzleBench(const char* name, SkOpts::Swizzle_8888_u32 fn) : fName(name), fFn_u32(fn) {}
    SwizzleBench(const char* name, SkOpts::Swizzle_8888_u8  fn) : fName(name), fFn_u8 (fn) {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return fName; }
    void onDraw(int loops, SkCanvas*) override {
        static const int K = 1023; // Arbitrary, but nice to be a non-power-of-two to trip up SIMD.
        uint32_t dst[K], src[K];
        while (loops --> 0) {
            if (fFn_u32) { fFn_u32(dst,                 src, K); }
            if (fFn_u8)  { fFn_u8 (dst, (const uint8_t*)src, K); }
        }
    }
private:
    const char* fName;
    SkOpts::Swizzle_8888_u32 fFn_u32 = nullptr;
    SkOpts::Swizzle_8888_u8  fFn_u8  = nullptr;
};


DEF_BENCH(return new SwizzleBench("SkOpts::RGBA_to_rgbA", SkOpts::RGBA_to_rgbA));
DEF_BENCH(return new SwizzleBench("SkOpts::RGBA_to_bgrA", SkOpts::RGBA_to_bgrA));
DEF_BENCH(return new SwizzleBench("SkOpts::RGBA_to_BGRA", SkOpts::RGBA_to_BGRA));
DEF_BENCH(return new SwizzleBench("SkOpts::RGB_to_RGB1",  SkOpts::RGB_to_RGB1));
DEF_BENCH(return new SwizzleBench("SkOpts::RGB_to_BGR1",  SkOpts::RGB_to_BGR1));
DEF_BENCH(return new SwizzleBench("SkOpts::gray_to_RGB1", SkOpts::gray_to_RGB1));
DEF_BENCH(return new SwizzleBench("SkOpts::grayA_to_RGBA", SkOpts::grayA_to_RGBA));
DEF_BENCH(return new SwizzleBench("SkOpts::grayA_to_rgbA", SkOpts::grayA_to_rgbA));
DEF_BENCH(return new SwizzleBench("SkOpts::inverted_CMYK_to_RGB1", SkOpts::inverted_CMYK_to_RGB1));
DEF_BENCH(return new SwizzleBench("SkOpts::inverted_CMYK_to_BGR1", SkOpts::inverted_CMYK_to_BGR1));
