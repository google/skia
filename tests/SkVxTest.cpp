/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVx.h"
#include "Test.h"

using float2 = SkVx<2,float>;
using float4 = SkVx<4,float>;
using float8 = SkVx<8,float>;

using byte2 = SkVx<2,uint8_t>;
using byte4 = SkVx<4,uint8_t>;
using byte8 = SkVx<8,uint8_t>;

DEF_TEST(SkVx, r) {
    static_assert(sizeof(float2) ==  8, "");
    static_assert(sizeof(float4) == 16, "");
    static_assert(sizeof(float8) == 32, "");

    static_assert(sizeof(byte2) == 2, "");
    static_assert(sizeof(byte4) == 4, "");
    static_assert(sizeof(byte8) == 8, "");
}
