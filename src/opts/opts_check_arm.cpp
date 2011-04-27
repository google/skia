/***************************************************************************
 Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 Copyright 2006-2010, The Android Open Source Project
 
 Licensed under the Apache License, Version 2.0 (the "License"); you
 may not use this file except in compliance with the License.  You may
 obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.
 ***************************************************************************/

/* Changes:
 * 2011-04-01 ARM
 *    Merged the functions from src/opts/opts_check_arm_neon.cpp
 *    Modified to return ARM version of memset16 and memset32 if no neon
 *    available in the core
 */

#include "SkUtils.h"

extern "C" void memset16_neon(uint16_t dst[], uint16_t value, int count);
extern "C" void memset32_neon(uint32_t dst[], uint32_t value, int count);

extern "C" void arm_memset16(uint16_t* dst, uint16_t value, int count);
extern "C" void arm_memset32(uint32_t* dst, uint32_t value, int count);

static inline bool hasNeonRegisters() {
#if defined(__ARM_HAVE_NEON) && defined(SK_CPU_LENDIAN)
    return true;
#else
    return false;
#endif
}

SkMemset16Proc SkMemset16GetPlatformProc() {
    if (hasNeonRegisters()) {
        return memset16_neon;
    } else {
#if defined(SK_CPU_LENDIAN)
        return arm_memset16;
#else
        return NULL;
#endif
    }
}

SkMemset32Proc SkMemset32GetPlatformProc() {
    if (hasNeonRegisters()) {
        return memset32_neon;
    } else {
#if defined(SK_CPU_LENDIAN)
        return arm_memset32;
#else
        return NULL;
#endif
    }
}
