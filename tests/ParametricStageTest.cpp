/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skcms/skcms.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "tests/Test.h"

#include <cmath>

static void check_error(skiatest::Reporter* r, float limit, skcms_TransferFunction fn) {
    float in[256], out[256];
    for (int i = 0; i < 256; i++) {
        in [i] = i / 255.0f;
        out[i] = 0.0f;  // Not likely important.  Just being tidy.
    }

    SkRasterPipeline_MemoryCtx ip = { in, 0},
                               op = {out, 0};

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::load_f32, &ip);
    p.appendTransferFunction(fn);
    p.append(SkRasterPipelineOp::store_f32, &op);

    p.run(0,0, 256/4,1);


    for (int i = 0; i < 256; i++) {
        float want = (in[i] <= fn.d) ? fn.c * in[i] + fn.f
                                     : powf(in[i] * fn.a + fn.b, fn.g) + fn.e;
        if (i % 4 == 3) {  // alpha should stay unchanged.
            want = in[i];
        }
        float err = fabsf(out[i] - want);
        if (err > limit) {
            ERRORF(r, "At %d, error was %g (got %g, want %g)", i, err, out[i], want);
        }
    }
}

static void check_error(skiatest::Reporter* r, float limit, float gamma) {
    skcms_TransferFunction fn = {0,0,0,0,0,0,0};
    fn.g = gamma;
    fn.a = 1;
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
