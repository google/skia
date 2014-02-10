/*
 * Copyright 2014 ARM Ltd.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMorphology_opts.h"
#include "SkMorphology_opts_neon.h"
#include "SkUtilsArm.h"

SkMorphologyImageFilter::Proc SkMorphologyGetPlatformProc(SkMorphologyProcType type) {
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
