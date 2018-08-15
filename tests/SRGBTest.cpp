/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkTypes.h"
#include "Test.h"
#include <math.h>
#include "../src/jumper/SkJumper.h"

DEF_TEST(sk_pipeline_srgb_roundtrip, r) {
    uint32_t reds[256];
    for (int i = 0; i < 256; i++) {
        reds[i] = i;
    }

    SkJumper_MemoryCtx ptr = { reds, 0 };

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

DEF_TEST(sk_pipeline_srgb_edge_cases, r) {
    // We need to run at least 4 pixels to make sure we hit all specializations.
    SkPM4f colors[4] = { {{0,1,1,1}}, {{0,0,0,0}}, {{0,0,0,0}}, {{0,0,0,0}} };
    auto& color = colors[0];

    SkJumper_MemoryCtx dst = { &color, 0 };

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);
    p.append_constant_color(&alloc, color);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_f32, &dst);
    p.run(0,0,4,1);

    if (color.r() != 0.0f) {
        ERRORF(r, "expected to_srgb() to map 0.0f to 0.0f, got %f", color.r());
    }
    if (color.g() != 1.0f) {
        float f = color.g();
        uint32_t x;
        memcpy(&x, &f, 4);
        ERRORF(r, "expected to_srgb() to map 1.0f to 1.0f, got %f (%08x)", color.g(), x);
    }
}
