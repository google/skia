/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermode.h"
#include "SkXfermode_proccoeff.h"
#include "SkUtilsArm.h"

// If we find we do have NEON, we'll call this method from SkXfermodes_opts_arm_neon.cpp.
SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_neon(const ProcCoeff& rec,
                                                         SkXfermode::Mode mode);

// If we don't have NEON, we'll call this method and return NULL.
SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl(const ProcCoeff& rec, SkXfermode::Mode mode);
SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl(const ProcCoeff& rec, SkXfermode::Mode mode) {
    return NULL;
}

SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec, SkXfermode::Mode mode);
SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec, SkXfermode::Mode mode) {
    return SK_ARM_NEON_WRAP(SkPlatformXfermodeFactory_impl)(rec, mode);
}

SkXfermodeProc SkPlatformXfermodeProcFactory(SkXfermode::Mode mode);
SkXfermodeProc SkPlatformXfermodeProcFactory(SkXfermode::Mode mode) { return NULL; }
