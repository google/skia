/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkRandom.h"

#include "SkChunkAlloc.h"
#include "SkDeque.h"
#include "SkTArray.h"
#include "SkTDArray.h"

// This file has several benchmarks using various data structures to do stack-like things:
//   - push
//   - push, immediately pop
//   - push many, pop all of them
//   - serial access
//   - random access
// When a data structure doesn't suppport an operation efficiently, we leave that combination out.
// Where possible we hint to the data structure to allocate in 4K pages.
//
// These benchmarks may help you decide which data structure to use for a dynamically allocated
// ordered list of allocations that grows on one end.
//
// Current overall winner (01/2014): SkTDArray.
// It wins every benchmark on every machine I tried (Desktop, Nexus S, Laptop).

template <typename Impl>
struct StackBench : public Benchmark {
    virtual bool isSuitableFor(Backend b) SK_OVERRIDE { return b == kNonRendering_Backend; }
    virtual const char* onGetName() SK_OVERRIDE { return Impl::kName; }
    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE { Impl::bench(loops); }
};

#define BENCH(name)                                                          \
    struct name { static const char* const kName; static void bench(int); }; \
    const char* const name::kName = #name;                                   \
    DEF_BENCH(return new StackBench<name>();)                                \
    void name::bench(int loops)

static const int K = 2049;

// Add K items, then iterate through them serially many times.

BENCH(Deque_Serial) {
    SkDeque s(sizeof(int), 1024);
    for (int i = 0; i < K; i++) *(int*)s.push_back() = i;

    volatile int junk = 0;
    for (int j = 0; j < loops; j++) {
        SkDeque::Iter it(s, SkDeque::Iter::kFront_IterStart);
        while(void* p = it.next()) {
            junk += *(int*)p;
        }
    }
}

BENCH(TArray_Serial) {
    SkTArray<int, true> s;
    for (int i = 0; i < K; i++) s.push_back(i);

    volatile int junk = 0;
    for (int j = 0; j < loops; j++) {
        for (int i = 0; i < s.count(); i++) junk += s[i];
    }
}

BENCH(TDArray_Serial) {
    SkTDArray<int> s;
    for (int i = 0; i < K; i++) s.push(i);

    volatile int junk = 0;
    for (int j = 0; j < loops; j++) {
        for (int i = 0; i < s.count(); i++) junk += s[i];
    }
}

// Add K items, then randomly access them many times.

BENCH(TArray_RandomAccess) {
    SkTArray<int, true> s;
    for (int i = 0; i < K; i++) s.push_back(i);

    SkRandom rand;
    volatile int junk = 0;
    for (int i = 0; i < K*loops; i++) {
        junk += s[rand.nextULessThan(K)];
    }
}

BENCH(TDArray_RandomAccess) {
    SkTDArray<int> s;
    for (int i = 0; i < K; i++) s.push(i);

    SkRandom rand;
    volatile int junk = 0;
    for (int i = 0; i < K*loops; i++) {
        junk += s[rand.nextULessThan(K)];
    }
}

// Push many times.

BENCH(ChunkAlloc_Push) {
    SkChunkAlloc s(4096);
    for (int i = 0; i < K*loops; i++) s.allocThrow(sizeof(int));
}

BENCH(Deque_Push) {
    SkDeque s(sizeof(int), 1024);
    for (int i = 0; i < K*loops; i++) *(int*)s.push_back() = i;
}

BENCH(TArray_Push) {
    SkTArray<int, true> s;
    for (int i = 0; i < K*loops; i++) s.push_back(i);
}

BENCH(TDArray_Push) {
    SkTDArray<int> s;
    for (int i = 0; i < K*loops; i++) s.push(i);
}

// Push then immediately pop many times.

BENCH(ChunkAlloc_PushPop) {
    SkChunkAlloc s(4096);
    for (int i = 0; i < K*loops; i++) {
        void* p = s.allocThrow(sizeof(int));
        s.unalloc(p);
    }
}

BENCH(Deque_PushPop) {
    SkDeque s(sizeof(int), 1024);
    for (int i = 0; i < K*loops; i++) {
        *(int*)s.push_back() = i;
        s.pop_back();
    }
}

BENCH(TArray_PushPop) {
    SkTArray<int, true> s;
    for (int i = 0; i < K*loops; i++) {
        s.push_back(i);
        s.pop_back();
    }
}

BENCH(TDArray_PushPop) {
    SkTDArray<int> s;
    for (int i = 0; i < K*loops; i++) {
        s.push(i);
        s.pop();
    }
}

// Push many items, then pop them all.

BENCH(Deque_PushAllPopAll) {
    SkDeque s(sizeof(int), 1024);
    for (int i = 0; i < K*loops; i++) *(int*)s.push_back() = i;
    for (int i = 0; i < K*loops; i++) s.pop_back();
}

BENCH(TArray_PushAllPopAll) {
    SkTArray<int, true> s;
    for (int i = 0; i < K*loops; i++) s.push_back(i);
    for (int i = 0; i < K*loops; i++) s.pop_back();
}

BENCH(TDArray_PushAllPopAll) {
    SkTDArray<int> s;
    for (int i = 0; i < K*loops; i++) s.push(i);
    for (int i = 0; i < K*loops; i++) s.pop();
}
