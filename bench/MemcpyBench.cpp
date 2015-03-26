/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkRandom.h"
#include "SkTemplates.h"
#include "SkUtils.h"

template <typename Memcpy32>
class Memcpy32Bench : public Benchmark {
public:
    explicit Memcpy32Bench(int count, Memcpy32 memcpy32, const char* name)
        : fCount(count)
        , fMemcpy32(memcpy32)
        , fName(SkStringPrintf("%s_%d", name, count)) {}

    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onPreDraw() override {
        fDst.reset(fCount);
        fSrc.reset(fCount);

        SkRandom rand;
        for (int i = 0; i < fCount; i++) {
            fSrc[i] = rand.nextU();
        }
    }

    void onDraw(const int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            fMemcpy32(fDst, fSrc, fCount);
        }
    }

private:
    SkAutoTMalloc<uint32_t> fDst, fSrc;

    int fCount;
    Memcpy32 fMemcpy32;
    const SkString fName;
};

template <typename Memcpy32>
static Memcpy32Bench<Memcpy32>* Bench(int count, Memcpy32 memcpy32, const char* name) {
    return new Memcpy32Bench<Memcpy32>(count, memcpy32, name);
}
#define BENCH(memcpy32, count) DEF_BENCH(return Bench(count, memcpy32, #memcpy32); )


// Let the libc developers do what they think is best.
static void memcpy32_memcpy(uint32_t* dst, const uint32_t* src, int count) {
    memcpy(dst, src, sizeof(uint32_t) * count);
}
BENCH(memcpy32_memcpy, 10)
BENCH(memcpy32_memcpy, 100)
BENCH(memcpy32_memcpy, 1000)
BENCH(memcpy32_memcpy, 10000)
BENCH(memcpy32_memcpy, 100000)

// Test our chosen best, from SkUtils.h
BENCH(sk_memcpy32, 10)
BENCH(sk_memcpy32, 100)
BENCH(sk_memcpy32, 1000)
BENCH(sk_memcpy32, 10000)
BENCH(sk_memcpy32, 100000)

#undef BENCH
