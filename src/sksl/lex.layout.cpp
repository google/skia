/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "disable_flex_warnings.h"
#include "lex.layout.c"
static_assert(YY_FLEX_MAJOR_VERSION * 100 + YY_FLEX_MINOR_VERSION * 10 +
              YY_FLEX_SUBMINOR_VERSION >= 261,
              "we require Flex 2.6.1 or better for security reasons");
