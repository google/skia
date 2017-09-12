/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This restricted SkJumper backend works on 8-bit per channel pixels stored in
// 16-bit channels.  This is a last attempt to write a performant low-precision
// backend with stage definitions that can be shared by x86 and ARM.

#include "SkJumper.h"

#define SI static inline

#if defined(__ARM_NEON)
    #include <arm_neon.h>

    #if defined(__arm__)
        #define ABI __attribute__((pcs("aapcs-vfp")))
    #else
        #define ABI
    #endif

    #define N 8
    #define WRAP(name) sk_##name##_neon_8bit
#else
    #include <immintrin.h>

    #define ABI

    #if defined(__AVX2__)
        #define N 16
        #define WRAP(name) sk_##name##_hsw_8bit
    #elif defined(__SSE4_1__)
        #define N 8
        #define WRAP(name) sk_##name##_sse41_8bit
    #elif defined(__SSE2__)
        #define N 8
        #define WRAP(name) sk_##name##_sse2_8bit
    #endif
#endif

using U8  = uint8_t  __attribute__((ext_vector_type(N)));
using U16 = uint16_t __attribute__((ext_vector_type(N)));
using U32 = uint32_t __attribute__((ext_vector_type(N)));
using I16 =  int16_t __attribute__((ext_vector_type(N)));

// We pass program as the second argument so that load_and_inc() will find it in %rsi on x86-64.
using Stage = void (ABI*)(size_t tail, void** program, size_t x, size_t y,
                          I16  r, I16  g, I16  b, I16  a,
                          I16 dr, I16 dg, I16 db, I16 da);

SI void* load_and_inc(void**& program) {
#if defined(__x86_64__)
    // If program is in %rsi, this is a single-instruction *program++.
    void* rax;
    asm("lodsq" : "=a"(rax), "+S"(program));
    return rax;
#else
    return *program++;
#endif
}

MAYBE_MSABI
ABI extern "C" void WRAP(start_pipeline)(const size_t x0,
                                         const size_t y0,
                                         const size_t xlimit,
                                         const size_t ylimit,
                                         void** program) {
    auto start = (Stage)load_and_inc(program);
    for (size_t y = y0; y < ylimit; y++) {
        size_t x = x0;
        for (; x + N <= xlimit; x += N) {
            start(   0,program,x,y, 0,0,0,0, 0,0,0,0);
        }
        if (size_t tail = xlimit - x) {
            start(tail,program,x,y, 0,0,0,0, 0,0,0,0);
        }
    }
}

ABI extern "C" void WRAP(just_return)(size_t,void**,size_t,size_t,
                                      I16,I16,I16,I16, I16,I16,I16,I16) {}

// Lazily resolved on first cast.  Does nothing if cast to Ctx::None.
struct Ctx {
    using None = decltype(nullptr);

    void*   ptr;
    void**& program;

    explicit Ctx(void**& p) : ptr(nullptr), program(p) {}

    template <typename T>
    operator T*() {
        if (!ptr) { ptr = load_and_inc(program); }
        return (T*)ptr;
    }
    operator None() { return nullptr; }
};

#define STAGE(name, ...)                                                               \
    SI void name##_k(__VA_ARGS__, size_t x, size_t y, size_t tail,                     \
                     I16&  r, I16&  g, I16&  b, I16&  a,                               \
                     I16& dr, I16& dg, I16& db, I16& da);                              \
    ABI extern "C" void WRAP(name)(size_t tail, void** program, size_t x, size_t y,    \
                                   I16  r, I16  g, I16  b, I16  a,                     \
                                   I16 dr, I16 dg, I16 db, I16 da) {                   \
        name##_k(Ctx{program}, x,y,tail, r,g,b,a, dr,dg,db,da);                        \
        auto next = (Stage)load_and_inc(program);                                      \
        next(tail,program,x,y, r,g,b,a, dr,dg,db,da);                                  \
    }                                                                                  \
    SI void name##_k(__VA_ARGS__, size_t x, size_t y, size_t tail,                     \
                     I16&  r, I16&  g, I16&  b, I16&  a,                               \
                     I16& dr, I16& dg, I16& db, I16& da)

STAGE(seed_shader, Ctx::None) {

}
