/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// IWYU pragma: private, include "png.h"

// IWYU pragma: begin_exports
#include "../externals/libpng/scripts/pnglibconf.h.prebuilt"
// IWYU pragma: end_exports
#undef PNG_READ_OPT_PLTE_SUPPORTED
