/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVx.h"
#include "Test.h"

using float2 = skvx::Vec<2,float>;
using float4 = skvx::Vec<4,float>;
using float8 = skvx::Vec<8,float>;

using double2 = skvx::Vec<2,double>;
using double4 = skvx::Vec<4,double>;
using double8 = skvx::Vec<8,double>;

using byte2 = skvx::Vec<2,uint8_t>;
using byte4 = skvx::Vec<4,uint8_t>;
using byte8 = skvx::Vec<8,uint8_t>;

using int2 = skvx::Vec<2,int32_t>;
using int4 = skvx::Vec<4,int32_t>;
using int8 = skvx::Vec<8,int32_t>;

using long2 = skvx::Vec<2,int64_t>;
using long4 = skvx::Vec<4,int64_t>;
using long8 = skvx::Vec<8,int64_t>;

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

        REPORTER_ASSERT(r,  any(mask));
        REPORTER_ASSERT(r, !all(mask));
    }

    {
        long4 mask = double4{1,2,3,4} < double4{1,2,4,8};
        REPORTER_ASSERT(r, mask[0] == int64_t( 0));
        REPORTER_ASSERT(r, mask[1] == int64_t( 0));
        REPORTER_ASSERT(r, mask[2] == int64_t(-1));
        REPORTER_ASSERT(r, mask[3] == int64_t(-1));

        REPORTER_ASSERT(r,  any(mask));
        REPORTER_ASSERT(r, !all(mask));
    }

    REPORTER_ASSERT(r, min(float4{1,2,3,4}) == 1);
    REPORTER_ASSERT(r, max(float4{1,2,3,4}) == 4);

    REPORTER_ASSERT(r, all(int4{1,2,3,4,5} == int4{1,2,3,4}));
    REPORTER_ASSERT(r, all(int4{1,2,3,4}   == int4{1,2,3,4}));
    REPORTER_ASSERT(r, all(int4{1,2,3}     == int4{1,2,3,0}));
    REPORTER_ASSERT(r, all(int4{1,2}       == int4{1,2,0,0}));
    REPORTER_ASSERT(r, all(int4{1}         == int4{1,0,0,0}));
    REPORTER_ASSERT(r, all(int4(1)         == int4{1,1,1,1}));
    REPORTER_ASSERT(r, all(int4{}          == int4{0,0,0,0}));
    REPORTER_ASSERT(r, all(int4()          == int4{0,0,0,0}));

    REPORTER_ASSERT(r, all(int4{1,2,2,1} == min(int4{1,2,3,4}, int4{4,3,2,1})));
    REPORTER_ASSERT(r, all(int4{4,3,3,4} == max(int4{1,2,3,4}, int4{4,3,2,1})));

    REPORTER_ASSERT(r, all(if_then_else(float4{1,2,3,2} <= float4{2,2,2,2}, float4(42), float4(47))
                           == float4{42,42,47,42}));

    REPORTER_ASSERT(r, all(floor(float4{-1.5f,1.5f,1.0f,-1.0f}) == float4{-2.0f,1.0f,1.0f,-1.0f}));
    REPORTER_ASSERT(r, all( ceil(float4{-1.5f,1.5f,1.0f,-1.0f}) == float4{-1.0f,2.0f,1.0f,-1.0f}));
    REPORTER_ASSERT(r, all(trunc(float4{-1.5f,1.5f,1.0f,-1.0f}) == float4{-1.0f,1.0f,1.0f,-1.0f}));
    REPORTER_ASSERT(r, all(round(float4{-1.5f,1.5f,1.0f,-1.0f}) == float4{-2.0f,2.0f,1.0f,-1.0f}));


    REPORTER_ASSERT(r, all(abs(float4{-2,-1,0,1}) == float4{2,1,0,1}));

    // TODO(mtklein): these tests could be made less loose.
    REPORTER_ASSERT(r, all( sqrt(float4{2,3,4,5}) < float4{2,2,3,3}));
    REPORTER_ASSERT(r, all(  rcp(float4{2,3,4,5}) < float4{1.0f,0.5f,0.5f,0.3f}));
    REPORTER_ASSERT(r, all(rsqrt(float4{2,3,4,5}) < float4{1.0f,1.0f,1.0f,0.5f}));

    REPORTER_ASSERT(r, all(cast<int>(float4{-1.5f,0.5f,1.0f,1.5f}) == int4{-1,0,1,1}));

    float buf[4] = {1,2,3,4};
    REPORTER_ASSERT(r, all(float4::Load(buf) == float4{1,2,3,4}));
    float4{2,3,4,5}.store(buf);
    REPORTER_ASSERT(r, all(float4::Load(buf) == float4{2,3,4,5}));

    {
        int4 iota = {0,1,2,3};
        int i = 0;
        for (int val : iota) {
            REPORTER_ASSERT(r, val == i++);
        }
        for (int& val : iota) {
            val = 42;
        }
        REPORTER_ASSERT(r, all(iota == 42));
    }

    REPORTER_ASSERT(r, all(mad(float4{1,2,3,4}, 2.0f, 3.0f) == float4{5,7,9,11}));

    REPORTER_ASSERT(r, all(shuffle<2,1,0,3>        (float4{1,2,3,4}) == float4{3,2,1,4}));
    REPORTER_ASSERT(r, all(shuffle<2,1>            (float4{1,2,3,4}) == float2{3,2}));
    REPORTER_ASSERT(r, all(shuffle<2,1,2,1,2,1,2,1>(float4{1,2,3,4}) == float8{3,2,3,2,3,2,3,2}));
    REPORTER_ASSERT(r, all(shuffle<3,3,3,3>        (float4{1,2,3,4}) == float4{4,4,4,4}));
}
