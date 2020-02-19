/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEFINES
#define SKSL_DEFINES

#include <cstdint>

#ifdef SKSL_STANDALONE
#if defined(_WIN32) || defined(__SYMBIAN32__)
#define SKSL_BUILD_FOR_WIN
#endif
#else
#ifdef SK_BUILD_FOR_WIN
#define SKSL_BUILD_FOR_WIN
#endif // SK_BUILD_FOR_WIN
#endif // SKSL_STANDALONE

#ifdef SKSL_STANDALONE
#define SkASSERT(x) do { if (!(x)) abort(); } while (false)
#define SkAssertResult(x) do { if (!(x)) abort(); } while (false)
#define SkDEBUGCODE(...) __VA_ARGS__
#define SK_API
#if !defined(SkUNREACHABLE)
#  if defined(_MSC_VER) && !defined(__clang__)
#    define SkUNREACHABLE __assume(false)
#  else
#    define SkUNREACHABLE __builtin_unreachable()
#  endif
#endif
#else
#include "include/core/SkTypes.h"
#endif

#define SKSL_PRINTF_LIKE(A, B) [[gnu::format(printf, (A), (B))]]

#define ABORT(...) (printf(__VA_ARGS__), sksl_abort())

using SKSL_INT = int32_t;
using SKSL_FLOAT = float;

#endif
