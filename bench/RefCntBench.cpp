/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <memory>
#include "Benchmark.h"
#include "SkAtomics.h"
#include "SkRefCnt.h"
#include "SkWeakRefCnt.h"

enum {
    M = 2
};

class AtomicInc32 : public Benchmark {
public:
    AtomicInc32() : fX(0) {}

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return "atomic_inc_32";
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            sk_atomic_inc(&fX);
        }
    }

private:
    int32_t fX;
    typedef Benchmark INHERITED;
};

class RefCntBench_Stack : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
    typedef Benchmark INHERITED;
};

class PlacedRefCnt : public SkRefCnt {
public:
    PlacedRefCnt() : SkRefCnt() { }
    void operator delete(void*) { }

private:
    typedef SkRefCnt INHERITED;
};

class RefCntBench_Heap : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
    typedef Benchmark INHERITED;
};

class RefCntBench_New : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class WeakRefCntBench_Stack : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
    typedef Benchmark INHERITED;
};

class PlacedWeakRefCnt : public SkWeakRefCnt {
public:
    PlacedWeakRefCnt() : SkWeakRefCnt() { }
    void operator delete(void*) { }
};

class WeakRefCntBench_Heap : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
    typedef Benchmark INHERITED;
};

class WeakRefCntBench_New : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new AtomicInc32(); )

DEF_BENCH( return new RefCntBench_Stack(); )
DEF_BENCH( return new RefCntBench_Heap(); )
DEF_BENCH( return new RefCntBench_New(); )

DEF_BENCH( return new WeakRefCntBench_Stack(); )
DEF_BENCH( return new WeakRefCntBench_Heap(); )
DEF_BENCH( return new WeakRefCntBench_New(); )
