/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterPipeline.h"
#include "SkTypes.h"
#include "Test.h"

#include <math.h>

DEF_TEST(srgb_roundtrip, r) {
    uint32_t reds[256];
    for (int i = 0; i < 256; i++) {
        reds[i] = i;
    }

    SkRasterPipeline_MemoryCtx ptr = { reds, 0 };

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_8888,  &ptr);
    p.append(SkRasterPipeline::from_srgb);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_8888, &ptr);

    p.run(0,0,256,1);

    for (int i = 0; i < 256; i++) {
        if (reds[i] != (uint32_t)i) {
            ERRORF(r, "%d doesn't round trip, %d", i, reds[i]);
        }
    }
}

DEF_TEST(srgb_edge_cases, r) {
    // We need to run at least 4 pixels to make sure we hit all specializations.
    float colors[4][4] = { {0,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };
    auto& color = colors[0];

    SkRasterPipeline_MemoryCtx dst = { &color, 0 };

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);
    p.append_constant_color(&alloc, color);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_f32, &dst);
    p.run(0,0,4,1);

    if (color[0] != 0.0f) {
        ERRORF(r, "expected to_srgb() to map 0.0f to 0.0f, got %f", color[0]);
    }
    if (color[1] != 1.0f) {
        float f = color[1];
        uint32_t x;
        memcpy(&x, &f, 4);
        ERRORF(r, "expected to_srgb() to map 1.0f to 1.0f, got %f (%08x)", color[1], x);
    }
}

DEF_TEST(srgb_roundtrip_extended, r) {
    // Test the quality of our roundtripping with values outside [0,1].
    SkColor4f rgba[128],
              back[128];

    const float range = 2.0f,  // test 0..range
                tol = 1.025f;  // expect to be within this ratio

    auto close = [&](float x, float y) {
        return x == y
            || (x/y < tol && y/x < tol);
    };

    for (int i = 0; i < 128; i++) {
        rgba[i] = SkColor4f{ range/128, 0,0,1 };
    }

    SkRasterPipeline_MemoryCtx src = { rgba, 0 },
                               dst = { back, 0 };

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f32,  &src);
    p.append(SkRasterPipeline::from_srgb);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_f32, &dst);
    p.run(0,0,128,1);

    for (int i = 0; i < 128; i++) {
        REPORTER_ASSERT(r, close(rgba[i].fR, back[i].fR));
    }
}
