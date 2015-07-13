/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkMutex.h"

class MutexBench : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return "mutex";
    }

    void onDraw(const int loops, SkCanvas*) override {
        SkMutex mu;
        for (int i = 0; i < loops; i++) {
            mu.acquire();
            mu.release();
        }
    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MutexBench(); )
