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

    Sk4h red  = SkFloatToHalf_finite_ftz({ 1.0f, 0.0f, 0.0f, 1.0f }),
         blue = SkFloatToHalf_finite_ftz({ 0.0f, 0.0f, 0.5f, 0.5f }),
         result;

    SkRasterPipeline p;
    p.append(SkRasterPipeline::load_s_f16, &blue);
    p.append(SkRasterPipeline::load_d_f16, &red);
    p.append(SkRasterPipeline::srcover);
    p.append(SkRasterPipeline::store_f16, &result);
    p.run(1);

    Sk4f f = SkHalfToFloat_finite_ftz(result);

    // We should see half-intensity magenta.
    REPORTER_ASSERT(r, f[0] == 0.5f);
    REPORTER_ASSERT(r, f[1] == 0.0f);
    REPORTER_ASSERT(r, f[2] == 0.5f);
    REPORTER_ASSERT(r, f[3] == 1.0f);
}

DEF_TEST(SkRasterPipeline_empty, r) {
    // No asserts... just a test that this is safe to run.
    SkRasterPipeline p;
    p.run(20);
}

DEF_TEST(SkRasterPipeline_nonsense, r) {
    // No asserts... just a test that this is safe to run and terminates.
    // srcover() calls st->next(); this makes sure we've always got something there to call.
    SkRasterPipeline p;
    p.append(SkRasterPipeline::srcover);
    p.run(20);
}
