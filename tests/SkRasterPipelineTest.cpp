/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkHalf.h"
#include "SkRasterPipeline.h"

DEF_TEST(SkRasterPipeline, r) {
    // Build and run a simple pipeline to exercise SkRasterPipeline,
    // drawing 50% transparent blue over opaque red in half-floats.
    uint64_t red  = 0x3c00000000003c00ull,
             blue = 0x3800380000000000ull,
             result;

    void* load_s_ctx = &blue;
    void* load_d_ctx = &red;
    void* store_ctx  = &result;

    SkRasterPipeline p;
    p.append(SkRasterPipeline::load_f16, &load_s_ctx);
    p.append(SkRasterPipeline::load_f16_d, &load_d_ctx);
    p.append(SkRasterPipeline::srcover);
    p.append(SkRasterPipeline::store_f16, &store_ctx);
    p.run(0,0, 1);

    // We should see half-intensity magenta.
    REPORTER_ASSERT(r, ((result >>  0) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((result >> 16) & 0xffff) == 0x0000);
    REPORTER_ASSERT(r, ((result >> 32) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((result >> 48) & 0xffff) == 0x3c00);
}

DEF_TEST(SkRasterPipeline_empty, r) {
    // No asserts... just a test that this is safe to run.
    SkRasterPipeline p;
    p.run(0,0, 20);
}

DEF_TEST(SkRasterPipeline_nonsense, r) {
    // No asserts... just a test that this is safe to run and terminates.
    // srcover() calls st->next(); this makes sure we've always got something there to call.
    SkRasterPipeline p;
    p.append(SkRasterPipeline::srcover);
    p.run(0,0, 20);
}
