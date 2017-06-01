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

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f32, &ip);
    p.append(SkRasterPipeline::parametric_r, &fn);
    p.append(SkRasterPipeline::parametric_g, &fn);
    p.append(SkRasterPipeline::parametric_b, &fn);
    p.append(SkRasterPipeline::parametric_a, &fn);
    p.append(SkRasterPipeline::store_f32, &op);

    p.run(0,0, 256/4);


    for (int i = 0; i < 256; i++) {
        float want = (in[i] <= fn.fD) ? fn.fC * in[i] + fn.fF
                                      : powf(in[i] * fn.fA + fn.fB, fn.fG) + fn.fE;
        float err = fabsf(out[i] - want);
        if (err > limit) {
            ERRORF(r, "At %d, error was %g (got %g, want %g)", i, err, out[i], want);
        }
    }
}

static void check_error(skiatest::Reporter* r, float limit, float gamma) {
    SkColorSpaceTransferFn fn = {0,0,0,0,0,0,0};
    fn.fG = gamma;
    fn.fA = 1;
    check_error(r, limit, fn);
}

DEF_TEST(Parametric_sRGB, r) {
    // Test our good buddy the sRGB transfer function in resplendent 7-parameter glory.
    check_error(r, 1/510.0f, {
        2.4f,
        1.0f / 1.055f,
        0.055f / 1.055f,
        1.0f / 12.92f,
        0.04045f,
        0.0f,
        0.0f,
    });
}

// A nice little spread of simple gammas.
DEF_TEST(Parametric_1dot0, r) { check_error(r, 1/510.0f, 1.0f); }

DEF_TEST(Parametric_1dot2, r) { check_error(r, 1/510.0f, 1.2f); }
DEF_TEST(Parametric_1dot4, r) { check_error(r, 1/510.0f, 1.4f); }
DEF_TEST(Parametric_1dot8, r) { check_error(r, 1/510.0f, 1.8f); }
DEF_TEST(Parametric_2dot0, r) { check_error(r, 1/510.0f, 2.0f); }
DEF_TEST(Parametric_2dot2, r) { check_error(r, 1/510.0f, 2.2f); }
DEF_TEST(Parametric_2dot4, r) { check_error(r, 1/510.0f, 2.4f); }

DEF_TEST(Parametric_inv_1dot2, r) { check_error(r, 1/510.0f, 1/1.2f); }
DEF_TEST(Parametric_inv_1dot4, r) { check_error(r, 1/510.0f, 1/1.4f); }
DEF_TEST(Parametric_inv_1dot8, r) { check_error(r, 1/510.0f, 1/1.8f); }
DEF_TEST(Parametric_inv_2dot0, r) { check_error(r, 1/510.0f, 1/2.0f); }
DEF_TEST(Parametric_inv_2dot2, r) { check_error(r, 1/510.0f, 1/2.2f); }
DEF_TEST(Parametric_inv_2dot4, r) { check_error(r, 1/510.0f, 1/2.4f); }
