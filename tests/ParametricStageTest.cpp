/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkRasterPipeline.h"
#include "Test.h"

static void check_error(skiatest::Reporter* r, float limit, SkColorSpaceTransferFn fn) {
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


    for (int i = 0; i < 256; i++) {
        float goal = (in[i] <= fn.fD) ? fn.fC * in[i] + fn.fF
                                      : powf(in[i] * fn.fA + fn.fB, fn.fG) + fn.fE;
        float err = out[i] == goal ? 0 : fabsf(out[i] - goal) / goal;

        // For most values make sure we're under the limit.
        REPORTER_ASSERT(r, err < limit);

        // 0 and 1 are serious business.
        if (i == 0 || i == 255) {
            REPORTER_ASSERT(r, err < limit*limit);
        }
    }
}

DEF_TEST(Parametric_sRGB, r) {
    // Test our good buddy the sRGB transfer function in resplendent 7-parameter glory.
    check_error(r, 0.01f, {
        2.4f,
        1.0f / 1.055f,
        0.055f / 1.055f,
        1.0f / 12.92f,
        0.04045f,
        0.0f,
        0.0f,
    });
}

DEF_TEST(Parametric_simple, r) {
    // A nice little spread of gammas.
    float gammas[] = { 1.2f, 1.4f, 1.8f, 2.0f, 2.2f, 2.4f };
    for (float gamma : gammas) {
        check_error(r, 0.01f, {
            gamma,
            0,0,0,0,0,0,
        });
        check_error(r, 0.01f, {
            1.0f/gamma,
            0,0,0,0,0,0,
        });
    }
}
