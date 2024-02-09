/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"

class CanvasSaveRestoreBench : public Benchmark {
public:
    CanvasSaveRestoreBench(int depth) : fDepth(depth) {
        fName.printf("canvas_save_restore_%d", fDepth);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kRaster; }
    SkISize onGetSize() override { return { 1, 1 }; }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkM44 m = SkM44::Rotate({0, 0, 1}, 1);

        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < fDepth; ++j) {
                canvas->save();
                canvas->concat(m);
            }
            canvas->drawColor(SkColors::kCyan);
            for (int j = 0; j < fDepth; ++j) {
                canvas->restore();
            }
        }
    }

private:
    const int fDepth;
    SkString fName;

    using INHERITED = Benchmark;
};

// Performance remains roughly constant up to 32 (the number of preallocated save records).
// After that, the cost of additional malloc/free calls starts to be measurable.
DEF_BENCH( return new CanvasSaveRestoreBench(8);)
DEF_BENCH( return new CanvasSaveRestoreBench(32);)
DEF_BENCH( return new CanvasSaveRestoreBench(128);)
DEF_BENCH( return new CanvasSaveRestoreBench(512);)
