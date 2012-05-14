/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkThread.h"
#include <memory>

enum {
    N = SkBENCHLOOP(1000000),
    M = SkBENCHLOOP(2)
};

class RefCntBench_Stack : public SkBenchmark {
public:
    RefCntBench_Stack(void* param) : INHERITED(param) {
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
    PlacedRefCnt() : SkRefCnt() { }
    void operator delete(void *p) { }
};

class RefCntBench_Heap : public SkBenchmark {
public:
    RefCntBench_Heap(void* param) : INHERITED(param) {
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

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return new RefCntBench_Stack(p); }
static SkBenchmark* Fact1(void* p) { return new RefCntBench_Heap(p); }

static BenchRegistry gReg01(Fact0);
static BenchRegistry gReg02(Fact1);

