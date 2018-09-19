/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"  // for tilemodes
#include "src/core/SkBitmapProcState.h"
#include "src/core/SkBitmapProcState_filter.h"
#include "src/core/SkColorData.h"
#include "src/core/SkUtilsArm.h"

// Required to ensure the table is part of the final binary.
extern const SkBitmapProcState::SampleProc32 gSkBitmapProcStateSample32_neon[];

#define   NAME_WRAP(x)  x ## _neon
#include "src/core/SkBitmapProcState_procs.h"
#include "src/opts/SkBitmapProcState_filter_neon.h"

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
