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

using double2 = SkVx<2,double>;
using double4 = SkVx<4,double>;
using double8 = SkVx<8,double>;

using byte2 = SkVx<2,uint8_t>;
using byte4 = SkVx<4,uint8_t>;
using byte8 = SkVx<8,uint8_t>;

using int2 = SkVx<2,int32_t>;
using int4 = SkVx<4,int32_t>;
using int8 = SkVx<8,int32_t>;

using long2 = SkVx<2,int64_t>;
using long4 = SkVx<4,int64_t>;
using long8 = SkVx<8,int64_t>;

DEF_TEST(SkVx, r) {
    static_assert(sizeof(float2) ==  8, "");
    static_assert(sizeof(float4) == 16, "");
    static_assert(sizeof(float8) == 32, "");

    static_assert(sizeof(byte2) == 2, "");
    static_assert(sizeof(byte4) == 4, "");
    static_assert(sizeof(byte8) == 8, "");

    {
        int4 mask = float4{1,2,3,4} < float4{1,2,4,8};
        REPORTER_ASSERT(r, mask[0] == int32_t( 0));
        REPORTER_ASSERT(r, mask[1] == int32_t( 0));
        REPORTER_ASSERT(r, mask[2] == int32_t(-1));
        REPORTER_ASSERT(r, mask[3] == int32_t(-1));
    }

    {
        long4 mask = double4{1,2,3,4} < double4{1,2,4,8};
        REPORTER_ASSERT(r, mask[0] == int64_t( 0));
        REPORTER_ASSERT(r, mask[1] == int64_t( 0));
        REPORTER_ASSERT(r, mask[2] == int64_t(-1));
        REPORTER_ASSERT(r, mask[3] == int64_t(-1));
    }
}
