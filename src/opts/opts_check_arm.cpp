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
#include "SkMorphology_opts.h"
#include "SkMorphology_opts_neon.h"
#include "SkBlurImage_opts_neon.h"

#if defined(SK_CPU_LENDIAN) && !SK_ARM_NEON_IS_NONE
extern "C" void memset16_neon(uint16_t dst[], uint16_t value, int count);
extern "C" void memset32_neon(uint32_t dst[], uint32_t value, int count);
#endif

#if defined(SK_CPU_LENDIAN)
extern "C" void arm_memset16(uint16_t* dst, uint16_t value, int count);
extern "C" void arm_memset32(uint32_t* dst, uint32_t value, int count);
#endif

SkMemset16Proc SkMemset16GetPlatformProc() {
    // FIXME: memset.arm.S is using syntax incompatible with XCode
#if !defined(SK_CPU_LENDIAN) || defined(SK_BUILD_FOR_IOS)
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
    // FIXME: memset.arm.S is using syntax incompatible with XCode
#if !defined(SK_CPU_LENDIAN) || defined(SK_BUILD_FOR_IOS)
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

SkMorphologyProc SkMorphologyGetPlatformProc(SkMorphologyProcType type) {
#if SK_ARM_NEON_IS_NONE
    return NULL;
#else
#if SK_ARM_NEON_IS_DYNAMIC
    if (!sk_cpu_arm_has_neon()) {
        return NULL;
    }
#endif
    switch (type) {
        case kDilateX_SkMorphologyProcType:
            return SkDilateX_neon;
        case kDilateY_SkMorphologyProcType:
            return SkDilateY_neon;
        case kErodeX_SkMorphologyProcType:
            return SkErodeX_neon;
        case kErodeY_SkMorphologyProcType:
            return SkErodeY_neon;
        default:
            return NULL;
    }
#endif
}

bool SkBoxBlurGetPlatformProcs(SkBoxBlurProc* boxBlurX,
                               SkBoxBlurProc* boxBlurY,
                               SkBoxBlurProc* boxBlurXY,
                               SkBoxBlurProc* boxBlurYX) {
#if SK_ARM_NEON_IS_NONE
    return false;
#else
#if SK_ARM_NEON_IS_DYNAMIC
    if (!sk_cpu_arm_has_neon()) {
        return false;
    }
#endif
    return SkBoxBlurGetPlatformProcs_NEON(boxBlurX, boxBlurY, boxBlurXY, boxBlurYX);
#endif
}
