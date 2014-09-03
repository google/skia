/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_SSE4_DEFINED
#define SkBlitRow_opts_SSE4_DEFINED

#include "SkBlitRow.h"

#ifdef CRBUG_399842_FIXED

/* Check if we are able to build assembly code, GCC/AT&T syntax:
 *  1) Clang and GCC are generally OK.  OS X's old LLVM-GCC 4.2 can't handle it;
 *  2) We're intentionally not linking this in even when supported (Clang) on Windows;
 *  3) MemorySanitizer cannot instrument assembly at all.
 */
#if /* 1)*/ (defined(__clang__) || (defined(__GNUC__) && !defined(SK_BUILD_FOR_MAC))) \
    /* 2)*/ && !defined(SK_BUILD_FOR_WIN)                                             \
    /* 3)*/ && !defined(MEMORY_SANITIZER)
extern "C" void S32A_Opaque_BlitRow32_SSE4_asm(SkPMColor* SK_RESTRICT dst,
                                               const SkPMColor* SK_RESTRICT src,
                                               int count, U8CPU alpha);

#define SK_ATT_ASM_SUPPORTED
#endif

#endif // CRBUG_399842_FIXED

#endif

