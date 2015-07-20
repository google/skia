/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Including Sk4pxXfermode.h from this file should find SK_ARM_HAS_NEON is defined.
#include "Sk4pxXfermode.h"

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_neon(const ProcCoeff& r, SkXfermode::Mode m);
SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_neon(const ProcCoeff& r, SkXfermode::Mode m) {
    return SkCreate4pxXfermode(r, m);
}
