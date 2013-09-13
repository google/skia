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
    M = 2
};

class RefCntBench_Stack : public SkBenchmark {
public:
    RefCntBench_Stack() {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_stack";
    }

    virtual void onDraw(SkCanvas*) {
        for (int i = 0; i < this->getLoops(); ++i) {
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
    void operator delete(void*) { }

private:
    typedef SkRefCnt INHERITED;
};

SK_DEFINE_INST_COUNT(PlacedRefCnt)

class RefCntBench_Heap : public SkBenchmark {
public:
    RefCntBench_Heap() {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_heap";
    }

    virtual void onDraw(SkCanvas*) {
        char memory[sizeof(PlacedRefCnt)];
        for (int i = 0; i < this->getLoops(); ++i) {
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
    RefCntBench_New() {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_new";
    }

    virtual void onDraw(SkCanvas*) {
        for (int i = 0; i < this->getLoops(); ++i) {
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
    WeakRefCntBench_Stack() {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_stack_weak";
    }

    virtual void onDraw(SkCanvas*) {
        for (int i = 0; i < this->getLoops(); ++i) {
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
    void operator delete(void*) { }
};

class WeakRefCntBench_Heap : public SkBenchmark {
public:
    WeakRefCntBench_Heap() {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_heap_weak";
    }

    virtual void onDraw(SkCanvas*) {
        char memory[sizeof(PlacedWeakRefCnt)];
        for (int i = 0; i < this->getLoops(); ++i) {
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
    WeakRefCntBench_New() {
        fIsRendering = false;
    }
protected:
    virtual const char* onGetName() {
        return "ref_cnt_new_weak";
    }

    virtual void onDraw(SkCanvas*) {
        for (int i = 0; i < this->getLoops(); ++i) {
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

DEF_BENCH( return new RefCntBench_Stack(); )
DEF_BENCH( return new RefCntBench_Heap(); )
DEF_BENCH( return new RefCntBench_New(); )

DEF_BENCH( return new WeakRefCntBench_Stack(); )
DEF_BENCH( return new WeakRefCntBench_Heap(); )
DEF_BENCH( return new WeakRefCntBench_New(); )
