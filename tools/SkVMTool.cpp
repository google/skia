/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCpu.h"
#include "src/core/SkVM.h"
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

// TODO: remove SkVM dependency on SkWStream
bool SkWStream::writeDecAsText   (int x)           { SkDebugf("%d", x); return true; }
bool SkWStream::writeHexAsText   (unsigned x, int) { SkDebugf("%x", x); return true; }
bool SkWStream::writeScalarAsText(float x)         { SkDebugf("%g", x); return true; }

using namespace skvm;

static Program build() {
    Builder b;

    Arg ptr = b.arg(0);
    I32 v = b.load32(ptr);
    b.store32(ptr, b.mul(v,v));

    return b.done();
}

int main(int argc, char** argv) {
#if defined(__x86_64__)
    SkCpu::CacheRuntimeFeatures();
#endif
    int loops = argc > 1 ? atoi(argv[1])
                         : 1;

    if (loops < 0) {
        // Benchmark program build and JIT.
        loops = -loops;
        while (loops --> 0) {
            Program program = build();
            int x = 4;
            program.eval(0, &x);
        }
    } else {
        // Benchmark JIT code.
        Program program = build();

        int buf[4096];
        for (int i = 0; i < 4096; i++) {
            buf[i] = 1;
        }

        while (loops --> 0) {
            program.eval(4096, buf);
        }
    }
    return 0;
}
