/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../src/jumper/SkJumper.h"
#include "SkColorSpace_New.h"
#include "SkRasterPipeline.h"
#include "Test.h"
#include <initializer_list>

DEF_TEST(SkColorSpace_New_TransferFnBasics, r) {
    auto gamut = SkMatrix44::I();
    auto blending = SkColorSpace_New::Blending::AsEncoded;

    SkColorSpace_New linearA{SkColorSpace_New::TransferFn::MakeLinear(),    gamut, blending},
                     linearB{SkColorSpace_New::TransferFn::MakeGamma(1),    gamut, blending},
                        srgb{SkColorSpace_New::TransferFn::MakeSRGB(),      gamut, blending},
                       gamma{SkColorSpace_New::TransferFn::MakeGamma(2.2f), gamut, blending};

    REPORTER_ASSERT(r,  linearA.gammaIsLinear());
    REPORTER_ASSERT(r,  linearB.gammaIsLinear());
    REPORTER_ASSERT(r, !   srgb.gammaIsLinear());
    REPORTER_ASSERT(r, !  gamma.gammaIsLinear());

    REPORTER_ASSERT(r, !linearA.gammaCloseToSRGB());
    REPORTER_ASSERT(r, !linearB.gammaCloseToSRGB());
    REPORTER_ASSERT(r,     srgb.gammaCloseToSRGB());
    REPORTER_ASSERT(r, !  gamma.gammaCloseToSRGB());

    REPORTER_ASSERT(r,  linearA.transferFn().equals(linearB.transferFn()));
    REPORTER_ASSERT(r, !linearA.transferFn().equals(   srgb.transferFn()));
    REPORTER_ASSERT(r, !linearA.transferFn().equals(  gamma.transferFn()));
    REPORTER_ASSERT(r, !linearB.transferFn().equals(   srgb.transferFn()));
    REPORTER_ASSERT(r, !linearB.transferFn().equals(  gamma.transferFn()));
    REPORTER_ASSERT(r, !   srgb.transferFn().equals(  gamma.transferFn()));
}

DEF_TEST(SkColorSpace_New_TransferFnStages, r) {
    // We'll create a little SkRasterPipelineBlitter-like scenario,
    // blending the same src color over the same dst color, but with
    // three different transfer functions, for simplicity the same for src and dst.
    SkColor src = 0x7f7f0000;

    SkColor dsts[3];
    for (SkColor& dst : dsts) {
        dst = 0xff007f00;
    }

    auto gamut = SkMatrix44::I();
    auto blending = SkColorSpace_New::Blending::Linear;
    SkColorSpace_New linear{SkColorSpace_New::TransferFn::MakeLinear(), gamut, blending},
                       srgb{SkColorSpace_New::TransferFn::MakeSRGB(),   gamut, blending},
                      gamma{SkColorSpace_New::TransferFn::MakeGamma(3), gamut, blending};
    SkColor* dst = dsts;
    for (const SkColorSpace_New* cs : {&linear, &srgb, &gamma}) {
        SkJumper_MemoryCtx src_ctx = {  &src, 0 },
                           dst_ctx = { dst++, 0 };

        SkRasterPipeline_<256> p;

        p.append(SkRasterPipeline::load_8888, &src_ctx);
        cs->transferFn().linearizeSrc(&p);
        p.append(SkRasterPipeline::premul);

        p.append(SkRasterPipeline::load_8888_dst, &dst_ctx);
        cs->transferFn().linearizeDst(&p);
        p.append(SkRasterPipeline::premul_dst);

        p.append(SkRasterPipeline::srcover);
        p.append(SkRasterPipeline::unpremul);
        cs->transferFn().encodeSrc(&p);
        p.append(SkRasterPipeline::store_8888, &dst_ctx);
        p.run(0,0,1,1);
    }

    // Double check the uninteresting channels: alpha's opaque, no blue.
    REPORTER_ASSERT(r, SkColorGetA(dsts[0]) == 0xff && SkColorGetB(dsts[0]) == 0x00);
    REPORTER_ASSERT(r, SkColorGetA(dsts[1]) == 0xff && SkColorGetB(dsts[1]) == 0x00);
    REPORTER_ASSERT(r, SkColorGetA(dsts[2]) == 0xff && SkColorGetB(dsts[2]) == 0x00);

    // Because we're doing linear blending, a more-exponential transfer function will
    // brighten the encoded values more when linearizing. So we expect to see that
    // linear is darker than sRGB, and sRGB in turn is darker than gamma 3.
    REPORTER_ASSERT(r, SkColorGetR(dsts[0]) < SkColorGetR(dsts[1]));
    REPORTER_ASSERT(r, SkColorGetR(dsts[1]) < SkColorGetR(dsts[2]));

    REPORTER_ASSERT(r, SkColorGetG(dsts[0]) < SkColorGetG(dsts[1]));
    REPORTER_ASSERT(r, SkColorGetG(dsts[1]) < SkColorGetG(dsts[2]));

}
