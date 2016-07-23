/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4x4f.h"
#include "Test.h"

DEF_TEST(Sk4x4f, r) {
    Sk4x4f f;

    Sk4f x{ 0, 1, 2, 3},
         y{ 4, 5, 6, 7},
         z{ 8, 9,10,11},
         w{12,13,14,15};
    f = Sk4x4f::Transpose(x,y,z,w);
    REPORTER_ASSERT(r, f.r[0] == 0 && f.r[1] == 4 && f.r[2] ==  8 && f.r[3] == 12);
    REPORTER_ASSERT(r, f.g[0] == 1 && f.g[1] == 5 && f.g[2] ==  9 && f.g[3] == 13);
    REPORTER_ASSERT(r, f.b[0] == 2 && f.b[1] == 6 && f.b[2] == 10 && f.b[3] == 14);
    REPORTER_ASSERT(r, f.a[0] == 3 && f.a[1] == 7 && f.a[2] == 11 && f.a[3] == 15);

    Sk4f s,t,u,v;
    f.transpose(&s,&t,&u,&v);
    REPORTER_ASSERT(r, (x == s).allTrue()
                    && (y == t).allTrue()
                    && (z == u).allTrue()
                    && (w == v).allTrue());


    float fs[16] = {0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15};
    f = Sk4x4f::Transpose(fs);
    REPORTER_ASSERT(r, f.r[0] == 0 && f.r[1] == 4 && f.r[2] ==  8 && f.r[3] == 12);
    REPORTER_ASSERT(r, f.g[0] == 1 && f.g[1] == 5 && f.g[2] ==  9 && f.g[3] == 13);
    REPORTER_ASSERT(r, f.b[0] == 2 && f.b[1] == 6 && f.b[2] == 10 && f.b[3] == 14);
    REPORTER_ASSERT(r, f.a[0] == 3 && f.a[1] == 7 && f.a[2] == 11 && f.a[3] == 15);

    float fs_back[16];
    f.transpose(fs_back);
    REPORTER_ASSERT(r, 0 == memcmp(fs, fs_back, sizeof(fs)));


    uint8_t bs[16] = {0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15};
    f = Sk4x4f::Transpose(bs);
    REPORTER_ASSERT(r, f.r[0] == 0 && f.r[1] == 4 && f.r[2] ==  8 && f.r[3] == 12);
    REPORTER_ASSERT(r, f.g[0] == 1 && f.g[1] == 5 && f.g[2] ==  9 && f.g[3] == 13);
    REPORTER_ASSERT(r, f.b[0] == 2 && f.b[1] == 6 && f.b[2] == 10 && f.b[3] == 14);
    REPORTER_ASSERT(r, f.a[0] == 3 && f.a[1] == 7 && f.a[2] == 11 && f.a[3] == 15);

    uint8_t bs_back[16];
    f.transpose(bs_back);
    REPORTER_ASSERT(r, 0 == memcmp(bs, bs_back, sizeof(bs)));
}
