/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCycles_DEFINED
#define SkCycles_DEFINED
#include <cstdint>
class SkCycles {
public:
    static uint64_t Now() {
        #if defined(SK_BUILD_FOR_WIN)
        {
            return 0ul;
        }
        #elif defined(SK_CPU_X86)
        {
            uint32_t cpuInfo;
            return __builtin_ia32_rdtscp(&cpuInfo);
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
