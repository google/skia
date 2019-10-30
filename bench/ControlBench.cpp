/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

// This benchmark's runtime should be fairly constant for a given machine,
// so it can be used as a baseline to control for thermal or other throttling.

struct ControlBench : public Benchmark {
    const char* onGetName() override { return "control"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas*) override {
        // Nothing terribly useful: force a memory read, a memory write, and some math.
        volatile uint32_t rand = 0;
        for (int i = 0; i < 1000*loops; i++) {
            rand *= 1664525;
            rand += 1013904223;
        }
    }
};
DEF_BENCH(return new ControlBench;)
