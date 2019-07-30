/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCpu.h"
#include "src/core/SkVM.h"
#include "tools/SkVMBuilders.h"
#include <chrono>
#include <stdio.h>
#include <stdlib.h>

void sk_abort_no_print() {
    abort();
}

void SkDebugf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

static skvm::Program plus_one() {
    skvm::Builder b;

    skvm::Arg ptr = b.varying<int>();
    skvm::I32 v = b.load32(ptr);
    b.store32(ptr, b.add(v, b.splat(1)));

    return b.done("plus_one");
}

static skvm::Program square() {
    skvm::Builder b;

    skvm::Arg ptr = b.varying<int>();
    skvm::I32 v = b.load32(ptr);
    b.store32(ptr, b.mul(v,v));

    return b.done("square");
}

static void print(double val, const char* units) {
    const char* scales[] = { "", "K", "M", "G", "T" };
    const char** scale = scales;

    while (val > 10000.0) {
        val *= 1/1000.0;
        scale++;
    }

    printf("%4d %s%s", (int)val, *scale, units);
}

template <typename Fn>
static double measure(Fn&& fn) {
    using clock = std::chrono::steady_clock;

    int loops = 0;
    auto start = clock::now();
    std::chrono::duration<double> elapsed;
    do {
        fn();
        loops++;
        elapsed = clock::now() - start;
    } while (elapsed < std::chrono::milliseconds(100));

    return loops / elapsed.count();
}

template <typename... Args>
static void time(const char* name, const skvm::Program& program, Args... args) {
    printf("%20s", name);

    for (int N : { 15, 255, 4095 }) {
        double loops_per_sec = measure([&]{
            program.eval(N, args...);
        });

        printf("\t");
        print(N*loops_per_sec, "px/s");
    }
    printf("\n");
}

int main(int argc, char** argv) {
#if defined(__x86_64__)
    SkCpu::CacheRuntimeFeatures();
#endif

    int src[4096],
        dst[4096];
    time("plus_one", plus_one(), dst);
    time(  "square",   square(), dst);

    time("srcover_f32"      , SrcoverBuilder_F32      ().done("srcover_f32"      ), src, dst);
    time("srcover_i32"      , SrcoverBuilder_I32      ().done("srcover_i32"      ), src, dst);
    time("srcover_i32_naive", SrcoverBuilder_I32_Naive().done("srcover_i32_naive"), src, dst);
    time("srcover_i32_SWAR" , SrcoverBuilder_I32_SWAR ().done("srcover_i32_SWAR" ), src, dst);

    return 0;
}
