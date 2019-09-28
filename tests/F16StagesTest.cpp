/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRasterPipeline.h"
#include "tests/Test.h"

DEF_TEST(F16Stages, r) {
    // Make sure SkRasterPipeline::load_f16 and store_f16 can handle a range of
    // ordinary (0<=x<=1) and interesting (x<0, x>1) values.
    float floats[16] = {
        0.0f, 0.25f, 0.5f, 1.0f,
        -1.25f, -0.5f, 1.25f, 2.0f,
        0,0,0,0, 0,0,0,0,  // pad a bit to make sure we qualify for platform-specific code
    };
    uint64_t halfs[4] = {0,0,0,0};

    SkRasterPipeline_MemoryCtx f32 = { floats, 0 },
                               f16 = { halfs,  0 };

    {
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline:: load_f32, &f32);
        p.append(SkRasterPipeline::store_f16, &f16);
        p.run(0,0,16/4,1);
    }
    REPORTER_ASSERT(r, ((halfs[0] >>  0) & 0xffff) == 0x0000);
    REPORTER_ASSERT(r, ((halfs[0] >> 16) & 0xffff) == 0x3400);
    REPORTER_ASSERT(r, ((halfs[0] >> 32) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((halfs[0] >> 48) & 0xffff) == 0x3c00);
    REPORTER_ASSERT(r, ((halfs[1] >>  0) & 0xffff) == 0xbd00);
    REPORTER_ASSERT(r, ((halfs[1] >> 16) & 0xffff) == 0xb800);
    REPORTER_ASSERT(r, ((halfs[1] >> 32) & 0xffff) == 0x3d00);
    REPORTER_ASSERT(r, ((halfs[1] >> 48) & 0xffff) == 0x4000);

    {
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline:: load_f16, &f16);
        p.append(SkRasterPipeline::store_f32, &f32);
        p.run(0,0,16/4,1);
    }
    REPORTER_ASSERT(r, floats[0] ==  0.00f);
    REPORTER_ASSERT(r, floats[1] ==  0.25f);
    REPORTER_ASSERT(r, floats[2] ==  0.50f);
    REPORTER_ASSERT(r, floats[3] ==  1.00f);
    REPORTER_ASSERT(r, floats[4] == -1.25f);
    REPORTER_ASSERT(r, floats[5] == -0.50f);
    REPORTER_ASSERT(r, floats[6] ==  1.25f);
    REPORTER_ASSERT(r, floats[7] ==  2.00f);
}
