/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// useless include to avoid "#if before #include" complaint from Skia presubmit
#include "SkSLUtil.h"
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#pragma clang diagnostic ignored "-Wnull-conversion"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wdeprecated-register"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4018)
#endif
