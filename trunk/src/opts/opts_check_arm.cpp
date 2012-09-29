/***************************************************************************
 * Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 * Copyright 2006-2010, The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 ***************************************************************************/

/* Changes:
 * 2011-04-01 ARM
 *    Merged the functions from src/opts/opts_check_arm_neon.cpp
 *    Modified to return ARM version of memset16 and memset32 if no neon
 *    available in the core
 */

#include "SkBlitRow.h"
#include "SkUtils.h"

#include "SkUtilsArm.h"

#if defined(SK_CPU_LENDIAN) && !SK_ARM_NEON_IS_NONE
extern "C" void memset16_neon(uint16_t dst[], uint16_t value, int count);
extern "C" void memset32_neon(uint32_t dst[], uint32_t value, int count);
#endif

#if defined(SK_CPU_LENDIAN)
extern "C" void arm_memset16(uint16_t* dst, uint16_t value, int count);
extern "C" void arm_memset32(uint32_t* dst, uint32_t value, int count);
#endif

SkMemset16Proc SkMemset16GetPlatformProc() {
#if !defined(SK_CPU_LENDIAN)
    return NULL;
#elif SK_ARM_NEON_IS_DYNAMIC
    if (sk_cpu_arm_has_neon()) {
        return memset16_neon;
    } else {
        return arm_memset16;
    }
#elif SK_ARM_NEON_IS_ALWAYS
    return memset16_neon;
#else
    return arm_memset16;
#endif
}

SkMemset32Proc SkMemset32GetPlatformProc() {
#if !defined(SK_CPU_LENDIAN)
    return NULL;
#elif SK_ARM_NEON_IS_DYNAMIC
    if (sk_cpu_arm_has_neon()) {
        return memset32_neon;
    } else {
        return arm_memset32;
    }
#elif SK_ARM_NEON_IS_ALWAYS
    return memset32_neon;
#else
    return arm_memset32;
#endif
}

SkBlitRow::ColorRectProc PlatformColorRectProcFactory() {
    return NULL;
}

