/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_SSE4_DEFINED
#define SkBlitRow_opts_SSE4_DEFINED

#include "SkBlitRow.h"

#if !defined(_MSC_VER)
extern "C" void S32A_Opaque_BlitRow32_SSE4_asm(SkPMColor* SK_RESTRICT dst,
                                               const SkPMColor* SK_RESTRICT src,
                                               int count, U8CPU alpha);
#endif

#endif

