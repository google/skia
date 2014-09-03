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
#include "SkBlitRect_opts_SSE2.h"
#include "SkBlitRow.h"
#include "SkBlitRow_opts_SSE2.h"
#include "SkBlitRow_opts_SSE4.h"
#include "SkBlurImage_opts_SSE2.h"
#include "SkBlurImage_opts_SSE4.h"
#include "SkMorphology_opts.h"
#include "SkMorphology_opts_SSE2.h"
#include "SkRTConf.h"
#include "SkUtils.h"
#include "SkUtils_opts_SSE2.h"
#include "SkXfermode.h"
#include "SkXfermode_proccoeff.h"

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
static int get_SIMD_level() {
    int cpu_info[4] = { 0 };

    getcpuid(1, cpu_info);
    if ((cpu_info[2] & (1<<20)) != 0) {
        return SK_CPU_SSE_LEVEL_SSE42;
    } else if ((cpu_info[2] & (1<<19)) != 0) {
        return SK_CPU_SSE_LEVEL_SSE41;
    } else if ((cpu_info[2] & (1<<9)) != 0) {
        return SK_CPU_SSE_LEVEL_SSSE3;
    } else if ((cpu_info[3] & (1<<26)) != 0) {
        return SK_CPU_SSE_LEVEL_SSE2;
    } else {
        return 0;
    }
}

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
        static int gSIMDLevel = get_SIMD_level();
        return (minLevel <= gSIMDLevel);
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////

SK_CONF_DECLARE( bool, c_hqfilter_sse, "bitmap.filter.highQualitySSE", true, "Use SSE optimized version of high quality image filters");

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

    /* Check fSampleProc32 */
    if (fSampleProc32 == S32_opaque_D32_filter_DX) {
        if (supports_simd(SK_CPU_SSE_LEVEL_SSSE3)) {
            fSampleProc32 = S32_opaque_D32_filter_DX_SSSE3;
        } else {
            fSampleProc32 = S32_opaque_D32_filter_DX_SSE2;
        }
    } else if (fSampleProc32 == S32_opaque_D32_filter_DXDY) {
        if (supports_simd(SK_CPU_SSE_LEVEL_SSSE3)) {
            fSampleProc32 = S32_opaque_D32_filter_DXDY_SSSE3;
        }
    } else if (fSampleProc32 == S32_alpha_D32_filter_DX) {
        if (supports_simd(SK_CPU_SSE_LEVEL_SSSE3)) {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSSE3;
        } else {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSE2;
        }
    } else if (fSampleProc32 == S32_alpha_D32_filter_DXDY) {
        if (supports_simd(SK_CPU_SSE_LEVEL_SSSE3)) {
            fSampleProc32 = S32_alpha_D32_filter_DXDY_SSSE3;
        }
    }

    /* Check fSampleProc16 */
    if (fSampleProc16 == S32_D16_filter_DX) {
        fSampleProc16 = S32_D16_filter_DX_SSE2;
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

    /* Check fShaderProc32 */
    if (c_hqfilter_sse) {
        if (fShaderProc32 == highQualityFilter32) {
            fShaderProc32 = highQualityFilter_SSE2;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

static SkBlitRow::Proc platform_16_procs[] = {
    S32_D565_Opaque_SSE2,               // S32_D565_Opaque
    NULL,                               // S32_D565_Blend
    S32A_D565_Opaque_SSE2,              // S32A_D565_Opaque
    NULL,                               // S32A_D565_Blend
    S32_D565_Opaque_Dither_SSE2,        // S32_D565_Opaque_Dither
    NULL,                               // S32_D565_Blend_Dither
    S32A_D565_Opaque_Dither_SSE2,       // S32A_D565_Opaque_Dither
    NULL,                               // S32A_D565_Blend_Dither
};

SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return platform_16_procs[flags];
    } else {
        return NULL;
    }
}

static SkBlitRow::Proc32 platform_32_procs_SSE2[] = {
    NULL,                               // S32_Opaque,
    S32_Blend_BlitRow32_SSE2,           // S32_Blend,
    S32A_Opaque_BlitRow32_SSE2,         // S32A_Opaque
    S32A_Blend_BlitRow32_SSE2,          // S32A_Blend,
};

#if defined(SK_ATT_ASM_SUPPORTED)
static SkBlitRow::Proc32 platform_32_procs_SSE4[] = {
    NULL,                               // S32_Opaque,
    S32_Blend_BlitRow32_SSE2,           // S32_Blend,
    S32A_Opaque_BlitRow32_SSE4_asm,     // S32A_Opaque
    S32A_Blend_BlitRow32_SSE2,          // S32A_Blend,
};
#endif

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
#if defined(SK_ATT_ASM_SUPPORTED)
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE41)) {
        return platform_32_procs_SSE4[flags];
    } else
#endif
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return platform_32_procs_SSE2[flags];
    } else {
        return NULL;
    }
}

SkBlitRow::ColorProc SkBlitRow::PlatformColorProc() {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return Color32_SSE2;
    } else {
        return NULL;
    }
}

SkBlitRow::ColorRectProc PlatformColorRectProcFactory(); // suppress warning

SkBlitRow::ColorRectProc PlatformColorRectProcFactory() {
/* Return NULL for now, since the optimized path in ColorRect32_SSE2 is disabled.
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return ColorRect32_SSE2;
    } else {
        return NULL;
    }
*/
    return NULL;
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

////////////////////////////////////////////////////////////////////////////////

SkMemset16Proc SkMemset16GetPlatformProc() {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return sk_memset16_SSE2;
    } else {
        return NULL;
    }
}

SkMemset32Proc SkMemset32GetPlatformProc() {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return sk_memset32_SSE2;
    } else {
        return NULL;
    }
}

SkMemcpy32Proc SkMemcpy32GetPlatformProc() {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return sk_memcpy32_SSE2;
    } else {
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

SkMorphologyImageFilter::Proc SkMorphologyGetPlatformProc(SkMorphologyProcType type) {
    if (!supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return NULL;
    }
    switch (type) {
        case kDilateX_SkMorphologyProcType:
            return SkDilateX_SSE2;
        case kDilateY_SkMorphologyProcType:
            return SkDilateY_SSE2;
        case kErodeX_SkMorphologyProcType:
            return SkErodeX_SSE2;
        case kErodeY_SkMorphologyProcType:
            return SkErodeY_SSE2;
        default:
            return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

bool SkBoxBlurGetPlatformProcs(SkBoxBlurProc* boxBlurX,
                               SkBoxBlurProc* boxBlurY,
                               SkBoxBlurProc* boxBlurXY,
                               SkBoxBlurProc* boxBlurYX) {
#ifdef SK_DISABLE_BLUR_DIVISION_OPTIMIZATION
    return false;
#else
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE41)) {
        return SkBoxBlurGetPlatformProcs_SSE4(boxBlurX, boxBlurY, boxBlurXY, boxBlurYX);
    }
    else if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return SkBoxBlurGetPlatformProcs_SSE2(boxBlurX, boxBlurY, boxBlurXY, boxBlurYX);
    }
    return false;
#endif
}

////////////////////////////////////////////////////////////////////////////////

extern SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_SSE2(const ProcCoeff& rec,
                                                                SkXfermode::Mode mode);

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl(const ProcCoeff& rec,
                                                    SkXfermode::Mode mode);

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl(const ProcCoeff& rec,
                                                    SkXfermode::Mode mode) {
    return NULL;
}

SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec,
                                               SkXfermode::Mode mode);

SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec,
                                               SkXfermode::Mode mode) {
    if (supports_simd(SK_CPU_SSE_LEVEL_SSE2)) {
        return SkPlatformXfermodeFactory_impl_SSE2(rec, mode);
    } else {
        return SkPlatformXfermodeFactory_impl(rec, mode);
    }
}

SkXfermodeProc SkPlatformXfermodeProcFactory(SkXfermode::Mode mode);

SkXfermodeProc SkPlatformXfermodeProcFactory(SkXfermode::Mode mode) {
    return NULL;
}
