/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <memory>
#include "Benchmark.h"
#include "SkRefCnt.h"
#include "SkThread.h"
#include "SkWeakRefCnt.h"

enum {
    M = 2
};

class AtomicInc32 : public Benchmark {
public:
    AtomicInc32() : fX(0) {}

    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        return "atomic_inc_32";
    }

    virtual void onDraw(const int loops, SkCanvas*) {
        for (int i = 0; i < loops; ++i) {
            sk_atomic_inc(&fX);
        }
    }

private:
    int32_t fX;
    typedef Benchmark INHERITED;
};

class AtomicInc64 : public Benchmark {
public:
    AtomicInc64() : fX(0) {}

    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        return "atomic_inc_64";
    }

    virtual void onDraw(const int loops, SkCanvas*) {
        for (int i = 0; i < loops; ++i) {
            sk_atomic_inc(&fX);
        }
    }

private:
    int64_t fX;
    typedef Benchmark INHERITED;
};

class RefCntBench_Stack : public Benchmark {
public:
    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        return "ref_cnt_stack";
    }

    virtual void onDraw(const int loops, SkCanvas*) {
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
    SK_DECLARE_INST_COUNT(PlacedRefCnt)

    PlacedRefCnt() : SkRefCnt() { }
    void operator delete(void*) { }

private:
    typedef SkRefCnt INHERITED;
};

class RefCntBench_Heap : public Benchmark {
public:
    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        return "ref_cnt_heap";
    }

    virtual void onDraw(const int loops, SkCanvas*) {
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
    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        return "ref_cnt_new";
    }

    virtual void onDraw(const int loops, SkCanvas*) {
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
    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        return "ref_cnt_stack_weak";
    }

    virtual void onDraw(const int loops, SkCanvas*) {
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
    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() SK_OVERRIDE {
        return "ref_cnt_heap_weak";
    }

    void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
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
    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() SK_OVERRIDE {
        return "ref_cnt_new_weak";
    }

    void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
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
DEF_BENCH( return new AtomicInc64(); )

DEF_BENCH( return new RefCntBench_Stack(); )
DEF_BENCH( return new RefCntBench_Heap(); )
DEF_BENCH( return new RefCntBench_New(); )

DEF_BENCH( return new WeakRefCntBench_Stack(); )
DEF_BENCH( return new WeakRefCntBench_Heap(); )
DEF_BENCH( return new WeakRefCntBench_New(); )
