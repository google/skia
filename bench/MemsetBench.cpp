/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkString.h"
#include "SkUtils.h"

class MemsetBench : public Benchmark {
    SkString    fName;

protected:
    int      fMinSize;
    int      fMaxSize;
    enum {
        kBufferSize = 10000,
        VALUE32 = 0x12345678,
        VALUE16 = 0x1234
    };

    enum MemsetType {
        MEMSET16 = 16,
        MEMSET32 = 32
    };

public:
    MemsetBench(MemsetType type, int minSize, int maxSize)  {
        SkASSERT((minSize < maxSize) && (maxSize <= kBufferSize));
        fMinSize = minSize;
        fMaxSize = maxSize;
        fName.printf("memset%d_%d_%d", type, minSize, maxSize);
    }

    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual void performTest() = 0;

protected:
    const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int i = 0; i < loops; ++i) {
            this->performTest();
        }
    }

private:
    typedef Benchmark INHERITED;
};

class Memset32Bench : public MemsetBench {
    uint32_t kBuffer[kBufferSize + 3];
public:
    Memset32Bench(int minSize, int maxSize)
        : INHERITED(MEMSET32, minSize, maxSize) {}

protected:
    void performTest() SK_OVERRIDE {
        for(int j = fMinSize; j < fMaxSize; ++j){
            sk_memset32(kBuffer, VALUE32, j);
            sk_memset32(kBuffer + 1, VALUE32, j);
            sk_memset32(kBuffer + 2, VALUE32, j);
            sk_memset32(kBuffer + 3, VALUE32, j);
        }
    }
private:
    typedef MemsetBench INHERITED;
};

class Memset16Bench : public MemsetBench {
    uint16_t kBuffer[kBufferSize + 7];
public:
    Memset16Bench(int minSize, int maxSize)
        : INHERITED(MEMSET16, minSize, maxSize) {}

protected:
    void performTest() SK_OVERRIDE {
        for(int j = fMinSize; j < fMaxSize; ++j){
            sk_memset16(kBuffer, VALUE16, j);
            sk_memset16(kBuffer + 1, VALUE16, j);
            sk_memset16(kBuffer + 2, VALUE16, j);
            sk_memset16(kBuffer + 3, VALUE16, j);
            sk_memset16(kBuffer + 4, VALUE16, j);
            sk_memset16(kBuffer + 5, VALUE16, j);
            sk_memset16(kBuffer + 6, VALUE16, j);
            sk_memset16(kBuffer + 7, VALUE16, j);
        }
    }
private:
    typedef MemsetBench INHERITED;
};

DEF_BENCH(return new Memset32Bench(1, 600);)
DEF_BENCH(return new Memset32Bench(600, 800);)
DEF_BENCH(return new Memset32Bench(800, 1000);)
DEF_BENCH(return new Memset32Bench(1000, 2000);)
DEF_BENCH(return new Memset32Bench(2000, 3000);)
DEF_BENCH(return new Memset32Bench(3000, 4000);)
DEF_BENCH(return new Memset32Bench(4000, 5000);)

DEF_BENCH(return new Memset16Bench(1, 600);)
DEF_BENCH(return new Memset16Bench(600, 800);)
DEF_BENCH(return new Memset16Bench(800, 1000);)
DEF_BENCH(return new Memset16Bench(1000, 2000);)
DEF_BENCH(return new Memset16Bench(2000, 3000);)
DEF_BENCH(return new Memset16Bench(3000, 4000);)
DEF_BENCH(return new Memset16Bench(4000, 5000);)
