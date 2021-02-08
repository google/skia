/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_LEXUTIL
#define SKSL_LEXUTIL

#include <cstdlib>

#define INVALID -1

#define SK_ABORT(...) (fprintf(stderr, __VA_ARGS__), abort())
#define SkASSERT(x) \
    (void)((x) || (SK_ABORT("failed SkASSERT(%s): %s:%d\n", #x, __FILE__, __LINE__), 0))
#define SkUNREACHABLE (SK_ABORT("unreachable"))

#endif
