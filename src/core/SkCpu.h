/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCpu_DEFINED
#define SkCpu_DEFINED

#include "include/private/base/SkFeatures.h"

#include <cstdint>

struct SkX64 {
    constexpr static uint32_t SSE1       = 1 << 0;
    constexpr static uint32_t SSE2       = 1 << 1;
    constexpr static uint32_t SSE3       = 1 << 2;
    constexpr static uint32_t SSSE3      = 1 << 3;
    constexpr static uint32_t SSE41      = 1 << 4;
    constexpr static uint32_t SSE42      = 1 << 5;
    constexpr static uint32_t AVX        = 1 << 6;
    constexpr static uint32_t F16C       = 1 << 7;
    constexpr static uint32_t FMA        = 1 << 8;
    constexpr static uint32_t AVX2       = 1 << 9;
    constexpr static uint32_t BMI1       = 1 << 10;
    constexpr static uint32_t BMI2       = 1 << 11;
    constexpr static uint32_t AVX512F    = 1 << 12;
    constexpr static uint32_t AVX512DQ   = 1 << 13;
    constexpr static uint32_t AVX512IFMA = 1 << 14;
    constexpr static uint32_t AVX512PF   = 1 << 15;
    constexpr static uint32_t AVX512ER   = 1 << 16;
    constexpr static uint32_t AVX512CD   = 1 << 17;
    constexpr static uint32_t AVX512BW   = 1 << 18;
    constexpr static uint32_t AVX512VL   = 1 << 19;
    constexpr static uint32_t ERMS       = 1 << 20;

    // Some of these are grouped together into "levels"
    // https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
    // -march=x86-64-v3
    constexpr static uint32_t ML3 = AVX2 | BMI1 | BMI2 | F16C | FMA;
    // -march=x86-64-v4
    constexpr static uint32_t ML4 = AVX512F  | AVX512DQ | AVX512CD | AVX512BW | AVX512VL;
};

struct SkLoongArch {
    constexpr static uint32_t SX  = 1 << 0;
    constexpr static uint32_t ASX = 1 << 1;
};

struct SkCpu {
    static void CacheRuntimeFeatures();
    static bool Supports(uint32_t);
private:
    static uint32_t gCachedFeatures;
};

inline bool SkCpu::Supports(uint32_t mask) {
    uint32_t features = gCachedFeatures;

    // If we mask in compile-time known lower limits, the compiler can
    // often compile away this entire function.
#if SK_CPU_X86
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE1
    features |= SkX64::SSE1;
    #endif
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE2
    features |= SkX64::SSE2;
    #endif
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE3
    features |= SkX64::SSE3;
    #endif
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSSE3
    features |= SkX64::SSSE3;
    #endif
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE41
    features |= SkX64::SSE41;
    #endif
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE42
    features |= SkX64::SSE42;
    #endif
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX
    features |= SkX64::AVX;
    #endif
    // F16C goes here if we add SK_CPU_X64_LEVEL_F16C
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX2
    features |= SkX64::AVX2;
    #endif
    #if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_ML4
    features |= (SkX64::AVX512F | SkX64::AVX512DQ | SkX64::AVX512CD | SkX64::AVX512BW | SkX64::AVX512VL);
    #endif
    // FMA doesn't fit neatly into this total ordering.
    // It's available on Haswell+ just like AVX2, but it's technically a different bit.
    // TODO: circle back on this if we find ourselves limited by lack of compile-time FMA

    #if defined(SK_CPU_LIMIT_AVX)
    features &= (SkX64::SSE1 | SkX64::SSE2 | SkX64::SSE3 | SkX64::SSSE3 | SkX64::SSE41 | SkX64::SSE42 | SkX64::AVX);
    #elif defined(SK_CPU_LIMIT_SSE41)
    features &= (SkX64::SSE1 | SkX64::SSE2 | SkX64::SSE3 | SkX64::SSSE3 | SkX64::SSE41);
    #elif defined(SK_CPU_LIMIT_SSE2)
    features &= (SkX64::SSE1 | SkX64::SSE2);
    #endif

#elif SK_CPU_LOONGARCH
    #if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    features |= SkLoongArch::SX;
    #endif
    #if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    features |= SkLoongArch::ASX;
    #endif

#endif
    return (features & mask) == mask;
}

#endif  // SkCpu_DEFINED
