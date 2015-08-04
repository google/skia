/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapFilter_opts_SSE2.h"
#include "SkBitmapProcState_opts_SSE2.h"
#include "SkBitmapProcState_opts_SSSE3.h"
#include "SkBitmapScaler.h"
#include "SkBlitMask.h"
#include "SkBlitRow.h"
#include "SkBlitRow_opts_SSE2.h"
#include "SkBlitRow_opts_SSE4.h"
#include "SkLazyPtr.h"
#include "SkRTConf.h"

#if defined(_MSC_VER) && defined(_WIN64)
#include <intrin.h>
#endif

/* This file must *not* be compiled with -msse or any other optional SIMD
   extension, otherwise gcc may generate SIMD instructions even for scalar ops
   (and thus give an invalid instruction on Pentium3 on the code below).
   For example, only files named *_SSE2.cpp in this directory should be
   compiled with -msse2 or higher. */


/* Function to get the CPU SSE-level in runtime, for different compilers. */
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
#elif defined(__x86_64__)
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

////////////////////////////////////////////////////////////////////////////////

/* Fetch the SIMD level directly from the CPU, at run-time.
 * Only checks the levels needed by the optimizations in this file.
 */
namespace {  // get_SIMD_level() technically must have external linkage, so no static.
int* get_SIMD_level() {
    int cpu_info[4] = { 0, 0, 0, 0 };
    getcpuid(1, cpu_info);

    int* level = SkNEW(int);

    if ((cpu_info[2] & (1<<20)) != 0) {
        *level = SK_CPU_SSE_LEVEL_SSE42;
    } else if ((cpu_info[2] & (1<<19)) != 0) {
        *level = SK_CPU_SSE_LEVEL_SSE41;
    } else if ((cpu_info[2] & (1<<9)) != 0) {
        *level = SK_CPU_SSE_LEVEL_SSSE3;
    } else if ((cpu_info[3] & (1<<26)) != 0) {
        *level = SK_CPU_SSE_LEVEL_SSE2;
    } else {
        *level = 0;
    }
    return level;
}
} // namespace

SK_DECLARE_STATIC_LAZY_PTR(int, gSIMDLevel, get_SIMD_level);

/* Verify that the requested SIMD level is supported in the build.
 * If not, check if the platform supports it.
 */
static inline bool supports_simd(int minLevel) {
#if defined(SK_CPU_SSE_LEVEL)
    if (minLevel <= SK_CPU_SSE_LEVEL) {
        return true;
    } else
#endif
    {
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
        /* For the Android framework we should always know at compile time if the device
         * we are building for supports SSSE3.  The one exception to this rule is on the
         * emulator where we are compiled without the -mssse3 option (so we have no
         * SSSE3 procs) but can be run on a host machine that supports SSSE3
         * instructions. So for that particular case we disable our SSSE3 options.
         */
        return false;
#else
        return minLevel <= *gSIMDLevel.get();
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////

void SkBitmapScaler::PlatformConvolutionProcs(SkConvolutionProcs* procs) {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        procs->fExtraHorizontalReads = 3;
        procs->fConvolveVertically = &convolveVertically_SSE2;
        procs->fConvolve4RowsHorizontally = &convolve4RowsHorizontally_SSE2;
        procs->fConvolveHorizontally = &convolveHorizontally_SSE2;
        procs->fApplySIMDPadding = &applySIMDPadding_SSE2;
    }
}

////////////////////////////////////////////////////////////////////////////////

void SkBitmapProcState::platformProcs() {
    /* Every optimization in the function requires at least SSE2 */
    if (!supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return;
    }
    const bool ssse3 = supports_simd(SK_CPU_SSE_LEVEL_SSSE3);

    /* Check fSampleProc32 */
    if (fSampleProc32 == S32_opaque_D32_filter_DX) {
        if (ssse3) {
            fSampleProc32 = S32_opaque_D32_filter_DX_SSSE3;
        } else {
            fSampleProc32 = S32_opaque_D32_filter_DX_SSE2;
        }
    } else if (fSampleProc32 == S32_opaque_D32_filter_DXDY) {
        if (ssse3) {
            fSampleProc32 = S32_opaque_D32_filter_DXDY_SSSE3;
        }
    } else if (fSampleProc32 == S32_alpha_D32_filter_DX) {
        if (ssse3) {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSSE3;
        } else {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSE2;
        }
    } else if (fSampleProc32 == S32_alpha_D32_filter_DXDY) {
        if (ssse3) {
            fSampleProc32 = S32_alpha_D32_filter_DXDY_SSSE3;
        }
    }

    /* Check fSampleProc16 */
    if (fSampleProc16 == S32_D16_filter_DX) {
        if (ssse3) {
            fSampleProc16 = S32_D16_filter_DX_SSSE3;
        } else {
            fSampleProc16 = S32_D16_filter_DX_SSE2;
        }
    } else if (ssse3 && fSampleProc16 == S32_D16_filter_DXDY) {
        fSampleProc16 = S32_D16_filter_DXDY_SSSE3;
    }

    /* Check fMatrixProc */
    if (fMatrixProc == ClampX_ClampY_filter_scale) {
        fMatrixProc = ClampX_ClampY_filter_scale_SSE2;
    } else if (fMatrixProc == ClampX_ClampY_nofilter_scale) {
        fMatrixProc = ClampX_ClampY_nofilter_scale_SSE2;
    } else if (fMatrixProc == ClampX_ClampY_filter_affine) {
        fMatrixProc = ClampX_ClampY_filter_affine_SSE2;
    } else if (fMatrixProc == ClampX_ClampY_nofilter_affine) {
        fMatrixProc = ClampX_ClampY_nofilter_affine_SSE2;
    }
}

////////////////////////////////////////////////////////////////////////////////

static const SkBlitRow::Proc16 platform_16_procs[] = {
    S32_D565_Opaque_SSE2,               // S32_D565_Opaque
    NULL,                               // S32_D565_Blend
    S32A_D565_Opaque_SSE2,              // S32A_D565_Opaque
    NULL,                               // S32A_D565_Blend
    S32_D565_Opaque_Dither_SSE2,        // S32_D565_Opaque_Dither
    NULL,                               // S32_D565_Blend_Dither
    S32A_D565_Opaque_Dither_SSE2,       // S32A_D565_Opaque_Dither
    NULL,                               // S32A_D565_Blend_Dither
};

SkBlitRow::Proc16 SkBlitRow::PlatformFactory565(unsigned flags) {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return platform_16_procs[flags];
    } else {
        return NULL;
    }
}

static const SkBlitRow::ColorProc16 platform_565_colorprocs_SSE2[] = {
    Color32A_D565_SSE2,                 // Color32A_D565,
    NULL,                               // Color32A_D565_Dither
};

SkBlitRow::ColorProc16 SkBlitRow::PlatformColorFactory565(unsigned flags) {
/* If you're thinking about writing an SSE4 version of this, do check it's
 * actually faster on Atom. Our original SSE4 version was slower than this
 * SSE2 version on Silvermont, and only marginally faster on a Core i7,
 * mainly due to the MULLD timings.
 */
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return platform_565_colorprocs_SSE2[flags];
    } else {
        return NULL;
    }
}

static const SkBlitRow::Proc32 platform_32_procs_SSE2[] = {
    NULL,                               // S32_Opaque,
    S32_Blend_BlitRow32_SSE2,           // S32_Blend,
    S32A_Opaque_BlitRow32_SSE2,         // S32A_Opaque
    S32A_Blend_BlitRow32_SSE2,          // S32A_Blend,
};

static const SkBlitRow::Proc32 platform_32_procs_SSE4[] = {
    NULL,                               // S32_Opaque,
    S32_Blend_BlitRow32_SSE2,           // S32_Blend,
    S32A_Opaque_BlitRow32_SSE4,         // S32A_Opaque
    S32A_Blend_BlitRow32_SSE2,          // S32A_Blend,
};

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE41)) {
        return platform_32_procs_SSE4[flags];
    } else
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return platform_32_procs_SSE2[flags];
    } else {
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

SkBlitMask::ColorProc SkBlitMask::PlatformColorProcs(SkColorType dstCT,
                                                     SkMask::Format maskFormat,
                                                     SkColor color) {
    if (SkMask::kA8_Format != maskFormat) {
        return NULL;
    }

    ColorProc proc = NULL;
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        switch (dstCT) {
            case kN32_SkColorType:
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
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        if (isOpaque) {
            return SkBlitLCD16OpaqueRow_SSE2;
        } else {
            return SkBlitLCD16Row_SSE2;
        }
    } else {
        return NULL;
    }

}

SkBlitMask::RowProc SkBlitMask::PlatformRowProcs(SkColorType, SkMask::Format, RowFlags) {
    return NULL;
}
