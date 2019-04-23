/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkUtils.h"

template <typename T, bool kInline>
class MemsetBench : public Benchmark {
public:
    explicit MemsetBench(int n)
        : fN(n)
        , fBuffer(n)
        , fName(SkStringPrintf("memset%d_%d%s", sizeof(T)*8, n, kInline ? "_inline" : "")) {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override;

private:
    int fN;
    SkAutoTMalloc<T> fBuffer;
    SkString fName;
};

template <> void MemsetBench<uint32_t, false>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        sk_memset32(fBuffer.get(), 0xFACEB004, fN);
    }
}

template <> void MemsetBench<uint16_t, false>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        sk_memset16(fBuffer.get(), 0x4973, fN);
    }
}

template <typename T>
static void memsetT(T* dst, T val, int n) {
    for (int i = 0; i < n; i++) { dst[i] = val; }
}

template <> void MemsetBench<uint32_t, true>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        memsetT<uint32_t>(fBuffer.get(), 0xFACEB004, fN);
    }
}

template <> void MemsetBench<uint16_t, true>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        memsetT<uint16_t>(fBuffer.get(), 0x4973, fN);
    }
}

DEF_BENCH(return (new MemsetBench<uint32_t,  true>(1)));
DEF_BENCH(return (new MemsetBench<uint32_t, false>(1)));
DEF_BENCH(return (new MemsetBench<uint32_t,  true>(10)));
DEF_BENCH(return (new MemsetBench<uint32_t, false>(10)));
DEF_BENCH(return (new MemsetBench<uint32_t,  true>(100)));
DEF_BENCH(return (new MemsetBench<uint32_t, false>(100)));
DEF_BENCH(return (new MemsetBench<uint32_t,  true>(1000)));
DEF_BENCH(return (new MemsetBench<uint32_t, false>(1000)));
DEF_BENCH(return (new MemsetBench<uint32_t,  true>(10000)));
DEF_BENCH(return (new MemsetBench<uint32_t, false>(10000)));
DEF_BENCH(return (new MemsetBench<uint32_t,  true>(100000)));
DEF_BENCH(return (new MemsetBench<uint32_t, false>(100000)));

DEF_BENCH(return (new MemsetBench<uint16_t,  true>(1)));
DEF_BENCH(return (new MemsetBench<uint16_t, false>(1)));
DEF_BENCH(return (new MemsetBench<uint16_t,  true>(10)));
DEF_BENCH(return (new MemsetBench<uint16_t, false>(10)));
DEF_BENCH(return (new MemsetBench<uint16_t,  true>(100)));
DEF_BENCH(return (new MemsetBench<uint16_t, false>(100)));
DEF_BENCH(return (new MemsetBench<uint16_t,  true>(1000)));
DEF_BENCH(return (new MemsetBench<uint16_t, false>(1000)));
DEF_BENCH(return (new MemsetBench<uint16_t,  true>(10000)));
DEF_BENCH(return (new MemsetBench<uint16_t, false>(10000)));
DEF_BENCH(return (new MemsetBench<uint16_t,  true>(100000)));
DEF_BENCH(return (new MemsetBench<uint16_t, false>(100000)));
