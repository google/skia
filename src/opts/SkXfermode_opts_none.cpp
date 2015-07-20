/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermode.h"
#include "SkXfermode_proccoeff.h"


SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec, SkXfermode::Mode mode);
SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec, SkXfermode::Mode mode) {
    return NULL;
}

SkXfermodeProc SkPlatformXfermodeProcFactory(SkXfermode::Mode mode);
SkXfermodeProc SkPlatformXfermodeProcFactory(SkXfermode::Mode mode) {
    return NULL;
}
