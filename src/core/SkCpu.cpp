/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkOnce.h"
#include "src/core/SkCpu.h"

#if defined(SK_CPU_X86)
    #if defined(SK_BUILD_FOR_WIN)
        #include <intrin.h>
        static void cpuid (uint32_t abcd[4]) { __cpuid  ((int*)abcd, 1);    }
        static void cpuid7(uint32_t abcd[4]) { __cpuidex((int*)abcd, 7, 0); }
        static uint64_t xgetbv(uint32_t xcr) { return _xgetbv(xcr); }
    #else
        #include <cpuid.h>
        #if !defined(__cpuid_count)  // Old Mac Clang doesn't have this defined.
            #define  __cpuid_count(eax, ecx, a, b, c, d) \
                __asm__("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "0"(eax), "2"(ecx))
        #endif
        static void cpuid (uint32_t abcd[4]) { __get_cpuid(1, abcd+0, abcd+1, abcd+2, abcd+3); }
        static void cpuid7(uint32_t abcd[4]) {
            __cpuid_count(7, 0, abcd[0], abcd[1], abcd[2], abcd[3]);
        }
        static uint64_t xgetbv(uint32_t xcr) {
            uint32_t eax, edx;
            __asm__ __volatile__ ( "xgetbv" : "=a"(eax), "=d"(edx) : "c"(xcr));
            return (uint64_t)(edx) << 32 | eax;
        }
    #endif

    static uint32_t read_cpu_features() {
        uint32_t features = 0;
        uint32_t abcd[4] = {0,0,0,0};

        // You might want to refer to http://www.sandpile.org/x86/cpuid.htm

        cpuid(abcd);
        if (abcd[3] & (1<<25)) { features |= SkCpu:: SSE1; }
        if (abcd[3] & (1<<26)) { features |= SkCpu:: SSE2; }
        if (abcd[2] & (1<< 0)) { features |= SkCpu:: SSE3; }
        if (abcd[2] & (1<< 9)) { features |= SkCpu::SSSE3; }
        if (abcd[2] & (1<<19)) { features |= SkCpu::SSE41; }
        if (abcd[2] & (1<<20)) { features |= SkCpu::SSE42; }

        if ((abcd[2] & (3<<26)) == (3<<26)         // XSAVE + OSXSAVE
             && (xgetbv(0) & (3<<1)) == (3<<1)) {  // XMM and YMM state enabled.
            if (abcd[2] & (1<<28)) { features |= SkCpu:: AVX; }
            if (abcd[2] & (1<<29)) { features |= SkCpu::F16C; }
            if (abcd[2] & (1<<12)) { features |= SkCpu:: FMA; }

            cpuid7(abcd);
            if (abcd[1] & (1<<5)) { features |= SkCpu::AVX2; }
            if (abcd[1] & (1<<3)) { features |= SkCpu::BMI1; }
            if (abcd[1] & (1<<8)) { features |= SkCpu::BMI2; }

            if ((xgetbv(0) & (7<<5)) == (7<<5)) {  // All ZMM state bits enabled too.
                if (abcd[1] & (1<<16)) { features |= SkCpu::AVX512F; }
                if (abcd[1] & (1<<17)) { features |= SkCpu::AVX512DQ; }
                if (abcd[1] & (1<<21)) { features |= SkCpu::AVX512IFMA; }
                if (abcd[1] & (1<<26)) { features |= SkCpu::AVX512PF; }
                if (abcd[1] & (1<<27)) { features |= SkCpu::AVX512ER; }
                if (abcd[1] & (1<<28)) { features |= SkCpu::AVX512CD; }
                if (abcd[1] & (1<<30)) { features |= SkCpu::AVX512BW; }
                if (abcd[1] & (1<<31)) { features |= SkCpu::AVX512VL; }
            }
        }
        return features;
    }

#elif defined(SK_CPU_ARM64) && __has_include(<sys/auxv.h>)
    #include <sys/auxv.h>

    static uint32_t read_cpu_features() {
        const uint32_t kHWCAP_CRC32   = (1<< 7),
                       kHWCAP_ASIMDHP = (1<<10);

        uint32_t features = 0;
        uint32_t hwcaps = getauxval(AT_HWCAP);
        if (hwcaps & kHWCAP_CRC32  ) { features |= SkCpu::CRC32; }
        if (hwcaps & kHWCAP_ASIMDHP) { features |= SkCpu::ASIMDHP; }

        // The Samsung Mongoose 3 core sets the ASIMDHP bit but doesn't support it.
        for (int core = 0; features & SkCpu::ASIMDHP; core++) {
            // These /sys files contain the core's MIDR_EL1 register, the source of
            // CPU {implementer, variant, part, revision} you'd see in /proc/cpuinfo.
            SkString path =
                SkStringPrintf("/sys/devices/system/cpu/cpu%d/regs/identification/midr_el1", core);

            // Can't use SkData::MakeFromFileName() here, I think because /sys can't be mmap()'d.
            SkFILEStream midr_el1(path.c_str());
            if (!midr_el1.isValid()) {
                // This is our ordinary exit path.
                // If we ask for MIDR_EL1 from a core that doesn't exist, we've checked all cores.
                if (core == 0) {
                    // On the other hand, if we can't read MIDR_EL1 from any core, assume the worst.
                    features &= ~(SkCpu::ASIMDHP);
                }
                break;
            }

            const char kMongoose3[] = "0x00000000531f0020";  // 53 == Samsung.
            char buf[SK_ARRAY_COUNT(kMongoose3) - 1];  // No need for the terminating \0.

            if (SK_ARRAY_COUNT(buf) != midr_el1.read(buf, SK_ARRAY_COUNT(buf))
                          || 0 == memcmp(kMongoose3, buf, SK_ARRAY_COUNT(buf))) {
                features &= ~(SkCpu::ASIMDHP);
            }
        }
        return features;
    }

#elif defined(SK_CPU_ARM32) && __has_include(<sys/auxv.h>) && \
    (!defined(__ANDROID_API__) || __ANDROID_API__ >= 18)
    // sys/auxv.h will always be present in the Android NDK due to unified
    //headers, but getauxval is only defined for API >= 18.
    #include <sys/auxv.h>

    static uint32_t read_cpu_features() {
        const uint32_t kHWCAP_NEON  = (1<<12);
        const uint32_t kHWCAP_VFPv4 = (1<<16);

        uint32_t features = 0;
        uint32_t hwcaps = getauxval(AT_HWCAP);
        if (hwcaps & kHWCAP_NEON ) {
            features |= SkCpu::NEON;
            if (hwcaps & kHWCAP_VFPv4) { features |= SkCpu::NEON_FMA|SkCpu::VFP_FP16; }
        }
        return features;
    }

#elif defined(SK_CPU_ARM32) && __has_include(<cpu-features.h>)
    #include <cpu-features.h>

    static uint32_t read_cpu_features() {
        uint32_t features = 0;
        uint64_t cpu_features = android_getCpuFeatures();
        if (cpu_features & ANDROID_CPU_ARM_FEATURE_NEON)     { features |= SkCpu::NEON; }
        if (cpu_features & ANDROID_CPU_ARM_FEATURE_NEON_FMA) { features |= SkCpu::NEON_FMA; }
        if (cpu_features & ANDROID_CPU_ARM_FEATURE_VFP_FP16) { features |= SkCpu::VFP_FP16; }
        return features;
    }

#else
    static uint32_t read_cpu_features() {
        return 0;
    }

#endif

uint32_t SkCpu::gCachedFeatures = 0;

void SkCpu::CacheRuntimeFeatures() {
    static SkOnce once;
    once([] { gCachedFeatures = read_cpu_features(); });
}
