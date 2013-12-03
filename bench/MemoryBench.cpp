/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkChunkAlloc.h"
#include "SkString.h"

class ChunkAllocBench : public SkBenchmark {
    SkString    fName;
    size_t      fMinSize;
public:
    ChunkAllocBench(size_t minSize)  {
        fMinSize = minSize;
        fName.printf("chunkalloc_" SK_SIZE_T_SPECIFIER, minSize);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        size_t inc = fMinSize >> 4;
        SkASSERT(inc > 0);
        size_t total = fMinSize * 64;

        SkChunkAlloc alloc(fMinSize);

        for (int i = 0; i < loops; ++i) {
            size_t size = 0;
            int calls = 0;
            while (size < total) {
                alloc.allocThrow(inc);
                size += inc;
                calls += 1;
            }
            alloc.reset();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH( return new ChunkAllocBench(64); )
DEF_BENCH( return new ChunkAllocBench(8*1024); )

static int* calloc(size_t num) {
    return (int*)sk_calloc_throw(num*sizeof(int));
}

static int* malloc_bzero(size_t num) {
    const size_t bytes = num*sizeof(int);
    int* ints = (int*)sk_malloc_throw(bytes);
    sk_bzero(ints, bytes);
    return ints;
}

class ZerosBench : public SkBenchmark {
    size_t   fNum;
    bool     fRead;
    bool     fWrite;
    bool     fUseCalloc;
    SkString fName;
public:
    ZerosBench(size_t num, bool read, bool write, bool useCalloc)
        : fNum(num)
        , fRead(read)
        , fWrite(write)
        , fUseCalloc(useCalloc) {
        fName.printf("memory_%s", useCalloc ? "calloc" : "malloc_bzero");
        if (read && write) {
            fName.appendf("_rw");
        } else if (read) {
            fName.appendf("_r");
        } else if (write) {
            fName.appendf("_w");
        }
        fName.appendf("_"SK_SIZE_T_SPECIFIER, num);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        for (int i = 0; i < loops; i++) {
            int* zeros = fUseCalloc ? calloc(fNum) : malloc_bzero(fNum);
            if (fRead) {
                volatile int x = 15;
                for (size_t j = 0; j < fNum; j++) {
                    x ^= zeros[j];
                }
            }
            if (fWrite) {
                for (size_t j = 0; j < fNum; j++) {
                    zeros[j] = 15;
                }
            }
            sk_free(zeros);
        }
    }
};

//                             zero count  r  w  useCalloc?
DEF_BENCH(return new ZerosBench(1024*1024, 0, 0, 0))
DEF_BENCH(return new ZerosBench(1024*1024, 0, 0, 1))
DEF_BENCH(return new ZerosBench(1024*1024, 0, 1, 0))
DEF_BENCH(return new ZerosBench(1024*1024, 0, 1, 1))
DEF_BENCH(return new ZerosBench(1024*1024, 1, 0, 0))
DEF_BENCH(return new ZerosBench(1024*1024, 1, 0, 1))
DEF_BENCH(return new ZerosBench(1024*1024, 1, 1, 0))
DEF_BENCH(return new ZerosBench(1024*1024, 1, 1, 1))

DEF_BENCH(return new ZerosBench(256*1024, 0, 0, 0))
DEF_BENCH(return new ZerosBench(256*1024, 0, 0, 1))
DEF_BENCH(return new ZerosBench(256*1024, 0, 1, 0))
DEF_BENCH(return new ZerosBench(256*1024, 0, 1, 1))
DEF_BENCH(return new ZerosBench(256*1024, 1, 0, 0))
DEF_BENCH(return new ZerosBench(256*1024, 1, 0, 1))
DEF_BENCH(return new ZerosBench(256*1024, 1, 1, 0))
DEF_BENCH(return new ZerosBench(256*1024, 1, 1, 1))

DEF_BENCH(return new ZerosBench(4*1024, 0, 0, 0))
DEF_BENCH(return new ZerosBench(4*1024, 0, 0, 1))
DEF_BENCH(return new ZerosBench(4*1024, 0, 1, 0))
DEF_BENCH(return new ZerosBench(4*1024, 0, 1, 1))
DEF_BENCH(return new ZerosBench(4*1024, 1, 0, 0))
DEF_BENCH(return new ZerosBench(4*1024, 1, 0, 1))
DEF_BENCH(return new ZerosBench(4*1024, 1, 1, 0))
DEF_BENCH(return new ZerosBench(4*1024, 1, 1, 1))

DEF_BENCH(return new ZerosBench(300, 0, 0, 0))
DEF_BENCH(return new ZerosBench(300, 0, 0, 1))
DEF_BENCH(return new ZerosBench(300, 0, 1, 0))
DEF_BENCH(return new ZerosBench(300, 0, 1, 1))
DEF_BENCH(return new ZerosBench(300, 1, 0, 0))
DEF_BENCH(return new ZerosBench(300, 1, 0, 1))
DEF_BENCH(return new ZerosBench(300, 1, 1, 0))
DEF_BENCH(return new ZerosBench(300, 1, 1, 1))

DEF_BENCH(return new ZerosBench(4, 0, 0, 0))
DEF_BENCH(return new ZerosBench(4, 0, 0, 1))
DEF_BENCH(return new ZerosBench(4, 0, 1, 0))
DEF_BENCH(return new ZerosBench(4, 0, 1, 1))
DEF_BENCH(return new ZerosBench(4, 1, 0, 0))
DEF_BENCH(return new ZerosBench(4, 1, 0, 1))
DEF_BENCH(return new ZerosBench(4, 1, 1, 0))
DEF_BENCH(return new ZerosBench(4, 1, 1, 1))
