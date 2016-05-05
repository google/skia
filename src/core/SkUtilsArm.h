/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtilsArm_DEFINED
#define SkUtilsArm_DEFINED

#include "SkTypes.h"

#if defined(SK_ARM_HAS_NEON)
    #define SK_ARM_NEON_WRAP(x) (x ## _neon)
#else
    #define SK_ARM_NEON_WRAP(x) (x)
#endif

#endif // SkUtilsArm_DEFINED
