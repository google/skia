/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorSpaceXformSteps.h"
#include "SkRasterPipeline.h"

// TODO: explain

SkColorSpaceXformSteps::SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType srcAT,
                                               SkColorSpace* dst) {
    // Set all bools to false, all floats to 0.0f.
    memset(this, 0, sizeof(*this));

    // We have some options about what to do with null src or dst here.
    SkASSERT(src && dst);

    this->unpremul        = srcAT == kPremul_SkAlphaType;
    this->linearize       = !src->gammaIsLinear();
    this->gamut_transform = src->toXYZD50Hash() != dst->toXYZD50Hash();
    this->encode          = !dst->gammaIsLinear();
    this->premul          = srcAT != kOpaque_SkAlphaType;

    if (this->gamut_transform && src->toXYZD50() && dst->fromXYZD50()) {
        auto xform = SkMatrix44(*dst->fromXYZD50(), *src->toXYZD50());
        if (xform.get(3,0) == 0 && xform.get(3,1) == 0 && xform.get(3,2) == 0 &&
            xform.get(3,3) == 1 &&
            xform.get(0,3) == 0 && xform.get(1,3) == 0 && xform.get(2,3) == 0) {

            for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++) {
                this->src_to_dst_matrix[3*r+c] = xform.get(r,c);
            }
        }
    }

    // Fill out all the transfer functions we'll use:
    SkColorSpaceTransferFn srcTF, dstTF;
    SkAssertResult(src->isNumericalTransferFn(&srcTF));
    SkAssertResult(dst->isNumericalTransferFn(&dstTF));
    this->srcTF    = srcTF;
    this->dstTFInv = dstTF.invert();

    // If we linearize then immediately reencode with the same transfer function, skip both.
    if ( this->linearize       &&
        !this->gamut_transform &&
         this->encode          &&
        0 == memcmp(&srcTF, &dstTF, sizeof(SkColorSpaceTransferFn)))
    {
        this->linearize  = false;
        this->encode     = false;
    }

    // Skip unpremul...premul if there are no non-linear operations between.
    if ( this->unpremul   &&
        !this->linearize  &&
        !this->encode     &&
         this->premul)
    {
        this->unpremul = false;
        this->premul   = false;
    }
}

// We don't use this, but the implementation of SkAppendColorSpaceXform should change
// when we remove SkColorSpaceXformCanvas, to treat dst == nullptr as sRGB just like src.
#include "SkColorSpaceXformCanvas.h"

void SkAppendColorSpaceXform(SkRasterPipeline* p, SkArenaAlloc* alloc,
                             SkColorSpace* src,
                             SkColorSpace* dst,
                             SkAlphaType srcAT) {
    // nullptr dst is currently a signal that src is already in the destination colorspace.
    if (!dst) {
        if (srcAT == kUnpremul_SkAlphaType) {
            p->append(SkRasterPipeline::premul);
        }
        return;
    }

    // nullptr src always means sRGB (and one day, so will nullptr dst).
    sk_sp<SkColorSpace> sRGB;
    if (!src) {
        sRGB = SkColorSpace::MakeSRGB();
        src = sRGB.get();
    }

    SkColorSpaceXformSteps steps{src,srcAT,dst};

    if (steps.unpremul) { p->append(SkRasterPipeline::unpremul); }
    if (steps.linearize) {
        if (src->gammaCloseToSRGB()) {
            p->append(SkRasterPipeline::from_srgb);
        } else {
            auto tf = alloc->make<SkColorSpaceTransferFn>(steps.srcTF);
            p->append(SkRasterPipeline::parametric_r, tf);
            p->append(SkRasterPipeline::parametric_g, tf);
            p->append(SkRasterPipeline::parametric_b, tf);
        }
    }
    if (steps.gamut_transform) {
        auto m = memcpy(alloc->makeArray<float>(9), steps.src_to_dst_matrix, 9*sizeof(float));
        p->append(SkRasterPipeline::matrix_3x3, m);
    }
    if (steps.encode) {
        if (dst->gammaCloseToSRGB()) {
            p->append(SkRasterPipeline::to_srgb);
        } else {
            auto tf = alloc->make<SkColorSpaceTransferFn>(steps.dstTFInv);
            p->append(SkRasterPipeline::parametric_r, tf);
            p->append(SkRasterPipeline::parametric_g, tf);
            p->append(SkRasterPipeline::parametric_b, tf);
        }
    }
    if (steps.premul)   { p->append(SkRasterPipeline::premul); }
}
