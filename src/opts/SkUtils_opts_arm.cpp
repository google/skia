/*
 * Copyright 2014 ARM Ltd.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkUtils.h"
#include "SkUtilsArm.h"

void sk_memset16_neon(uint16_t dst[], uint16_t value, int count);
void sk_memset32_neon(uint32_t dst[], uint32_t value, int count);

SkMemset16Proc SkMemset16GetPlatformProc() {
#if SK_ARM_NEON_IS_ALWAYS
    return sk_memset16_neon;
#elif SK_ARM_NEON_IS_DYNAMIC
    return sk_cpu_arm_has_neon() ? sk_memset16_neon : nullptr;
#else
    return nullptr;
#endif
}

SkMemset32Proc SkMemset32GetPlatformProc() {
#if SK_ARM_NEON_IS_ALWAYS
    return sk_memset32_neon;
#elif SK_ARM_NEON_IS_DYNAMIC
    return sk_cpu_arm_has_neon() ? sk_memset32_neon : nullptr;
#else
    return nullptr;
#endif
}

SkMemcpy32Proc SkMemcpy32GetPlatformProc() {
    return NULL;
}
