/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

// skcms_internals.h contains APIs shared by skcms' internals and its test tools.
// Please don't use this header from outside the skcms repo.

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ~~~~ General Helper Macros ~~~~
// skcms can leverage some C++ extensions when they are present.
#define ARRAY_COUNT(arr) (int)(sizeof((arr)) / sizeof(*(arr)))

#if defined(__clang__) && defined(__has_cpp_attribute)
    #if __has_cpp_attribute(clang::fallthrough)
        #define SKCMS_FALLTHROUGH [[clang::fallthrough]]
    #endif

    #if __has_cpp_attribute(clang::musttail) && !__has_feature(memory_sanitizer) \
                                             && !__has_feature(address_sanitizer)
        // Sanitizers do not work well with [[clang::musttail]], and corrupt the src/dst pointers.
        #define SKCMS_MUSTTAIL [[clang::musttail]]
    #else
        #define SKCMS_MUSTTAIL
    #endif
#endif

#ifndef SKCMS_FALLTHROUGH
    #define SKCMS_FALLTHROUGH
#endif
#ifndef SKCMS_MUSTTAIL
    #define SKCMS_MUSTTAIL
#endif

#if defined(__clang__)
    #define SKCMS_MAYBE_UNUSED __attribute__((unused))
    #pragma clang diagnostic ignored "-Wused-but-marked-unused"
#elif defined(__GNUC__)
    #define SKCMS_MAYBE_UNUSED __attribute__((unused))
#elif defined(_MSC_VER)
    #define SKCMS_MAYBE_UNUSED __pragma(warning(suppress:4100))
#else
    #define SKCMS_MAYBE_UNUSED
#endif

// sizeof(x) will return size_t, which is 32-bit on some machines and 64-bit on others.
// We have better testing on 64-bit machines, so force 32-bit machines to behave like 64-bit.
//
// Please do not use sizeof() directly, and size_t only when required.
// (We have no way of enforcing these requests...)
#define SAFE_SIZEOF(x) ((uint64_t)sizeof(x))

// Same sort of thing for _Layout structs with a variable sized array at the end (named "variable").
#define SAFE_FIXED_SIZE(type) ((uint64_t)offsetof(type, variable))

// ~~~~ Shared ~~~~
typedef struct skcms_ICCTag {
    uint32_t       signature;
    uint32_t       type;
    uint32_t       size;
    const uint8_t* buf;
} skcms_ICCTag;

void skcms_GetTagByIndex    (const skcms_ICCProfile*, uint32_t idx, skcms_ICCTag*);
bool skcms_GetTagBySignature(const skcms_ICCProfile*, uint32_t sig, skcms_ICCTag*);

float skcms_MaxRoundtripError(const skcms_Curve* curve, const skcms_TransferFunction* inv_tf);

// 252 of a random shuffle of all possible bytes.
// 252 is evenly divisible by 3 and 4.  Only 192, 10, 241, and 43 are missing.
// Used for ICC profile equivalence testing.
extern const uint8_t skcms_252_random_bytes[252];

// ~~~~ Portable Math ~~~~
static inline float floorf_(float x) {
    float roundtrip = (float)((int)x);
    return roundtrip > x ? roundtrip - 1 : roundtrip;
}
static inline float fabsf_(float x) { return x < 0 ? -x : x; }
float powf_(float, float);

#ifdef __cplusplus
}
#endif
