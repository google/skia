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

// ~~~~~~~~~~~~~~~~~~~ //

using namespace skvm;

template <typename Fn>
static int time(Fn&& fn) {
    using clock = std::chrono::steady_clock;
    const int ms = 200;

    int n = 0;
    for (auto start = clock::now(); clock::now() - start < std::chrono::milliseconds(ms); ) {
        fn();
        n++;
    }
    return n * (1000/ms);
}

static void print(double val, const char* unit) {
    const char* labels[] = { "", "K", "M", "G", "T" };
    const char** label = labels;

    while (val > 1000.0) {
        val *= 1/1000.0;
        label++;
    }

    printf("\t%d %s%s", (int)val, *label, unit);
}

template <typename... Args>
static void exercise(const char* name, skvm::Builder&& b, Args... args) {
    printf("%20s", name);
    {
        b.done(name);
        int loops = time([&]{
            b.done(nullptr);
        });
        print(loops, "Hz");
    }

    for (const int N : {15, 255, 4095}) {
        skvm::Program p = b.done(name);
        int loops = time([&]{
            p.eval(N, args...);
        });
        print(N*loops, "px/s");
    }

    printf("\n");
}

int main(int argc, char** argv) {
#if defined(__x86_64__)
    SkCpu::CacheRuntimeFeatures();
#endif
    printf("%20s\tbuild\tN=15\t\tN=255\t\tN=4095\n", "");

    int ints[4096];
    //float floats[4096];

    {
        Builder b;
        Arg ptr = b.arg(0);
        I32 v = b.load32(ptr);
        b.store32(ptr, b.mul(v,v));

        exercise("square", std::move(b), ints);
    }
    {
        Builder b;
        Arg ptr = b.arg(0);
        I32 v = b.load32(ptr);
        b.store32(ptr, b.add(v,b.splat(1)));

        exercise("plus_one", std::move(b), ints);
    }

    return 0;
}
