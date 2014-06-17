/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_SSE4_DEFINED
#define SkBlitRow_opts_SSE4_DEFINED

#include "SkBlitRow.h"

/* Check if we are able to build assembly code, GCC/AT&T syntax.
 * Had problems with LLVM-GCC 4.2.
 */
#if defined(__clang__) || (defined(__GNUC__) && !defined(SK_BUILD_FOR_MAC))
extern "C" void S32A_Opaque_BlitRow32_SSE4_asm(SkPMColor* SK_RESTRICT dst,
                                               const SkPMColor* SK_RESTRICT src,
                                               int count, U8CPU alpha);

#define SK_ATT_ASM_SUPPORTED
#endif

#endif

