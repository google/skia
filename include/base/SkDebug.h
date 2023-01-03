/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDebug_DEFINED
#define SkDebug_DEFINED

#include "include/base/SkAPI.h"

// Sets SK_DEBUG or SK_RELEASE
#include "include/base/SkLoadUserConfig.h" // IWYU pragma: keep

#if !defined(SK_PRINTF_LIKE)
#  if defined(__clang__) || defined(__GNUC__)
#    define SK_PRINTF_LIKE(A, B) __attribute__((format(printf, (A), (B))))
#  else
#    define SK_PRINTF_LIKE(A, B)
#  endif
#endif

#if !defined(SkDebugf)
    void SK_SPI SkDebugf(const char format[], ...) SK_PRINTF_LIKE(1, 2);
#endif

#if defined(SK_DEBUG)
    #define SkDEBUGCODE(...)  __VA_ARGS__
    #define SkDEBUGF(...)     SkDebugf(__VA_ARGS__)
#else
    #define SkDEBUGCODE(...)
    #define SkDEBUGF(...)
#endif

#endif
