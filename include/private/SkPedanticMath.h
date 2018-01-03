/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPedanticMath_DEFINED
#define SkPedanticMath_DEFINED

// To avoid UBSAN complaints about 2's compliment overflows
//
static inline int32_t SkSub32(int32_t a, int32_t b) {
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

#endif
