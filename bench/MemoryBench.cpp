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
    
    enum {
        N = SkBENCHLOOP(1000)
    };
public:
    ChunkAllocBench(void* param, size_t minSize) : INHERITED(param) {
        fMinSize = minSize;
        fName.printf("chunkalloc_" SK_SIZE_T_SPECIFIER, minSize);
    }
    
protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }
    
    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        size_t inc = fMinSize >> 4;
        SkASSERT(inc > 0);
        size_t total = fMinSize * 64;

        SkChunkAlloc alloc(fMinSize);

        for (int i = 0; i < N; ++i) {
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

static SkBenchmark* F0(void* p) { return new ChunkAllocBench(p, 64); }
static SkBenchmark* F1(void* p) { return new ChunkAllocBench(p, 8*1024); }

static BenchRegistry gR0(F0);
static BenchRegistry gR1(F1);

