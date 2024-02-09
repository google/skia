/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkMemset.h"

using namespace skia_private;

template <typename T>
class MemsetBench : public Benchmark {
public:
    explicit MemsetBench(size_t bytes)
        : fN(bytes / sizeof(T))
        , fBuffer(fN)
        , fName(SkStringPrintf("memset%zu_%zu", sizeof(T)*8, bytes)) {}

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override;

private:
    int fN;
    AutoTMalloc<T> fBuffer;
    SkString fName;
};

template <> void MemsetBench<uint64_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::memset64(fBuffer.get(), 0xFACEFACEFACEFACE, fN);
    }
}

template <> void MemsetBench<uint32_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::memset32(fBuffer.get(), 0xFACEB004, fN);
    }
}

template <> void MemsetBench<uint16_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::memset16(fBuffer.get(), 0x4973, fN);
    }
}

DEF_BENCH(return (new MemsetBench<uint64_t>(16)));
DEF_BENCH(return (new MemsetBench<uint64_t>(64)));
DEF_BENCH(return (new MemsetBench<uint64_t>(256)));
DEF_BENCH(return (new MemsetBench<uint64_t>(512)));
DEF_BENCH(return (new MemsetBench<uint64_t>(768)));
DEF_BENCH(return (new MemsetBench<uint64_t>(1024)));
DEF_BENCH(return (new MemsetBench<uint64_t>(2048)));
DEF_BENCH(return (new MemsetBench<uint64_t>(4096)));
DEF_BENCH(return (new MemsetBench<uint64_t>(65536)));

DEF_BENCH(return (new MemsetBench<uint32_t>(16)));
DEF_BENCH(return (new MemsetBench<uint32_t>(64)));
DEF_BENCH(return (new MemsetBench<uint32_t>(256)));
DEF_BENCH(return (new MemsetBench<uint32_t>(512)));
DEF_BENCH(return (new MemsetBench<uint32_t>(768)));
DEF_BENCH(return (new MemsetBench<uint32_t>(1024)));
DEF_BENCH(return (new MemsetBench<uint32_t>(2048)));
DEF_BENCH(return (new MemsetBench<uint32_t>(4096)));
DEF_BENCH(return (new MemsetBench<uint32_t>(65536)));

DEF_BENCH(return (new MemsetBench<uint16_t>(16)));
DEF_BENCH(return (new MemsetBench<uint16_t>(64)));
DEF_BENCH(return (new MemsetBench<uint16_t>(256)));
DEF_BENCH(return (new MemsetBench<uint16_t>(512)));
DEF_BENCH(return (new MemsetBench<uint16_t>(768)));
DEF_BENCH(return (new MemsetBench<uint16_t>(1024)));
DEF_BENCH(return (new MemsetBench<uint16_t>(2048)));
DEF_BENCH(return (new MemsetBench<uint16_t>(4096)));
DEF_BENCH(return (new MemsetBench<uint16_t>(65536)));
