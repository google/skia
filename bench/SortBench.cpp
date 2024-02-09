/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkRandom.h"
#include "src/base/SkTSort.h"

#include <algorithm>
#include <stdlib.h>

using namespace skia_private;

static const int N = 1000;

static void rand_proc(int array[N]) {
    SkRandom rand;
    for (int i = 0; i < N; ++i) {
        array[i] = rand.nextS();
    }
}

static void randN_proc(int array[N]) {
    SkRandom rand;
    int mod = N / 10;
    for (int i = 0; i < N; ++i) {
        array[i] = rand.nextU() % mod;
    }
}

static void forward_proc(int array[N]) {
    for (int i = 0; i < N; ++i) {
        array[i] = i;
    }
}

static void backward_proc(int array[N]) {
    for (int i = 0; i < N; ++i) {
        array[i] = -i;
    }
}

static void same_proc(int array[N]) {
    for (int i = 0; i < N; ++i) {
        array[i] = N;
    }
}

typedef void (*SortProc)(int array[N]);

enum Type {
    kRand, kRandN, kFore, kBack, kSame
};

static const struct {
    const char* fName;
    SortProc    fProc;
} gRec[] = {
    { "rand", rand_proc },
    { "rand10", randN_proc },
    { "forward", forward_proc },
    { "backward", backward_proc },
    { "repeated", same_proc },
};

static void skqsort_sort(int array[N]) {
    SkTQSort<int>(array, array + N);
}

static void skheap_sort(int array[N]) {
    SkTHeapSort<int>(array, N);
}

extern "C" {
    static int int_compare(const void* a, const void* b) {
        const int ai = *(const int*)a;
        const int bi = *(const int*)b;
        return ai < bi ? -1 : (ai > bi);
    }
}

static void qsort_sort(int array[N]) {
    qsort(array, N, sizeof(int), int_compare);
}

static void stdsort_sort(int array[N]) {
    std::sort(array, array+N);
}

enum SortType {
    kSKQSort, kSKHeap, kQSort, kStdSort,
};

static const struct {
    const char* fName;
    SortProc    fProc;
} gSorts[] = {
    { "skqsort", skqsort_sort },
    { "skheap",   skheap_sort },
    { "qsort",     qsort_sort },
    { "stdsort", stdsort_sort },
};

class SortBench : public Benchmark {
    SkString           fName;
    const Type         fType;
    const SortProc     fSortProc;
    AutoTMalloc<int> fUnsorted;

public:
    SortBench(Type t, SortType s) : fType(t), fSortProc(gSorts[s].fProc) {
        fName.printf("sort_%s_%s", gSorts[s].fName, gRec[t].fName);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    // Delayed initialization only done if onDraw will be called.
    void onDelayedSetup() override {
        fUnsorted.reset(N);
        gRec[fType].fProc(fUnsorted.get());
    }

    void onDraw(int loops, SkCanvas*) override {
        AutoTMalloc<int> sorted(N);
        for (int i = 0; i < loops; i++) {
            memcpy(sorted.get(), fUnsorted.get(), N*sizeof(int));
            fSortProc(sorted.get());
#ifdef SK_DEBUG
            for (int j = 1; j < N; ++j) {
                SkASSERT(sorted[j - 1] <= sorted[j]);
            }
#endif
        }
    }

private:
    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

static Benchmark* NewSkQSort(Type t) {
    return new SortBench(t, kSKQSort);
}
static Benchmark* NewSkHeap(Type t) {
    return new SortBench(t, kSKHeap);
}
static Benchmark* NewQSort(Type t) {
    return new SortBench(t, kQSort);
}
static Benchmark* NewStdSort(Type t) {
    return new SortBench(t, kStdSort);
}

DEF_BENCH( return NewSkQSort(kRand); )
DEF_BENCH( return NewSkHeap(kRand); )
DEF_BENCH( return NewQSort(kRand); )
DEF_BENCH( return NewStdSort(kRand); )

DEF_BENCH( return NewSkQSort(kRandN); )
DEF_BENCH( return NewSkHeap(kRandN); )
DEF_BENCH( return NewQSort(kRandN); )
DEF_BENCH( return NewStdSort(kRandN); )

DEF_BENCH( return NewSkQSort(kFore); )
DEF_BENCH( return NewSkHeap(kFore); )
DEF_BENCH( return NewQSort(kFore); )
DEF_BENCH( return NewStdSort(kFore); )

DEF_BENCH( return NewSkQSort(kBack); )
DEF_BENCH( return NewSkHeap(kBack); )
DEF_BENCH( return NewQSort(kBack); )
DEF_BENCH( return NewStdSort(kBack); )

DEF_BENCH( return NewSkQSort(kSame); )
DEF_BENCH( return NewSkHeap(kSame); )
DEF_BENCH( return NewQSort(kSame); )
DEF_BENCH( return NewStdSort(kSame); )
