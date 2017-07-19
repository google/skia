/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState_opts_SSE2.h"
#include "SkBitmapProcState_opts_SSSE3.h"
#include "SkBlitMask.h"
#include "SkBlitRow.h"
#include "SkBlitRow_opts_SSE2.h"
#include "SkCpu.h"


/*
 *****************************************
 *********This file is deprecated*********
 *****************************************
 * New CPU-specific work should be done in
 * SkOpts framework. Run-time detection of
 * available instruction set extensions is
 * implemented in src/core/SkOpts.cpp file
 *****************************************
 */


/* This file must *not* be compiled with -msse or any other optional SIMD
   extension, otherwise gcc may generate SIMD instructions even for scalar ops
   (and thus give an invalid instruction on Pentium3 on the code below).
   For example, only files named *_SSE2.cpp in this directory should be
   compiled with -msse2 or higher. */

////////////////////////////////////////////////////////////////////////////////

void SkBitmapProcState::platformProcs() {
    /* Every optimization in the function requires at least SSE2 */
    if (!SkCpu::Supports(SkCpu::SSE2)) {
        return;
    }
    const bool ssse3 = SkCpu::Supports(SkCpu::SSSE3);

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

static const SkBlitRow::Proc32 platform_32_procs_SSE2[] = {
    nullptr,                               // S32_Opaque,
    S32_Blend_BlitRow32_SSE2,           // S32_Blend,
    nullptr,                            // Ported to SkOpts
    S32A_Blend_BlitRow32_SSE2,          // S32A_Blend,
};

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    if (SkCpu::Supports(SkCpu::SSE2)) {
        return platform_32_procs_SSE2[flags];
    } else {
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////

SkBlitMask::BlitLCD16RowProc SkBlitMask::PlatformBlitRowProcs16(bool isOpaque) {
    if (SkCpu::Supports(SkCpu::SSE2)) {
        if (isOpaque) {
            return SkBlitLCD16OpaqueRow_SSE2;
        } else {
            return SkBlitLCD16Row_SSE2;
        }
    } else {
        return nullptr;
    }

}

SkBlitMask::RowProc SkBlitMask::PlatformRowProcs(SkColorType, SkMask::Format, RowFlags) {
    return nullptr;
}
