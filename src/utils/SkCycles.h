/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This is an experimental (and probably temporary) solution that allows
 * to compare performance SkVM blitters vs RasterPipeline blitters.
 * In addition to measuring performance (which is questionable) it also produces
 * other counts (pixels, scanlines) and more detailed traces that
 * can explain the current results (SkVM is slower) and help improve it.
 * The entire code is hidden under build flag skia_compare_vm_vs_rp=true
 * and will not appear at all without it.
 */
#ifndef SkCycles_DEFINED
#define SkCycles_DEFINED
#include <cstdint>
#include <x86intrin.h>
class SkCycles {
public:
    static uint64_t Now() {
        #ifndef SKIA_COMPARE_VM_VS_RP
        {
            return 0ul;
        }
        #elif defined(SK_BUILD_FOR_WIN)
        {
            return 0ul;
        }
        #elif defined(SK_BUILD_FOR_IOS)
        {
            return 0ul;
        }
        #elif defined(SK_BUILD_FOR_ANDROID)
        {
            return 0ul;
        }
        #elif defined(SK_CPU_X86)
        {
            unsigned aux;
            return __rdtscp(&aux);
        }
        #elif defined(SK_CPU_ARM64)
        {
            int64_t cycles;
            asm volatile("mrs %0, cntvct_el0" : "=r"(cycles));
            return cycles;
        }
        #else
        {
            return 0ul;
        }
        #endif
    }
};
#endif  // SkCycles_DEFINED
