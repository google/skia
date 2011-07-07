/*
 **
 ** Copyright 2009, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include "SkBitmapProcState_opts_SSE2.h"
#include "SkBlitRow_opts_SSE2.h"
#include "SkUtils_opts_SSE2.h"
#include "SkUtils.h"

/* This file must *not* be compiled with -msse or -msse2, otherwise
   gcc may generate sse2 even for scalar ops (and thus give an invalid
   instruction on Pentium3 on the code below).  Only files named *_SSE2.cpp
   in this directory should be compiled with -msse2. */

#if defined(__x86_64__) || defined(_WIN64)
/* All x86_64 machines have SSE2, so don't even bother checking. */
static inline bool hasSSE2() {
    return true;
}
#else
#ifdef _MSC_VER
static inline void getcpuid(int info_type, int info[4]) {
    __asm {
        mov    eax, [info_type]
        cpuid
        mov    edi, [info]
        mov    [edi], eax
        mov    [edi+4], ebx
        mov    [edi+8], ecx
        mov    [edi+12], edx
    }
}
#else
static inline void getcpuid(int info_type, int info[4]) {
    // We save and restore ebx, so this code can be compatible with -fPIC
    asm volatile (
        "pushl %%ebx      \n\t"
        "cpuid            \n\t"
        "movl %%ebx, %1   \n\t"
        "popl %%ebx       \n\t"
        : "=a"(info[0]), "=r"(info[1]), "=c"(info[2]), "=d"(info[3])
        : "a"(info_type)
    );
}
#endif

static inline bool hasSSE2() {
    int cpu_info[4] = { 0 };
    getcpuid(1, cpu_info);
    return (cpu_info[3] & (1<<26)) != 0;
}
#endif

void SkBitmapProcState::platformProcs() {
    if (hasSSE2()) {
        if (fSampleProc32 == S32_opaque_D32_filter_DX) {
            fSampleProc32 = S32_opaque_D32_filter_DX_SSE2;
        } else if (fSampleProc32 == S32_alpha_D32_filter_DX) {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSE2;
        }
    }
}

static SkBlitRow::Proc32 platform_32_procs[] = {
    NULL,                               // S32_Opaque,
    S32_Blend_BlitRow32_SSE2,           // S32_Blend,
    S32A_Opaque_BlitRow32_SSE2,         // S32A_Opaque
    S32A_Blend_BlitRow32_SSE2,          // S32A_Blend,
};

SkBlitRow::Proc SkBlitRow::PlatformProcs4444(unsigned flags) {
    return NULL;
}

SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
    return NULL;
}

SkBlitRow::ColorProc SkBlitRow::PlatformColorProc() {
    if (hasSSE2()) {
        return Color32_SSE2;
    } else {
        return NULL;
    }
}

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    if (hasSSE2()) {
        return platform_32_procs[flags];
    } else {
        return NULL;
    }
}


SkBlitMask::Proc SkBlitMask::PlatformProcs(SkBitmap::Config dstConfig,
                                           SkColor color) {
    SkBlitMask::Proc proc = NULL;
    if (hasSSE2()) {
        switch (dstConfig) {
            case SkBitmap::kARGB_8888_Config:
                // The SSE2 version is not (yet) faster for black, so we check
                // for that.
                if (SK_ColorBLACK != color) {
                    proc = SkARGB32_BlitMask_SSE2;
                }
                break;
            default:
                 break;
        }
    }
    return proc;
}

SkMemset16Proc SkMemset16GetPlatformProc() {
    if (hasSSE2()) {
        return sk_memset16_SSE2;
    } else {
        return NULL;
    }
}

SkMemset32Proc SkMemset32GetPlatformProc() {
    if (hasSSE2()) {
        return sk_memset32_SSE2;
    } else {
        return NULL;
    }
}
