/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkBitmapProcState_filter.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkShader.h"   // for tilemodes
#include "SkUtilsArm.h"

// Required to ensure the table is part of the final binary.
extern const SkBitmapProcState::SampleProc32 gSkBitmapProcStateSample32_neon[];

#define   NAME_WRAP(x)  x ## _neon
#include "SkBitmapProcState_filter_neon.h"
#include "SkBitmapProcState_procs.h"

const SkBitmapProcState::SampleProc32 gSkBitmapProcStateSample32_neon[] = {
    S32_opaque_D32_nofilter_DXDY_neon,
    S32_alpha_D32_nofilter_DXDY_neon,
    S32_opaque_D32_nofilter_DX_neon,
    S32_alpha_D32_nofilter_DX_neon,
    S32_opaque_D32_filter_DXDY_neon,
    S32_alpha_D32_filter_DXDY_neon,
    S32_opaque_D32_filter_DX_neon,
    S32_alpha_D32_filter_DX_neon,
};
