/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkWeakRefCnt.h"
#include <memory>
#include <new>

enum {
    M = 2
};

class RefCntBench_Stack : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return "ref_cnt_stack";
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            SkRefCnt ref;
            for (int j = 0; j < M; ++j) {
                ref.ref();
                ref.unref();
            }
        }
    }

private:
    using INHERITED = Benchmark;
};

class PlacedRefCnt : public SkRefCnt {
public:
    PlacedRefCnt() : SkRefCnt() { }
    void operator delete(void*) { }

private:
    using INHERITED = SkRefCnt;
};

class RefCntBench_Heap : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return "ref_cnt_heap";
    }

    void onDraw(int loops, SkCanvas*) override {
        char memory[sizeof(PlacedRefCnt)];
        for (int i = 0; i < loops; ++i) {
            PlacedRefCnt* ref = new (memory) PlacedRefCnt();
            for (int j = 0; j < M; ++j) {
                ref->ref();
                ref->unref();
            }
            ref->unref();
        }
    }

private:
    using INHERITED = Benchmark;
};

class RefCntBench_New : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return "ref_cnt_new";
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            SkRefCnt* ref = new SkRefCnt();
            for (int j = 0; j < M; ++j) {
                ref->ref();
                ref->unref();
            }
            ref->unref();
        }
    }

private:
    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

class WeakRefCntBench_Stack : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return "ref_cnt_stack_weak";
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            SkWeakRefCnt ref;
            for (int j = 0; j < M; ++j) {
                ref.ref();
                ref.unref();
            }
        }
    }

private:
    using INHERITED = Benchmark;
};

class PlacedWeakRefCnt : public SkWeakRefCnt {
public:
    PlacedWeakRefCnt() : SkWeakRefCnt() { }
    void operator delete(void*) { }
};

class WeakRefCntBench_Heap : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return "ref_cnt_heap_weak";
    }

    void onDraw(int loops, SkCanvas*) override {
        char memory[sizeof(PlacedWeakRefCnt)];
        for (int i = 0; i < loops; ++i) {
            PlacedWeakRefCnt* ref = new (memory) PlacedWeakRefCnt();
            for (int j = 0; j < M; ++j) {
                ref->ref();
                ref->unref();
            }
            ref->unref();
        }
    }

private:
    using INHERITED = Benchmark;
};

class WeakRefCntBench_New : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return "ref_cnt_new_weak";
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            SkWeakRefCnt* ref = new SkWeakRefCnt();
            for (int j = 0; j < M; ++j) {
                ref->ref();
                ref->unref();
            }
            ref->unref();
        }
    }

private:
    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new RefCntBench_Stack(); )
DEF_BENCH( return new RefCntBench_Heap(); )
DEF_BENCH( return new RefCntBench_New(); )

DEF_BENCH( return new WeakRefCntBench_Stack(); )
DEF_BENCH( return new WeakRefCntBench_Heap(); )
DEF_BENCH( return new WeakRefCntBench_New(); )
