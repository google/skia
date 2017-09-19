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

#define ABORT(...) (fprintf(stderr, __VA_ARGS__), abort())
#define ASSERT(x) (void)((x) || (ABORT("failed assert(%s): %s:%d\n", #x, __FILE__, __LINE__), 0))

#endif
