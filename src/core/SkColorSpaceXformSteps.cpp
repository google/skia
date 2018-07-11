/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformSteps.h"
#include "SkRasterPipeline.h"

// TODO: explain

SkColorSpaceXformSteps::SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType srcAT,
                                               SkColorSpace* dst) {
    // Set all bools to false, all floats to 0.0f.
    memset(this, 0, sizeof(*this));

    // We have some options about what to do with null src or dst here.
    SkASSERT(src && dst);

    this->flags.unpremul        = srcAT == kPremul_SkAlphaType;
    this->flags.linearize       = !src->gammaIsLinear();
    this->flags.gamut_transform = src->toXYZD50Hash() != dst->toXYZD50Hash();
    this->flags.encode          = !dst->gammaIsLinear();
    this->flags.premul          = srcAT != kOpaque_SkAlphaType;

    if (this->flags.gamut_transform && src->toXYZD50() && dst->fromXYZD50()) {
        auto xform = SkMatrix44(*dst->fromXYZD50(), *src->toXYZD50());
        if (xform.get(3,0) == 0 && xform.get(3,1) == 0 && xform.get(3,2) == 0 &&
            xform.get(3,3) == 1 &&
            xform.get(0,3) == 0 && xform.get(1,3) == 0 && xform.get(2,3) == 0) {

            for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++) {
                this->src_to_dst_matrix[3*c+r] = xform.get(r,c);
            }
        }
    }

    // Fill out all the transfer functions we'll use:
    SkColorSpaceTransferFn srcTF, dstTF;
    SkAssertResult(src->isNumericalTransferFn(&srcTF));
    SkAssertResult(dst->isNumericalTransferFn(&dstTF));
    this->srcTF         = srcTF;
    this->dstTFInv      = dstTF.invert();
    this->srcTF_is_sRGB = src->gammaCloseToSRGB();
    this->dstTF_is_sRGB = dst->gammaCloseToSRGB();

    // If we linearize then immediately reencode with the same transfer function, skip both.
    if ( this->flags.linearize       &&
        !this->flags.gamut_transform &&
         this->flags.encode          &&
        0 == memcmp(&srcTF, &dstTF, sizeof(SkColorSpaceTransferFn)))
    {
        this->flags.linearize  = false;
        this->flags.encode     = false;
    }

    // Skip unpremul...premul if there are no non-linear operations between.
    if ( this->flags.unpremul   &&
        !this->flags.linearize  &&
        !this->flags.encode     &&
         this->flags.premul)
    {
        this->flags.unpremul = false;
        this->flags.premul   = false;
    }
}

void SkColorSpaceXformSteps::apply(float* rgba) const {
    if (flags.unpremul) {
        // I don't know why isfinite(x) stopped working on the Chromecast bots...
        auto is_finite = [](float x) { return x*0 == 0; };

        float invA = is_finite(1.0f / rgba[3]) ? 1.0f / rgba[3] : 0;
        rgba[0] *= invA;
        rgba[1] *= invA;
        rgba[2] *= invA;
    }
    if (flags.linearize) {
        rgba[0] = srcTF(rgba[0]);
        rgba[1] = srcTF(rgba[1]);
        rgba[2] = srcTF(rgba[2]);
    }
    if (flags.gamut_transform) {
        float temp[3] = { rgba[0], rgba[1], rgba[2] };
        for (int i = 0; i < 3; ++i) {
            rgba[i] = src_to_dst_matrix[    i] * temp[0] +
                      src_to_dst_matrix[3 + i] * temp[1] +
                      src_to_dst_matrix[6 + i] * temp[2];
        }
    }
    if (flags.encode) {
        rgba[0] = dstTFInv(rgba[0]);
        rgba[1] = dstTFInv(rgba[1]);
        rgba[2] = dstTFInv(rgba[2]);
    }
    if (flags.premul) {
        rgba[0] *= rgba[3];
        rgba[1] *= rgba[3];
        rgba[2] *= rgba[3];
    }
}

void SkColorSpaceXformSteps::apply(SkRasterPipeline* p) const {
    if (flags.unpremul) { p->append(SkRasterPipeline::unpremul); }
    if (flags.linearize) {
        if (srcTF_is_sRGB) {
            p->append(SkRasterPipeline::from_srgb);
        } else if (srcTF.fA == 1 &&
                   srcTF.fB == 0 &&
                   srcTF.fC == 0 &&
                   srcTF.fD == 0 &&
                   srcTF.fE == 0 &&
                   srcTF.fF == 0) {
            p->append(SkRasterPipeline::gamma, &srcTF.fG);
        } else {
            p->append(SkRasterPipeline::parametric, &srcTF);
        }
    }
    if (flags.gamut_transform) {
        p->append(SkRasterPipeline::matrix_3x3, &src_to_dst_matrix);
    }
    if (flags.encode) {
        if (dstTF_is_sRGB) {
            p->append(SkRasterPipeline::to_srgb);
        } else if (dstTFInv.fA == 1 &&
                   dstTFInv.fB == 0 &&
                   dstTFInv.fC == 0 &&
                   dstTFInv.fD == 0 &&
                   dstTFInv.fE == 0 &&
                   dstTFInv.fF == 0) {
            p->append(SkRasterPipeline::gamma, &dstTFInv.fG);
        } else {
            p->append(SkRasterPipeline::parametric, &dstTFInv);
        }
    }
    if (flags.premul) { p->append(SkRasterPipeline::premul); }
}
