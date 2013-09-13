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

static void rand_proc(int array[], int count) {
    SkRandom rand;
    for (int i = 0; i < count; ++i) {
        array[i] = rand.nextS();
    }
}

static void randN_proc(int array[], int count) {
    SkRandom rand;
    int mod = N / 10;
    for (int i = 0; i < count; ++i) {
        array[i] = rand.nextU() % mod;
    }
}

static void forward_proc(int array[], int count) {
    for (int i = 0; i < count; ++i) {
        array[i] = i;
    }
}

static void backward_proc(int array[], int count) {
    for (int i = 0; i < count; ++i) {
        array[i] = -i;
    }
}

static void same_proc(int array[], int count) {
    for (int i = 0; i < count; ++i) {
        array[i] = count;
    }
}

typedef void (*SortProc)(int array[], int count);

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

static void skqsort_sort(int array[], int count) {
    SkTQSort<int>(array, array + count);
}

static void skheap_sort(int array[], int count) {
    SkTHeapSort<int>(array, count);
}

extern "C" {
    static int int_compare(const void* a, const void* b) {
        const int ai = *(const int*)a;
        const int bi = *(const int*)b;
        return ai < bi ? -1 : (ai > bi);
    }
}

static void qsort_sort(int array[], int count) {
    qsort(array, count, sizeof(int), int_compare);
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
    SkString    fName;
    enum { MAX = 100000 };
    int         fUnsorted[MAX];
    int         fSorted[MAX];
    int         fCount;
    SortProc    fSortProc;

public:
    SortBench(Type t, int n, SortType s)  {
        if (n > MAX) {
            n = MAX;
        }
        fName.printf("sort_%s_%s", gSorts[s].fName, gRec[t].fName);
        fCount = n;
        gRec[t].fProc(fUnsorted, n);
        fSortProc = gSorts[s].fProc;
        fIsRendering = false;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas*) {
        for (int i = 0; i < this->getLoops(); i++) {
            memcpy(fSorted, fUnsorted, fCount * sizeof(int));
            fSortProc(fSorted, fCount);
#ifdef SK_DEBUG
            for (int j = 1; j < fCount; ++j) {
                SkASSERT(fSorted[j - 1] <= fSorted[j]);
            }
#endif
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* NewSkQSort(Type t) {
    return new SortBench(t, N, kSKQSort);
}
static SkBenchmark* NewSkHeap(Type t) {
    return new SortBench(t, N, kSKHeap);
}
static SkBenchmark* NewQSort(Type t) {
    return new SortBench(t, N, kQSort);
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
