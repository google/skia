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

#include "SkUtils.h"

#if defined(__ARM_HAVE_NEON) && defined(SK_CPU_LENDIAN)
extern "C" void memset16_neon(uint16_t dst[], uint16_t value, int count);
extern "C" void memset32_neon(uint32_t dst[], uint32_t value, int count);
#endif

#if defined(SK_CPU_LENDIAN)
extern "C" void arm_memset16(uint16_t* dst, uint16_t value, int count);
extern "C" void arm_memset32(uint32_t* dst, uint32_t value, int count);
#endif

SkMemset16Proc SkMemset16GetPlatformProc() {
#if defined(__ARM_HAVE_NEON) && defined(SK_CPU_LENDIAN)
    return memset16_neon;
#elif defined(SK_CPU_LENDIAN)
    return arm_memset16;
#else
    return NULL;
#endif
}

SkMemset32Proc SkMemset32GetPlatformProc() {
#if defined(__ARM_HAVE_NEON) && defined(SK_CPU_LENDIAN)
    return memset32_neon;
#elif defined(SK_CPU_LENDIAN)
    return arm_memset32;
#else
    return NULL;
#endif
}
