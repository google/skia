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

    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual void onPreDraw() SK_OVERRIDE {
        fDst.reset(fCount);
        fSrc.reset(fCount);

        SkRandom rand;
        for (int i = 0; i < fCount; i++) {
            fSrc[i] = rand.nextU();
        }
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
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

// Let the compiler's autovectorizer do what it thinks is best.
static void memcpy32_autovectorize(uint32_t* dst, const uint32_t* src, int count) {
    while (count --> 0) {
        *dst++ = *src++;
    }
}
BENCH(memcpy32_autovectorize, 10)
BENCH(memcpy32_autovectorize, 100)
BENCH(memcpy32_autovectorize, 1000)
BENCH(memcpy32_autovectorize, 10000)
BENCH(memcpy32_autovectorize, 100000)

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

// Align dst to 16 bytes, then use aligned stores.  src isn't algined, so use unaligned loads.
static void memcpy32_sse2_align(uint32_t* dst, const uint32_t* src, int count) {
    if (count >= 16) {
        while (uintptr_t(dst) & 0xF) {
            *dst++ = *src++;
            count--;
        }

        __m128i* dst128 = reinterpret_cast<__m128i*>(dst);
        const __m128i* src128 = reinterpret_cast<const __m128i*>(src);
        dst += 16 * (count / 16);
        src += 16 * (count / 16);
        while (count >= 16) {
            __m128i a = _mm_loadu_si128(src128++);
            __m128i b = _mm_loadu_si128(src128++);
            __m128i c = _mm_loadu_si128(src128++);
            __m128i d = _mm_loadu_si128(src128++);

            _mm_store_si128(dst128++, a);
            _mm_store_si128(dst128++, b);
            _mm_store_si128(dst128++, c);
            _mm_store_si128(dst128++, d);

            count -= 16;
        }
    }

    while (count --> 0) {
        *dst++ = *src++;
    }
}
BENCH(memcpy32_sse2_align, 10)
BENCH(memcpy32_sse2_align, 100)
BENCH(memcpy32_sse2_align, 1000)
BENCH(memcpy32_sse2_align, 10000)
BENCH(memcpy32_sse2_align, 100000)

// Leave both dst and src unaliged, and so use unaligned stores for dst and unaligned loads for src.
static void memcpy32_sse2_unalign(uint32_t* dst, const uint32_t* src, int count) {
    __m128i* dst128 = reinterpret_cast<__m128i*>(dst);
    const __m128i* src128 = reinterpret_cast<const __m128i*>(src);
    dst += 16 * (count / 16);
    src += 16 * (count / 16);
    while (count >= 16) {
        __m128i a = _mm_loadu_si128(src128++);
        __m128i b = _mm_loadu_si128(src128++);
        __m128i c = _mm_loadu_si128(src128++);
        __m128i d = _mm_loadu_si128(src128++);

        _mm_storeu_si128(dst128++, a);
        _mm_storeu_si128(dst128++, b);
        _mm_storeu_si128(dst128++, c);
        _mm_storeu_si128(dst128++, d);

        count -= 16;
    }

    while (count --> 0) {
        *dst++ = *src++;
    }
}
BENCH(memcpy32_sse2_unalign, 10)
BENCH(memcpy32_sse2_unalign, 100)
BENCH(memcpy32_sse2_unalign, 1000)
BENCH(memcpy32_sse2_unalign, 10000)
BENCH(memcpy32_sse2_unalign, 100000)

// Test our chosen best, from SkUtils.h
BENCH(sk_memcpy32, 10)
BENCH(sk_memcpy32, 100)
BENCH(sk_memcpy32, 1000)
BENCH(sk_memcpy32, 10000)
BENCH(sk_memcpy32, 100000)

#endif // SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

#undef BENCH
