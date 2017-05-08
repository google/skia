/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_DEFINED
#define SkJumper_DEFINED

// This file contains definitions shared by SkJumper.cpp (compiled normally as part of Skia)
// and SkJumper_stages.cpp (compiled into Skia _and_ offline into SkJumper_generated.h).
// Keep it simple!

#if defined(JUMPER) && (defined(__aarch64__) || defined(__arm__))
    // To reduce SkJumper's dependency on the Android NDK,
    // we provide what we need from <string.h>, <stdint.h>, and <stddef.h> ourselves.
    #define memcpy __builtin_memcpy

    using  int8_t  =   signed char;
    using uint8_t  = unsigned char;
    using  int16_t =   signed short;
    using uint16_t = unsigned short;
    using  int32_t =   signed int;
    using uint32_t = unsigned int;
    #if defined(__aarch64__)
        using  int64_t =   signed long;
        using uint64_t = unsigned long;
        using size_t = uint64_t;
    #else
        using  int64_t =   signed long long;
        using uint64_t = unsigned long long;
        using size_t = uint32_t;
    #endif

    // Now pretend we've included <stdint.h> (or it'll be included again by <arm_neon.h>).
    #define __CLANG_STDINT_H
    #define _STDINT_H_
#else
    #include <string.h>
    #include <stdint.h>
#endif

// SkJumper_stages.cpp has some unusual constraints on what constants it can use.
//
// If the constant is baked into the instruction, that's ok.
// If the constant is synthesized through code, that's ok.
// If the constant is loaded from memory, that's no good.
//
// We offer a couple facilities to get at any other constants you need:
//   - the C() function usually constrains constants to be directly baked into an instruction; or
//   - the _i and _f user-defined literal operators call C() for you in a prettier way; or
//   - you can load values from this struct.

static const int SkJumper_kMaxStride = 8;

struct SkJumper_constants {
    float iota[SkJumper_kMaxStride];   //  0,1,2,3,4,...
};

struct SkJumper_GatherCtx {
    const void*     pixels;
    const uint32_t* ctable;
    int             stride;
};

// State shared by save_xy, accumulate, and bilinear_* / bicubic_*.
struct SkJumper_SamplerCtx {
    float      x[SkJumper_kMaxStride];
    float      y[SkJumper_kMaxStride];
    float     fx[SkJumper_kMaxStride];
    float     fy[SkJumper_kMaxStride];
    float scalex[SkJumper_kMaxStride];
    float scaley[SkJumper_kMaxStride];
};

#endif//SkJumper_DEFINED
