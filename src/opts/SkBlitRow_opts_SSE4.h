/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_SSE4_DEFINED
#define SkBlitRow_opts_SSE4_DEFINED

#include "SkBlitRow.h"

void S32A_Opaque_BlitRow32_SSE4(SkPMColor* SK_RESTRICT,
                                const SkPMColor* SK_RESTRICT,
                                int count,
                                U8CPU alpha);
#endif

