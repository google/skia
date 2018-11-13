/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState_opts_SSE2.h"
#include "SkBitmapProcState_opts_SSSE3.h"
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
            fSampleProc32 = S32_alpha_D32_filter_DX_SSSE3;
        } else {
            fSampleProc32 = S32_opaque_D32_filter_DX_SSE2;
        }
    } else if (fSampleProc32 == S32_alpha_D32_filter_DX) {
        if (ssse3) {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSSE3;
        } else {
            fSampleProc32 = S32_alpha_D32_filter_DX_SSE2;
        }
    }

    /* Check fMatrixProc */
    if (fMatrixProc == ClampX_ClampY_filter_scale) {
        fMatrixProc = ClampX_ClampY_filter_scale_SSE2;
    } else if (fMatrixProc == ClampX_ClampY_nofilter_scale) {
        fMatrixProc = ClampX_ClampY_nofilter_scale_SSE2;
    }
}
