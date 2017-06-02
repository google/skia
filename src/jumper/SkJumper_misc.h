/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_misc_DEFINED
#define SkJumper_misc_DEFINED

#include "SkJumper.h"  // for memcpy()

// Miscellany used by SkJumper_stages.cpp and SkJumper_vectors.h.

// Every function in this file should be marked static and inline using SI.
#if defined(JUMPER)
    #define SI __attribute__((always_inline)) static inline
#else
    #define SI static inline
#endif


template <typename T, typename P>
SI T unaligned_load(const P* p) {  // const void* would work too, but const P* helps ARMv7 codegen.
    T v;
    memcpy(&v, p, sizeof(v));
    return v;
}

template <typename T, typename P>
SI void unaligned_store(P* p, T v) {
    memcpy(p, &v, sizeof(v));
}

template <typename Dst, typename Src>
SI Dst bit_cast(const Src& src) {
    static_assert(sizeof(Dst) == sizeof(Src), "");
    return unaligned_load<Dst>(&src);
}

template <typename Dst, typename Src>
SI Dst widen_cast(const Src& src) {
    static_assert(sizeof(Dst) > sizeof(Src), "");
    Dst dst;
    memcpy(&dst, &src, sizeof(Src));
    return dst;
}

// Our program is an array of void*, either
//   - 1 void* per stage with no context pointer, the next stage;
//   - 2 void* per stage with a context pointer, first the context pointer, then the next stage.

// load_and_inc() steps the program forward by 1 void*, returning that pointer.
SI void* load_and_inc(void**& program) {
#if defined(__GNUC__) && defined(__x86_64__)
    // If program is in %rsi (we try to make this likely) then this is a single instruction.
    void* rax;
    asm("lodsq" : "=a"(rax), "+S"(program));  // Write-only %rax, read-write %rsi.
    return rax;
#else
    // On ARM *program++ compiles into pretty ideal code without any handholding.
    return *program++;
#endif
}

// LazyCtx doesn't do anything unless you call operator T*(), encapsulating the logic
// from above that stages without a context pointer are represented by just 1 void*.
struct LazyCtx {
    void*   ptr;
    void**& program;

    explicit LazyCtx(void**& p) : ptr(nullptr), program(p) {}

    template <typename T>
    operator T*() {
        if (!ptr) { ptr = load_and_inc(program); }
        return (T*)ptr;
    }
};

#endif//SkJumper_misc_DEFINED
