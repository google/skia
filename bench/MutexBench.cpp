/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkMutex.h"
#include "SkSharedMutex.h"
#include "SkSpinlock.h"
#include "SkString.h"

template <typename Mutex>
class MutexBench : public Benchmark {
public:
    MutexBench(SkString benchPrefix) : fBenchName(benchPrefix += "UncontendedBenchmark") { }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fBenchName.c_str();
    }

    void onDraw(const int loops, SkCanvas*) override {
        Mutex mu;
        for (int i = 0; i < loops; i++) {
            mu.acquire();
            mu.release();
        }
    }

private:
    typedef Benchmark INHERITED;
    SkString fBenchName;
};

class SharedBench : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return "SkSharedMutexSharedUncontendedBenchmark";
    }

    void onDraw(const int loops, SkCanvas*) override {
        SkSharedMutex mu;
        for (int i = 0; i < loops; i++) {
            mu.acquireShared();
            mu.releaseShared();
        }
    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MutexBench<SkSharedMutex>(SkString("SkSharedMutex")); )
DEF_BENCH( return new MutexBench<SkMutex>(SkString("SkMutex")); )
DEF_BENCH( return new MutexBench<SkSpinlock>(SkString("SkSpinlock")); )
DEF_BENCH( return new SharedBench; )

