/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBlitRow_opts_arm_DEFINED
#define SkBlitRow_opts_arm_DEFINED

#include "SkBlitRow.h"
#include "SkUtilsArm.h"

// Define USE_NEON_CODE to indicate that we need to build NEON routines
#define USE_NEON_CODE  (!SK_ARM_NEON_IS_NONE)

// Define USE_ARM_CODE to indicate that we need to build ARM routines
#define USE_ARM_CODE   (!SK_ARM_NEON_IS_ALWAYS)

#if USE_NEON_CODE
// These are defined in SkBlitRow_opts_arm_neon.cpp
extern const SkBlitRow::Proc sk_blitrow_platform_565_procs_arm_neon[];
extern const SkBlitRow::Proc sk_blitrow_platform_4444_procs_arm_neon[];
extern const SkBlitRow::Proc32 sk_blitrow_platform_32_procs_arm_neon[];

extern void Color32_arm_neon(SkPMColor* dst, const SkPMColor* src, int count,
                             SkPMColor color);
#endif

#if USE_ARM_CODE
// These are defined in SkBlitRow_opts_arm.cpp
extern const SkBlitRow::Proc sk_blitrow_platform_565_procs_arm[];
extern const SkBlitRow::Proc sk_blitrow_platform_4444_procs_arm[];
extern const SkBlitRow::Proc32 sk_blitrow_platform_32_procs_arm[];
#endif

// Defined in SkBlitRow_opts_arm.cpp, used in all cases.
extern void S32A_Blend_BlitRow32_arm(SkPMColor* SK_RESTRICT dst,
                                     const SkPMColor* SK_RESTRICT src,
                                     int count, U8CPU alpha);

#endif
