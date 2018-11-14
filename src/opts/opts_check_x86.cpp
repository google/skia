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


void SkBitmapProcState::platformProcs() {
    if (SkCpu::Supports(SkCpu::SSE2)) {
        if (fSampleProc32 == S32_D32_filter_DX) {
            if (SkCpu::Supports(SkCpu::SSSE3)) {
                fSampleProc32 = S32_D32_filter_DX_SSSE3;
            } else {
                fSampleProc32 = S32_D32_filter_DX_SSE2;
            }
        }

        if (fMatrixProc == ClampX_ClampY_filter_scale) {
            fMatrixProc = ClampX_ClampY_filter_scale_SSE2;
        } else if (fMatrixProc == ClampX_ClampY_nofilter_scale) {
            fMatrixProc = ClampX_ClampY_nofilter_scale_SSE2;
        }
    }
}
