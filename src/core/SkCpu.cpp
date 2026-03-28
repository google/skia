/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This code detects what features the CPU we are currently running on has using cpuid
 * (a built-in x86 instruction). The canonical source for the magic numbers/bits is
 * Intel® Architecture Instruction Set Extensions Programming Reference (specifically
 * 1.4 DETECTION OF FUTURE INSTRUCTIONS AND FEATURES). https://www.sandpile.org/x86/cpuid.htm
 * also visualizes this and is easier to reference.
 * Intel® 64 and IA-32 Architectures Software Developer's Manual gives more details
 * for some of the more intricate detection of features.
 */

#include "src/core/SkCpu.h"

#include "include/private/base/SkFeatures.h"
#include "include/private/base/SkOnce.h"

#if defined(SK_CPU_X86)
    #if defined(_MSC_VER)
        #include <intrin.h>
    #else
        #include <cpuid.h>
    #endif
#elif defined(SK_CPU_LOONGARCH)
    #include <sys/auxv.h>
#endif

namespace {
#if defined(SK_CPU_X86)
    // Family/Model/Stepping/Feature Flags https://www.sandpile.org/x86/cpuid.htm#leaf_0000_0001h
    constexpr uint32_t kFMSFLeaf = 1;
    // Feature Flags https://www.sandpile.org/x86/cpuid.htm#leaf_0000_0007h
    constexpr uint32_t kFlagsLeaf = 7;
    constexpr uint32_t kFlagsSubleaf = 0;

    #if defined(_MSC_VER)
        // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex
        void cpu_features(uint32_t abcd[4]) { __cpuid  ((int*)abcd, kFMSFLeaf); }
        void cpu_flags(uint32_t abcd[4]) { __cpuidex((int*)abcd, kFlagsLeaf, kFlagsSubleaf); }
        uint64_t xgetbv() {
            // https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
            constexpr uint32_t xcr = 0;
            return _xgetbv(xcr);
        }
    #else
        #if !defined(__cpuid_count)  // Old Mac Clang doesn't have this defined.
            #define  __cpuid_count(eax, ecx, a, b, c, d) \
                __asm__("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "0"(eax), "2"(ecx))
        #endif
        // https://www.felixcloutier.com/x86/cpuid
        void cpu_features(uint32_t abcd[4]) {
            __cpuid(kFMSFLeaf, abcd[0], abcd[1], abcd[2], abcd[3]);
        }
        void cpu_flags(uint32_t abcd[4]) {
            // __cpuid_count is just like __cpuid except it also takes a subleaf option (which
            // we set to 0, as that's where the info we care about is).
            __cpuid_count(kFlagsLeaf, kFlagsSubleaf, abcd[0], abcd[1], abcd[2], abcd[3]);
        }
        uint64_t xgetbv() {
            // https://www.felixcloutier.com/x86/xgetbv
            // "Execute XGETBV with ECX = 0 to discover the value of XCR0 ...
            constexpr uint32_t xcr = 0;
            uint32_t eax, edx;
            __asm__ __volatile__ ( "xgetbv" : "=a"(eax), "=d"(edx) : "c"(xcr));
            return (uint64_t)(edx) << 32 | eax;
        }
    #endif

    // The comments on each of the below lines correspond to constants defined in <cpuid.h>

    // cpuid(1) -> EDX
    constexpr uint32_t kSSE     = (1u << 25); // bit_SSE
    constexpr uint32_t kSSE2    = (1u << 26); // bit_SSE2

    // cpuid(1) -> ECX
    constexpr uint32_t kSSE3    = (1u <<  0); // bit_SSE3
    constexpr uint32_t kSSSE3   = (1u <<  9); // bit_SSSE3
    constexpr uint32_t kSSE41   = (1u << 19); // bit_SSE4_1
    constexpr uint32_t kSSE42   = (1u << 20); // bit_SSE4_2
    constexpr uint32_t kFMA     = (1u << 12); // bit_FMA
    constexpr uint32_t kAVX     = (1u << 28); // bit_AVX
    constexpr uint32_t kF16C    = (1u << 29); // bit_F16C
    constexpr uint32_t kXSAVE   = (1u << 26); // bit_XSAVE
    constexpr uint32_t kOSXSAVE = (1u << 27); // bit_OSXSAVE

    // cpuid(7,0) -> EBX
    constexpr uint32_t kBMI1       = (1u <<  3); // bit_BMI
    constexpr uint32_t kAVX2       = (1u <<  5); // bit_AVX2
    constexpr uint32_t kBMI2       = (1u <<  8); // bit_BMI2
    constexpr uint32_t kERMS       = (1u <<  9); // bit_ENH_MOVSB
    constexpr uint32_t kAVX512F    = (1u << 16); // bit_AVX512F
    constexpr uint32_t kAVX512DQ   = (1u << 17); // bit_AVX512DQ
    constexpr uint32_t kAVX512IFMA = (1u << 21); // bit_AVX512IFMA
    constexpr uint32_t kAVX512PF   = (1u << 26); // bit_AVX512PF
    constexpr uint32_t kAVX512ER   = (1u << 27); // bit_AVX512ER
    constexpr uint32_t kAVX512CD   = (1u << 28); // bit_AVX512CD
    constexpr uint32_t kAVX512BW   = (1u << 30); // bit_AVX512BW
    constexpr uint32_t kAVX512VL   = (1u << 31); // bit_AVX512VL

    // xgetbv(0) -> XCR (eXtended Control Register)
    constexpr uint64_t kXCR0_XMM_YMM_STATE = 0b00000110; // Bits 1 and 2
    constexpr uint64_t kXCR0_ZMM_STATE     = 0b11100000; // Bits 5, 6, and 7

    uint32_t read_cpu_features() {
        uint32_t features = 0;
        uint32_t abcd[4] = {0,0,0,0};

        #define EAX abcd[0]
        #define EBX abcd[1]
        #define ECX abcd[2]
        #define EDX abcd[3]

        cpu_features(abcd);
        if (EDX & kSSE)    { features |= SkCpu:: SSE1; }
        if (EDX & kSSE2)   { features |= SkCpu:: SSE2; }
        if (ECX & kSSE3)   { features |= SkCpu:: SSE3; }
        if (ECX & kSSSE3)  { features |= SkCpu::SSSE3; }
        if (ECX & kSSE41)  { features |= SkCpu::SSE41; }
        if (ECX & kSSE42)  { features |= SkCpu::SSE42; }

        // From Intel® 64 and IA-32 Architectures Software Developer's Manual
        // 14.3 DETECTION OF INTEL® AVX INSTRUCTIONS
        // 1) Detect we have XGETBV, i.e. cpuid(1) returns ECX with bit 27 set
        //    (In Intel's docs, this is represented by CPUID.01H:ECX.OSXSAVE[27] = 1)
        //    For historical reasons, we check bit 26 (kXSAVE; see
        //    https://codereview.chromium.org/1428153003) which is the hardware support. Bit 27
        //    (kOSXSAVE) means the OS *also* supports this, which the Intel docs say is important.
        if ((ECX & (kXSAVE | kOSXSAVE)) == (kXSAVE | kOSXSAVE)) {
            // 2) Call XGETBV to get eXtended Control Register info.
            const uint64_t xcr = xgetbv();
            if ((xcr & kXCR0_XMM_YMM_STATE) == kXCR0_XMM_YMM_STATE) {
                // 3) Check ECX bit 28 for AVX support
                if (ECX & kAVX)  { features |= SkCpu:: AVX; }
                // Now that we have AVX, we can detect other features that list it as a prereq.
                if (ECX & kF16C) { features |= SkCpu::F16C; } // 14.4.1 Detection of F16C Instr...
                if (ECX & kFMA)  { features |= SkCpu:: FMA; } // 14.5.3 Detection of FMA

                // Fill the register values with values from leaf 7.
                cpu_flags(abcd);
                if (EBX & kAVX2)  { features |= SkCpu::AVX2; } // 14.7.1 Detection of Intel® AVX2

                // These don't strictly require AVX support, but only exist on newer chips anyway.
                if (EBX & kBMI1)  { features |= SkCpu::BMI1; }
                if (EBX & kBMI2)  { features |= SkCpu::BMI2; }
                if (EBX & kERMS)  { features |= SkCpu::ERMS; }

                // 15.2 DETECTION OF AVX-512 FOUNDATION INSTRUCTIONS
                // 1) Detect we have XGETBV (which we did above).
                // 2) Execute XGETBV and verify that XCR0[7:5] = '111b' ...
                //    and that XCR0[2:1] = 11b' (which we did above)
                if ((xcr & kXCR0_ZMM_STATE) == kXCR0_ZMM_STATE) {
                    // 3) Check EBX bit 16 for AVX support
                    if (EBX & kAVX512F)    { features |= SkCpu::AVX512F; }
                    // ... and any other extensions.
                    if (EBX & kAVX512DQ)   { features |= SkCpu::AVX512DQ; }
                    if (EBX & kAVX512IFMA) { features |= SkCpu::AVX512IFMA; }
                    if (EBX & kAVX512PF)   { features |= SkCpu::AVX512PF; }
                    if (EBX & kAVX512ER)   { features |= SkCpu::AVX512ER; }
                    if (EBX & kAVX512CD)   { features |= SkCpu::AVX512CD; }
                    if (EBX & kAVX512BW)   { features |= SkCpu::AVX512BW; }
                    if (EBX & kAVX512VL)   { features |= SkCpu::AVX512VL; }
                }
            }
        }
        return features;
    }
#elif defined(SK_CPU_LOONGARCH)
    uint32_t read_cpu_features() {
        uint32_t features = 0;
        uint64_t hwcap = getauxval(AT_HWCAP);

        if (hwcap & HWCAP_LOONGARCH_LSX)  { features |= SkCpu::LOONGARCH_SX; }
        if (hwcap & HWCAP_LOONGARCH_LASX) { features |= SkCpu::LOONGARCH_ASX; }

        return features;
    }
#else
    uint32_t read_cpu_features() {
        return 0;
    }
#endif
}  // anonymous namespace

uint32_t SkCpu::gCachedFeatures = 0;

void SkCpu::CacheRuntimeFeatures() {
    static SkOnce once;
    once([] { gCachedFeatures = read_cpu_features(); });
}
