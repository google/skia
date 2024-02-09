/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkStream.h"

class StreamBench : public Benchmark {
    SkString    fName;
    const bool  fTestWrite4;
public:
    StreamBench(bool testWrite4) : fTestWrite4(testWrite4) {
        fName.printf("wstream_%d", testWrite4);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        const char t3[] = { 1, 2, 3 };
        const char t5[] = { 1, 2, 3, 4, 5 };
        for (int i = 0; i < loops*100; ++i) {
            SkDynamicMemoryWStream stream;
            for (int j = 0; j < 10000; ++j) {
                if (fTestWrite4) {
                    stream.write32(j);
                    stream.write32(j+j);
                } else {
                    stream.write(t3, 3);
                    stream.write(t5, 5);
                }
            }
        }
    }

private:
    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new StreamBench(false);)
DEF_BENCH(return new StreamBench(true);)
