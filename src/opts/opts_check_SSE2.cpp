/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState_opts_SSE2.h"
#include "SkBitmapProcState_opts_SSSE3.h"
#include "SkBitmapFilter_opts_SSE2.h"
#include "SkBlitMask.h"
#include "SkBlitRow.h"
#include "SkBlitRect_opts_SSE2.h"
#include "SkBlitRow_opts_SSE2.h"
#include "SkUtils_opts_SSE2.h"
#include "SkUtils.h"

#include "SkRTConf.h"

#if defined(_MSC_VER) && defined(_WIN64)
#include <intrin.h>
#endif

/* This file must *not* be compiled with -msse or -msse2, otherwise
   gcc may generate sse2 even for scalar ops (and thus give an invalid
   instruction on Pentium3 on the code below).  Only files named *_SSE2.cpp
   in this directory should be compiled with -msse2. */


#ifdef _MSC_VER
static inline void getcpuid(int info_type, int info[4]) {
#if defined(_WIN64)
    __cpuid(info, info_type);
#else
    __asm {
        mov    eax, [info_type]
        cpuid
        mov    edi, [info]
        mov    [edi], eax
        mov    [edi+4], ebx
        mov    [edi+8], ecx
        mov    [edi+12], edx
    }
#endif
}
#else
#if defined(__x86_64__)
static inline void getcpuid(int info_type, int info[4]) {
    asm volatile (
        "cpuid \n\t"
        : "=a"(info[0]), "=b"(info[1]), "=c"(info[2]), "=d"(info[3])
        : "a"(info_type)
    );
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
#endif

#if defined(__x86_64__) || defined(_WIN64) || SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
/* All x86_64 machines have SSE2, or we know it's supported at compile time,  so don't even bother checking. */
static inline bool hasSSE2() {
    return true;
}
#else

static inline bool hasSSE2() {
    int cpu_info[4] = { 0 };
    getcpuid(1, cpu_info);
    return (cpu_info[3] & (1<<26)) != 0;
}
#endif

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
/* If we know SSSE3 is supported at compile time, don't even bother checking. */
static inline bool hasSSSE3() {
    return true;
}
#else

static inline bool hasSSSE3() {
    int cpu_info[4] = { 0 };
    getcpuid(1, cpu_info);
    return (cpu_info[2] & 0x200) != 0;
}
#endif

static bool cachedHasSSE2() {
    static bool gHasSSE2 = hasSSE2();
    return gHasSSE2;
}

static bool cachedHasSSSE3() {
    static bool gHasSSSE3 = hasSSSE3();
    return gHasSSSE3;
}

SK_CONF_DECLARE( bool, c_hqfilter_sse, "bitmap.filter.highQualitySSE", false, "Use SSE optimized version of high quality image filters");

void SkBitmapProcState::platformConvolutionProcs(SkConvolutionProcs* procs) {
    if (cachedHasSSE2()) {
        procs->fExtraHorizontalReads = 3;
        procs->fConvolveVertically = &convolveVertically_SSE2;
        procs->fConvolve4RowsHorizontally = &convolve4RowsHorizontally_SSE2;
        procs->fConvolveHorizontally = &convolveHorizontally_SSE2;
        procs->fApplySIMDPadding = &applySIMDPadding_SSE2;
    }
}

void SkBitmapProcState::platformProcs() {
    if (cachedHasSSSE3()) {
        if (fSampleProc32 == S32_opaque_D32_filter_DX) {
            fSampleProc32 = S32_opaque_D32_filter_DX_SSSE3;
        } else if (fSampleProc32 == S32_alpha_D32_filter_DX) {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSSE3;
        }

        if (fSampleProc32 == S32_opaque_D32_filter_DXDY) {
            fSampleProc32 = S32_opaque_D32_filter_DXDY_SSSE3;
        } else if (fSampleProc32 == S32_alpha_D32_filter_DXDY) {
            fSampleProc32 = S32_alpha_D32_filter_DXDY_SSSE3;
        }
    } else if (cachedHasSSE2()) {
        if (fSampleProc32 == S32_opaque_D32_filter_DX) {
            fSampleProc32 = S32_opaque_D32_filter_DX_SSE2;
        } else if (fSampleProc32 == S32_alpha_D32_filter_DX) {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSE2;
        }

        if (fSampleProc16 == S32_D16_filter_DX) {
            fSampleProc16 = S32_D16_filter_DX_SSE2;
        }
    }

    if (cachedHasSSSE3() || cachedHasSSE2()) {
        if (fMatrixProc == ClampX_ClampY_filter_scale) {
            fMatrixProc = ClampX_ClampY_filter_scale_SSE2;
        } else if (fMatrixProc == ClampX_ClampY_nofilter_scale) {
            fMatrixProc = ClampX_ClampY_nofilter_scale_SSE2;
        }

        if (fMatrixProc == ClampX_ClampY_filter_affine) {
            fMatrixProc = ClampX_ClampY_filter_affine_SSE2;
        } else if (fMatrixProc == ClampX_ClampY_nofilter_affine) {
            fMatrixProc = ClampX_ClampY_nofilter_affine_SSE2;
        }
        if (c_hqfilter_sse) {
            if (fShaderProc32 == highQualityFilter32) {
                fShaderProc32 = highQualityFilter_SSE2;
            }
        }
    }
}

static SkBlitRow::Proc32 platform_32_procs[] = {
    NULL,                               // S32_Opaque,
    S32_Blend_BlitRow32_SSE2,           // S32_Blend,
    S32A_Opaque_BlitRow32_SSE2,         // S32A_Opaque
    S32A_Blend_BlitRow32_SSE2,          // S32A_Blend,
};

SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
    return NULL;
}

SkBlitRow::ColorProc SkBlitRow::PlatformColorProc() {
    if (cachedHasSSE2()) {
        return Color32_SSE2;
    } else {
        return NULL;
    }
}

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    if (cachedHasSSE2()) {
        return platform_32_procs[flags];
    } else {
        return NULL;
    }
}


SkBlitMask::ColorProc SkBlitMask::PlatformColorProcs(SkBitmap::Config dstConfig,
                                                     SkMask::Format maskFormat,
                                                     SkColor color) {
    if (SkMask::kA8_Format != maskFormat) {
        return NULL;
    }

    ColorProc proc = NULL;
    if (cachedHasSSE2()) {
        switch (dstConfig) {
            case SkBitmap::kARGB_8888_Config:
                // The SSE2 version is not (yet) faster for black, so we check
                // for that.
                if (SK_ColorBLACK != color) {
                    proc = SkARGB32_A8_BlitMask_SSE2;
                }
                break;
            default:
                break;
        }
    }
    return proc;
}

SkBlitMask::BlitLCD16RowProc SkBlitMask::PlatformBlitRowProcs16(bool isOpaque) {
    if (cachedHasSSE2()) {
        if (isOpaque) {
            return SkBlitLCD16OpaqueRow_SSE2;
        } else {
            return SkBlitLCD16Row_SSE2;
        }
    } else {
        return NULL;
    }

}
SkBlitMask::RowProc SkBlitMask::PlatformRowProcs(SkBitmap::Config dstConfig,
                                                 SkMask::Format maskFormat,
                                                 RowFlags flags) {
    return NULL;
}

SkMemset16Proc SkMemset16GetPlatformProc() {
    if (cachedHasSSE2()) {
        return sk_memset16_SSE2;
    } else {
        return NULL;
    }
}

SkMemset32Proc SkMemset32GetPlatformProc() {
    if (cachedHasSSE2()) {
        return sk_memset32_SSE2;
    } else {
        return NULL;
    }
}

SkBlitRow::ColorRectProc PlatformColorRectProcFactory(); // suppress warning

SkBlitRow::ColorRectProc PlatformColorRectProcFactory() {
    if (cachedHasSSE2()) {
        return ColorRect32_SSE2;
    } else {
        return NULL;
    }
}
