/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkRandom.h"
#include "SkTSort.h"
#include "SkString.h"

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
    // End is inclusive for SkTQSort!
    SkTQSort<int>(array, array + N - 1);
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

enum SortType {
    kSKQSort, kSKHeap, kQSort
};

static const struct {
    const char* fName;
    SortProc    fProc;
} gSorts[] = {
    { "skqsort", skqsort_sort },
    { "skheap", skheap_sort },
    { "qsort", qsort_sort },
};

class SortBench : public SkBenchmark {
    SkString           fName;
    const Type         fType;
    const SortProc     fSortProc;
    SkAutoTMalloc<int> fUnsorted;

public:
    SortBench(Type t, SortType s) : fType(t), fSortProc(gSorts[s].fProc) {
        fName.printf("sort_%s_%s", gSorts[s].fName, gRec[t].fName);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    // Delayed initialization only done if onDraw will be called.
    virtual void onPreDraw() SK_OVERRIDE {
        fUnsorted.reset(N);
        gRec[fType].fProc(fUnsorted.get());
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        SkAutoTMalloc<int> sorted(N);
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
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* NewSkQSort(Type t) {
    return new SortBench(t, kSKQSort);
}
static SkBenchmark* NewSkHeap(Type t) {
    return new SortBench(t, kSKHeap);
}
static SkBenchmark* NewQSort(Type t) {
    return new SortBench(t, kQSort);
}

DEF_BENCH( return NewSkQSort(kRand); )
DEF_BENCH( return NewSkHeap(kRand); )
DEF_BENCH( return NewQSort(kRand); )

DEF_BENCH( return NewSkQSort(kRandN); )
DEF_BENCH( return NewSkHeap(kRandN); )
DEF_BENCH( return NewQSort(kRandN); )

DEF_BENCH( return NewSkQSort(kFore); )
DEF_BENCH( return NewSkHeap(kFore); )
DEF_BENCH( return NewQSort(kFore); )

DEF_BENCH( return NewSkQSort(kBack); )
DEF_BENCH( return NewSkHeap(kBack); )
DEF_BENCH( return NewQSort(kBack); )

DEF_BENCH( return NewSkQSort(kSame); )
DEF_BENCH( return NewSkHeap(kSame); )
DEF_BENCH( return NewQSort(kSame); )
