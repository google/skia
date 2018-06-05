/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformSteps.h"

// TODO: explain

SkColorSpaceXformSteps::SkColorSpaceXformSteps(const SkColorSpace* src, SkAlphaType srcAT,
                                               const SkColorSpace* dst) {
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
        auto xform = SkMatrix44(*src->toXYZD50(),  *dst->fromXYZD50());
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
