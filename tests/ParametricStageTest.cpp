/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkRasterPipeline.h"
#include "Test.h"

static float measure_error(SkColorSpaceTransferFn fn) {
    float in[256], out[256];
    for (int i = 0; i < 256; i++) {
        in [i] = i / 255.0f;
        out[i] = 0.0f;  // Not likely important.  Just being tidy.
    }

    const float* ip = in;
    float*       op = out;

    SkRasterPipeline p;
    p.append(SkRasterPipeline::load_f32, &ip);
    p.append(SkRasterPipeline::parametric_r, &fn);
    p.append(SkRasterPipeline::parametric_g, &fn);
    p.append(SkRasterPipeline::parametric_b, &fn);
    p.append(SkRasterPipeline::parametric_a, &fn);
    p.append(SkRasterPipeline::store_f32, &op);

    p.run(0, 256/4);


    float max_err = 0;
    for (int i = 0; i < 256; i++) {
        float goal = (in[i] <= fn.fD) ? fn.fC * in[i] + fn.fF
                                      : powf(in[i] * fn.fA + fn.fB, fn.fG) + fn.fE;
        max_err = SkTMax(max_err,
                         out[i] == goal ? 0 : fabsf(out[i] - goal) / goal);
    }
    return max_err;
}

DEF_TEST(Parametric_sRGB, r) {
    // Our good buddy the sRGB transfer function in resplendent 7-parameter glory.
    SkColorSpaceTransferFn fn = {
        2.4f,
        1.0f / 1.055f,
        0.055f / 1.055f,
        1.0f / 12.92f,
        0.04045f,
        0.0f,
        0.0f,
    };
    REPORTER_ASSERT(r, measure_error(fn) <= 0.01f);
}

DEF_TEST(Parametric_simple, r) {
    // A nice little spread of gammas.
    float gammas[] = { 1.2f, 1.4f, 1.8f, 2.0f, 2.2f, 2.4f };
    for (float gamma : gammas) {
        SkColorSpaceTransferFn fn = {
            gamma,
            0,0,0,0,0,0,
        };
        REPORTER_ASSERT(r, measure_error(fn) <= 0.01f);
    }
    for (float gamma : gammas) {
        SkColorSpaceTransferFn fn = {
            1.0f/gamma,
            0,0,0,0,0,0,
        };
        REPORTER_ASSERT(r, measure_error(fn) <= 0.01f);
    }
}
