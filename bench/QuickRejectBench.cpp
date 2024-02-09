/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "src/base/SkRandom.h"

class QuickRejectBench : public Benchmark {
    enum { N = 1000000 };
    float fFloats[N];
    int   fInts  [N];

    const char* onGetName() override { return "quick_reject"; }
    bool isSuitableFor(Backend backend) override { return backend != Backend::kNonRendering; }

    void onDelayedSetup() override  {
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fFloats[i] = 300.0f * (rand.nextSScalar1() + 0.5f);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        while (loops --> 0) {
            for (int i = 0; i < N - 4; i++) {
                if (canvas->quickReject(*(SkRect*)(fFloats+i))) {
                    fInts[i] = 11;
                } else {
                    fInts[i] = 24;
                }
            }
        }
    }
};
DEF_BENCH( return new QuickRejectBench; )

class ConcatBench : public Benchmark {
    SkMatrix fMatrix;

    const char* onGetName() override { return "concat"; }
    bool isSuitableFor(Backend backend) override { return backend != Backend::kNonRendering; }

    void onDelayedSetup() override  {
        SkRandom r;
        fMatrix.setScale(5.0f, 5.0f);
        fMatrix.setTranslateX(10.0f);
        fMatrix.setTranslateY(10.0f);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        while (loops --> 0) {
            canvas->setMatrix(SkMatrix::Scale(3, 3));
            canvas->concat(fMatrix);
        }
    }
};
DEF_BENCH( return new ConcatBench; )
