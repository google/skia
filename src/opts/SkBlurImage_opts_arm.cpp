/*
 * Copyright 2014 ARM Ltd.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurImage_opts_neon.h"
#include "SkUtilsArm.h"

bool SkBoxBlurGetPlatformProcs(SkBoxBlurProc* boxBlurX,
                               SkBoxBlurProc* boxBlurY,
                               SkBoxBlurProc* boxBlurXY,
                               SkBoxBlurProc* boxBlurYX) {
#if SK_ARM_NEON_IS_NONE
    return false;
#elif defined(SK_CPU_ARM64)  // Temporary fix for
    return false;            // http://skbug.com/2845
#else
#if SK_ARM_NEON_IS_DYNAMIC
    if (!sk_cpu_arm_has_neon()) {
        return false;
    }
#endif
    return SkBoxBlurGetPlatformProcs_NEON(boxBlurX, boxBlurY, boxBlurXY, boxBlurYX);
#endif
}
