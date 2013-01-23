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

static void rand_proc(int array[], int count) {
    SkRandom rand;
    for (int i = 0; i < count; ++i) {
        array[i] = rand.nextS();
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
    kRand, kFore, kBack, kSame
};

static const struct {
    const char* fName;
    SortProc    fProc;
} gRec[] = {
    { "random", rand_proc },
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
    SortBench(void* param, Type t, int N, SortType s) : INHERITED(param) {
        if (N > MAX) {
            N = MAX;
        }
        fName.printf("sort_%s_%s", gSorts[s].fName, gRec[t].fName);
        fCount = N;
        gRec[t].fProc(fUnsorted, N);
        fSortProc = gSorts[s].fProc;
        fIsRendering = false;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        int n = SkBENCHLOOP(20);
        for (int i = 0; i < n; i++) {
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

#define N   10*1000

static SkBenchmark* NewSkQSort(void* param, Type t) {
    return new SortBench(param, t, N, kSKQSort);
}

DEF_BENCH( return NewSkQSort(p, kRand); )
DEF_BENCH( return NewSkQSort(p, kFore); )
DEF_BENCH( return NewSkQSort(p, kBack); )
DEF_BENCH( return NewSkQSort(p, kSame); )

static SkBenchmark* NewSkHeap(void* param, Type t) {
    return new SortBench(param, t, N, kSKHeap);
}

DEF_BENCH( return NewSkHeap(p, kRand); )
DEF_BENCH( return NewSkHeap(p, kFore); )
DEF_BENCH( return NewSkHeap(p, kBack); )
DEF_BENCH( return NewSkHeap(p, kSame); )

static SkBenchmark* NewQSort(void* param, Type t) {
    return new SortBench(param, t, N, kQSort);
}

DEF_BENCH( return NewQSort(p, kRand); )
DEF_BENCH( return NewQSort(p, kFore); )
DEF_BENCH( return NewQSort(p, kBack); )
DEF_BENCH( return NewQSort(p, kSame); )

