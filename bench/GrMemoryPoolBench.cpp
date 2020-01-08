/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/private/GrTypesPriv.h"
#include "include/utils/SkRandom.h"
#include "src/gpu/GrMemoryPool.h"

#include <type_traits>

namespace {

// sizeof is a multiple of GrMemoryPool::kAlignment for 4, 8, or 16 byte alignment
using Aligned = std::aligned_storage<32, GrMemoryPool::kAlignment>::type;
static_assert(sizeof(Aligned) == 32);
static_assert(sizeof(Aligned) % GrMemoryPool::kAlignment == 0);

// sizeof is not a multiple of GrMemoryPool::kAlignment (will not be a multiple of max_align_t
// if it's 4, 8, or 16, as desired).
using Unaligned = std::aligned_storage<30, 2>::type;
static_assert(sizeof(Unaligned) == 30);
static_assert(sizeof(Unaligned) % GrMemoryPool::kAlignment != 0);

// When max_align_t == 16, 8, or 4 the padded Unaligned will also be 32
static_assert(GrAlignTo(sizeof(Unaligned), GrMemoryPool::kAlignment) == sizeof(Aligned));

// All benchmarks create and delete the same number of objects. The key difference is the order
// of operations, the size of the objects being allocated, and the size of the pool.
typedef void (*RunBenchProc)(GrMemoryPool*, int);

}

// N objects are created, and then destroyed in reverse order (fully unwinding the cursor within
// each block of the memory pool).
template <typename T>
static void run_stack(GrMemoryPool* pool, int loops) {
    static const int kMaxObjects = 4 * (1 << 10);
    T* objs[kMaxObjects];
    for (int i = 0; i < loops; ++i) {
        // Push N objects into the pool (or heap if pool is null)
        for (int j = 0; j < kMaxObjects; ++j) {
            objs[j] = pool ? (T*) pool->allocate(sizeof(T)) : new T;
        }
        // Pop N objects off in LIFO order
        for (int j = kMaxObjects - 1; j >= 0; --j) {
            if (pool) {
                pool->release(objs[j]);
            } else {
                delete objs[j];
            }
        }

        // Everything has been cleaned up for the next loop
    }
}

// N objects are created, and then destroyed in creation order (is not able to unwind the cursor
// within each block, but can reclaim the block once everything is destroyed).
template <typename T>
static void run_queue(GrMemoryPool* pool, int loops) {
    static const int kMaxObjects = 4 * (1 << 10);
    T* objs[kMaxObjects];
    for (int i = 0; i < loops; ++i) {
        // Push N objects into the pool (or heap if pool is null)
        for (int j = 0; j < kMaxObjects; ++j) {
            objs[j] = pool ? (T*) pool->allocate(sizeof(T)) : new T;
        }
        // Pop N objects off in FIFO order
        for (int j = 0; j < kMaxObjects; ++j) {
            if (pool) {
                pool->release(objs[j]);
            } else {
                delete objs[j];
            }
        }

        // Everything has been cleaned up for the next loop
    }
}

// N objects are created and immediately destroyed, so space at the start of the pool should be
// immediately reclaimed.
template <typename T>
static void run_pushpop(GrMemoryPool* pool, int loops) {
    static const int kMaxObjects = 4 * (1 << 10);
    T* objs[kMaxObjects];
    for (int i = 0; i < loops; ++i) {
        // Push N objects into the pool (or heap if pool is null)
        for (int j = 0; j < kMaxObjects; ++j) {
            if (pool) {
                objs[j] = (T*) pool->allocate(sizeof(T));
                pool->release(objs[j]);
            } else {
                objs[j] = new T;
                delete objs[j];
            }
        }

        // Everything has been cleaned up for the next loop
    }
}

// N object creations and destructions are invoked in random order.
template <typename T>
static void run_random(GrMemoryPool* pool, int loops) {
    static const int kMaxObjects = 4 * (1 << 10);
    T* objs[kMaxObjects];
    for (int i = 0; i < kMaxObjects; ++i) {
        objs[i] = nullptr;
    }

    auto del = [&](int j) {
        // Delete
        if (pool) {
            pool->release(objs[j]);
        } else {
            delete objs[j];
        }
        objs[j] = nullptr;
    };

    SkRandom r;
    for (int i = 0; i < loops; ++i) {
        // Execute 2*kMaxObjects operations, which should average to N create and N destroy,
        // followed by a small number of remaining deletions.
        for (int j = 0; j < 2 * kMaxObjects; ++j) {
            int k = r.nextRangeU(0, kMaxObjects-1);
            if (objs[k]) {
                del(k);
            } else {
                // Create
                objs[k] = pool ? (T*) pool->allocate(sizeof(T)) : new T;
            }
        }

        // Ensure everything is null for the next loop
        for (int j = 0; j < kMaxObjects; ++j) {
            if (objs[j]) {
                del(j);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class GrMemoryPoolBench : public Benchmark {
public:
    GrMemoryPoolBench(const char* name, RunBenchProc proc, int poolSize)
            : fPoolSize(poolSize)
            , fProc(proc) {
        fName.printf("grmemorypool_%s", name);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        std::unique_ptr<GrMemoryPool> pool;
        if (fPoolSize > 0) {
            pool = GrMemoryPool::Make(fPoolSize, fPoolSize);
        } // else keep it null to test regular new/delete performance

        fProc(pool.get(), loops);
    }

    SkString     fName;
    int          fPoolSize;
    RunBenchProc fProc;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static const int kLargePool = 10 * (1 << 10);
static const int kSmallPool = GrMemoryPool::kMinAllocationSize;

DEF_BENCH( return new GrMemoryPoolBench("stack_aligned_lg",      run_stack<Aligned>,     kLargePool); )
DEF_BENCH( return new GrMemoryPoolBench("stack_aligned_sm",      run_stack<Aligned>,     kSmallPool); )
DEF_BENCH( return new GrMemoryPoolBench("stack_aligned_ref",     run_stack<Aligned>,     0); )
DEF_BENCH( return new GrMemoryPoolBench("stack_unaligned_lg",    run_stack<Unaligned>,   kLargePool); )
DEF_BENCH( return new GrMemoryPoolBench("stack_unaligned_sm",    run_stack<Unaligned>,   kSmallPool); )
DEF_BENCH( return new GrMemoryPoolBench("stack_unaligned_ref",   run_stack<Unaligned>,   0); )

DEF_BENCH( return new GrMemoryPoolBench("queue_aligned_lg",      run_queue<Aligned>,     kLargePool); )
DEF_BENCH( return new GrMemoryPoolBench("queue_aligned_sm",      run_queue<Aligned>,     kSmallPool); )
DEF_BENCH( return new GrMemoryPoolBench("queue_aligned_ref",     run_queue<Aligned>,     0); )
DEF_BENCH( return new GrMemoryPoolBench("queue_unaligned_lg",    run_queue<Unaligned>,   kLargePool); )
DEF_BENCH( return new GrMemoryPoolBench("queue_unaligned_sm",    run_queue<Unaligned>,   kSmallPool); )
DEF_BENCH( return new GrMemoryPoolBench("queue_unaligned_ref",   run_queue<Unaligned>,   0); )

DEF_BENCH( return new GrMemoryPoolBench("pushpop_aligned_lg",    run_pushpop<Aligned>,   kLargePool); )
DEF_BENCH( return new GrMemoryPoolBench("pushpop_aligned_sm",    run_pushpop<Aligned>,   kSmallPool); )
// DEF_BENCH( return new GrMemoryPoolBench("pushpop_aligned_ref",   run_pushpop<Aligned>,   0); )
DEF_BENCH( return new GrMemoryPoolBench("pushpop_unaligned_lg",  run_pushpop<Unaligned>, kLargePool); )
DEF_BENCH( return new GrMemoryPoolBench("pushpop_unaligned_sm",  run_pushpop<Unaligned>, kSmallPool); )
// DEF_BENCH( return new GrMemoryPoolBench("pushpop_unaligned_ref", run_pushpop<Unaligned>, 0); )
// pushpop_x_ref are not meaningful because the compiler completely optimizes away new T; delete *.

DEF_BENCH( return new GrMemoryPoolBench("random_aligned_lg",     run_random<Aligned>,    kLargePool); )
DEF_BENCH( return new GrMemoryPoolBench("random_aligned_sm",     run_random<Aligned>,    kSmallPool); )
DEF_BENCH( return new GrMemoryPoolBench("random_aligned_ref",    run_random<Aligned>,    0); )
DEF_BENCH( return new GrMemoryPoolBench("random_unaligned_lg",   run_random<Unaligned>,  kLargePool); )
DEF_BENCH( return new GrMemoryPoolBench("random_unaligned_sm",   run_random<Unaligned>,  kSmallPool); )
DEF_BENCH( return new GrMemoryPoolBench("random_unaligned_ref",  run_random<Unaligned>,  0); )
