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

void Color32A_D565_SSE4(uint16_t dst[], SkPMColor src, int count, int x, int y);

#endif

