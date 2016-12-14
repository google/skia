/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkStream.h"

class StreamBench : public Benchmark {
    SkString fName;
public:
    StreamBench()  {
        fName.printf("wstream");
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops*100; ++i) {
            SkDynamicMemoryWStream stream;
            for (int j = 0; j < 100000; ++j) {
                stream.write32(j);
                stream.write32(j+j);
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new StreamBench;)
