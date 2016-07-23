/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSwizzle.h"

#include "SkOpts.h"

void SkSwapRB(uint32_t* dest, const uint32_t* src, int count) {
    SkOpts::RGBA_to_BGRA(dest, src, count);
}
