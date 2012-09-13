/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkRefCnt.h"
#include "SkThread.h"
#include "SkWeakRefCnt.h"
#include <memory>

enum {
    N = SkBENCHLOOP(100000),
    M = SkBENCHLOOP(2)
};

class RefCntBench_Stack : public SkBenchmark {
public:
    RefCntBench_Stack(void* param) : INHERITED(param) {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_stack";
    }

    virtual void onDraw(SkCanvas* canvas) {
        for (int i = 0; i < N; ++i) {
            SkRefCnt ref;
            for (int j = 0; j < M; ++j) {
                ref.ref();
                ref.unref();
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class PlacedRefCnt : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(PlacedRefCnt)

    PlacedRefCnt() : SkRefCnt() { }
    void operator delete(void *p) { }

private:
    typedef SkRefCnt INHERITED;
};

SK_DEFINE_INST_COUNT(PlacedRefCnt)

class RefCntBench_Heap : public SkBenchmark {
public:
    RefCntBench_Heap(void* param) : INHERITED(param) {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_heap";
    }

    virtual void onDraw(SkCanvas* canvas) {
        char memory[sizeof(PlacedRefCnt)];
        for (int i = 0; i < N; ++i) {
            PlacedRefCnt* ref = new (memory) PlacedRefCnt();
            for (int j = 0; j < M; ++j) {
                ref->ref();
                ref->unref();
            }
            ref->unref();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class RefCntBench_New : public SkBenchmark {
public:
    RefCntBench_New(void* param) : INHERITED(param) {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_new";
    }

    virtual void onDraw(SkCanvas* canvas) {
        for (int i = 0; i < N; ++i) {
            SkRefCnt* ref = new SkRefCnt();
            for (int j = 0; j < M; ++j) {
                ref->ref();
                ref->unref();
            }
            ref->unref();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class WeakRefCntBench_Stack : public SkBenchmark {
public:
    WeakRefCntBench_Stack(void* param) : INHERITED(param) {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_stack_weak";
    }

    virtual void onDraw(SkCanvas* canvas) {
        for (int i = 0; i < N; ++i) {
            SkWeakRefCnt ref;
            for (int j = 0; j < M; ++j) {
                ref.ref();
                ref.unref();
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class PlacedWeakRefCnt : public SkWeakRefCnt {
public:
    PlacedWeakRefCnt() : SkWeakRefCnt() { }
    void operator delete(void *p) { }
};

class WeakRefCntBench_Heap : public SkBenchmark {
public:
    WeakRefCntBench_Heap(void* param) : INHERITED(param) {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_heap_weak";
    }

    virtual void onDraw(SkCanvas* canvas) {
        char memory[sizeof(PlacedWeakRefCnt)];
        for (int i = 0; i < N; ++i) {
            PlacedWeakRefCnt* ref = new (memory) PlacedWeakRefCnt();
            for (int j = 0; j < M; ++j) {
                ref->ref();
                ref->unref();
            }
            ref->unref();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class WeakRefCntBench_New : public SkBenchmark {
public:
    WeakRefCntBench_New(void* param) : INHERITED(param) {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_new_weak";
    }

    virtual void onDraw(SkCanvas* canvas) {
        for (int i = 0; i < N; ++i) {
            SkWeakRefCnt* ref = new SkWeakRefCnt();
            for (int j = 0; j < M; ++j) {
                ref->ref();
                ref->unref();
            }
            ref->unref();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact00(void* p) { return new RefCntBench_Stack(p); }
static SkBenchmark* Fact01(void* p) { return new RefCntBench_Heap(p); }
static SkBenchmark* Fact02(void* p) { return new RefCntBench_New(p); }

static SkBenchmark* Fact10(void* p) { return new WeakRefCntBench_Stack(p); }
static SkBenchmark* Fact11(void* p) { return new WeakRefCntBench_Heap(p); }
static SkBenchmark* Fact12(void* p) { return new WeakRefCntBench_New(p); }

static BenchRegistry gReg00(Fact00);
static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg02(Fact02);

static BenchRegistry gReg10(Fact10);
static BenchRegistry gReg11(Fact11);
static BenchRegistry gReg12(Fact12);
