/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

// This tests a Gr class
#if SK_SUPPORT_GPU

#include "Benchmark.h"
#include "GrMemoryPool.h"
#include "SkRandom.h"
#include "SkTDArray.h"
#include "SkTemplates.h"

// change this to 0 to compare GrMemoryPool to default new / delete
#define OVERRIDE_NEW    1

struct A {
    int gStuff[10];
#if OVERRIDE_NEW
    void* operator new (size_t size) { return gBenchPool.allocate(size); }
    void operator delete (void* mem) { if (mem) { return gBenchPool.release(mem); } }
#endif
    static GrMemoryPool gBenchPool;
};
GrMemoryPool A::gBenchPool(10 * (1 << 10), 10 * (1 << 10));

/**
 * This benchmark creates and deletes objects in stack order
 */
class GrMemoryPoolBenchStack : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return "grmemorypool_stack";
    }

    void onDraw(const int loops, SkCanvas*) override {
        SkRandom r;
        enum {
            kMaxObjects = 4 * (1 << 10),
        };
        A* objects[kMaxObjects];

        // We delete if a random [-1, 1] fixed pt is < the thresh. Otherwise,
        // we allocate. We start allocate-biased and ping-pong to delete-biased
        SkFixed delThresh = -SK_FixedHalf;
        const int kSwitchThreshPeriod = loops / (2 * kMaxObjects);
        int s = 0;

        int count = 0;
        for (int i = 0; i < loops; i++, ++s) {
            if (kSwitchThreshPeriod == s) {
                delThresh = -delThresh;
                s = 0;
            }
            SkFixed del = r.nextSFixed1();
            if (count &&
                (kMaxObjects == count || del < delThresh)) {
                delete objects[count-1];
                --count;
            } else {
                objects[count] = new A;
                ++count;
            }
        }
        for (int i = 0; i < count; ++i) {
            delete objects[i];
        }
    }

private:
    typedef Benchmark INHERITED;
};

struct B {
    int gStuff[10];
#if OVERRIDE_NEW
    void* operator new (size_t size) { return gBenchPool.allocate(size); }
    void operator delete (void* mem) { if (mem) { return gBenchPool.release(mem); } }
#endif
    static GrMemoryPool gBenchPool;
};
GrMemoryPool B::gBenchPool(10 * (1 << 10), 10 * (1 << 10));

/**
 * This benchmark creates objects and deletes them in random order
 */
class GrMemoryPoolBenchRandom : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return "grmemorypool_random";
    }

    void onDraw(const int loops, SkCanvas*) override {
        SkRandom r;
        enum {
            kMaxObjects = 4 * (1 << 10),
        };
        SkAutoTDelete<B> objects[kMaxObjects];

        for (int i = 0; i < loops; i++) {
            uint32_t idx = r.nextRangeU(0, kMaxObjects-1);
            if (NULL == objects[idx].get()) {
                objects[idx].reset(new B);
            } else {
                objects[idx].free();
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};

struct C {
    int gStuff[10];
#if OVERRIDE_NEW
    void* operator new (size_t size) { return gBenchPool.allocate(size); }
    void operator delete (void* mem) { if (mem) { return gBenchPool.release(mem); } }
#endif
    static GrMemoryPool gBenchPool;
};
GrMemoryPool C::gBenchPool(10 * (1 << 10), 10 * (1 << 10));

/**
 * This benchmark creates objects and deletes them in queue order
 */
class GrMemoryPoolBenchQueue : public Benchmark {
    enum {
        M = 4 * (1 << 10),
    };
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return "grmemorypool_queue";
    }

    void onDraw(const int loops, SkCanvas*) override {
        SkRandom r;
        C* objects[M];
        for (int i = 0; i < loops; i++) {
            uint32_t count = r.nextRangeU(0, M-1);
            for (uint32_t i = 0; i < count; i++) {
                objects[i] = new C;
            }
            for (uint32_t i = 0; i < count; i++) {
                delete objects[i];
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new GrMemoryPoolBenchStack(); )
DEF_BENCH( return new GrMemoryPoolBenchRandom(); )
DEF_BENCH( return new GrMemoryPoolBenchQueue(); )

#endif
