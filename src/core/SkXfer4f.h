/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermodePriv_DEFINED
#define SkXfermodePriv_DEFINED

#include "SkXfermode.h"

enum SkXfef4fFlags {
    kSrcIsOpaque_SkXfer4fFlag   = 1 << 0,
    kDstIsSRGB_SkXfer4fFlag     = 1 << 1,
};

typedef void (*SkPM4fXfer1Proc)(uint32_t dst[], const SkPM4f& src, int count);
typedef void (*SkPM4fXferNProc)(uint32_t dst[], const SkPM4f src[], int count);

SkPM4fXfer1Proc SkPM4fXfer1ProcFactory(SkXfermode::Mode, uint32_t flags);
SkPM4fXferNProc SkPM4fXferNProcFactory(SkXfermode::Mode, uint32_t flags);

#endif
