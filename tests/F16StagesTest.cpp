/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterPipeline.h"
#include "Test.h"

DEF_TEST(F16Stages, r) {
    // Make sure SkRasterPipeline::load_f16 and store_f16 can handle a range of
    // ordinary (0<=x<=1) and interesting (x<0, x>1) values.
    float floats[16] = {
        0.0f, 0.25f, 0.5f, 1.0f,
        -1.25f, -0.5f, 1.25f, 2.0f,
        0,0,0,0, 0,0,0,0,  // pad a bit to make sure we qualify for platform-specific code
    };
    uint16_t halfs[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

    float*    f32 = floats;
    uint16_t* f16 = halfs;

    {
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline:: load_f32, &f32);
        p.append(SkRasterPipeline::store_f16, &f16);
        p.run(0,0,16/4);
    }
    REPORTER_ASSERT(r, f16[0] == 0x0000);
    REPORTER_ASSERT(r, f16[1] == 0x3400);
    REPORTER_ASSERT(r, f16[2] == 0x3800);
    REPORTER_ASSERT(r, f16[3] == 0x3c00);
    REPORTER_ASSERT(r, f16[4] == 0xbd00);
    REPORTER_ASSERT(r, f16[5] == 0xb800);
    REPORTER_ASSERT(r, f16[6] == 0x3d00);
    REPORTER_ASSERT(r, f16[7] == 0x4000);

    {
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline:: load_f16, &f16);
        p.append(SkRasterPipeline::store_f32, &f32);
        p.run(0,0,16/4);
    }
    REPORTER_ASSERT(r, f32[0] ==  0.00f);
    REPORTER_ASSERT(r, f32[1] ==  0.25f);
    REPORTER_ASSERT(r, f32[2] ==  0.50f);
    REPORTER_ASSERT(r, f32[3] ==  1.00f);
    REPORTER_ASSERT(r, f32[4] == -1.25f);
    REPORTER_ASSERT(r, f32[5] == -0.50f);
    REPORTER_ASSERT(r, f32[6] ==  1.25f);
    REPORTER_ASSERT(r, f32[7] ==  2.00f);
}
