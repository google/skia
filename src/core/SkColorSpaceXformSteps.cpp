/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkColorSpaceXformSteps.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <cstring>

// See skia.org/docs/user/color  (== site/docs/user/color.md).

SkColorSpaceXformSteps::SkColorSpaceXformSteps(const SkColorSpace* src, SkAlphaType srcAT,
                                               const SkColorSpace* dst, SkAlphaType dstAT) {
    // Opaque outputs are treated as the same alpha type as the source input.
    // TODO: we'd really like to have a good way of explaining why we think this is useful.
    if (dstAT == kOpaque_SkAlphaType) {
        dstAT =  srcAT;
    }

    // We have some options about what to do with null src or dst here.
    // This pair seems to be the most consistent with legacy expectations.
    if (!src) { src = sk_srgb_singleton(); }
    if (!dst) { dst = src; }

    if (src->hash() == dst->hash() && srcAT == dstAT) {
        SkASSERT(SkColorSpace::Equals(src,dst));
        return;
    }

    this->fFlags.unpremul        = srcAT == kPremul_SkAlphaType;
    this->fFlags.linearize       = !src->gammaIsLinear();
    this->fFlags.gamut_transform = src->toXYZD50Hash() != dst->toXYZD50Hash();
    this->fFlags.encode          = !dst->gammaIsLinear();
    this->fFlags.premul          = srcAT != kOpaque_SkAlphaType && dstAT == kPremul_SkAlphaType;

    if (this->fFlags.gamut_transform) {
        skcms_Matrix3x3 src_to_dst;  // TODO: switch fSrcToDstMatrix to row-major
        src->gamutTransformTo(dst, &src_to_dst);

        this->fSrcToDstMatrix[0] = src_to_dst.vals[0][0];
        this->fSrcToDstMatrix[1] = src_to_dst.vals[1][0];
        this->fSrcToDstMatrix[2] = src_to_dst.vals[2][0];

        this->fSrcToDstMatrix[3] = src_to_dst.vals[0][1];
        this->fSrcToDstMatrix[4] = src_to_dst.vals[1][1];
        this->fSrcToDstMatrix[5] = src_to_dst.vals[2][1];

        this->fSrcToDstMatrix[6] = src_to_dst.vals[0][2];
        this->fSrcToDstMatrix[7] = src_to_dst.vals[1][2];
        this->fSrcToDstMatrix[8] = src_to_dst.vals[2][2];
    } else {
    #ifdef SK_DEBUG
        skcms_Matrix3x3 srcM, dstM;
        src->toXYZD50(&srcM);
        dst->toXYZD50(&dstM);
        SkASSERT(0 == memcmp(&srcM, &dstM, 9*sizeof(float)) && "Hash collision");
    #endif
    }

    // Fill out all the transfer functions we'll use.
    src->   transferFn(&this->fSrcTF   );
    dst->invTransferFn(&this->fDstTFInv);

    // If we linearize then immediately reencode with the same transfer function, skip both.
    if ( this->fFlags.linearize       &&
        !this->fFlags.gamut_transform &&
         this->fFlags.encode          &&
         src->transferFnHash() == dst->transferFnHash())
    {
    #ifdef SK_DEBUG
        skcms_TransferFunction dstTF;
        dst->transferFn(&dstTF);
        for (int i = 0; i < 7; i++) {
            SkASSERT( (&fSrcTF.g)[i] == (&dstTF.g)[i] && "Hash collision" );
        }
    #endif
        this->fFlags.linearize  = false;
        this->fFlags.encode     = false;
    }

    // Skip unpremul...premul if there are no non-linear operations between.
    if ( this->fFlags.unpremul   &&
        !this->fFlags.linearize  &&
        !this->fFlags.encode     &&
         this->fFlags.premul)
    {
        this->fFlags.unpremul = false;
        this->fFlags.premul   = false;
    }
}

void SkColorSpaceXformSteps::apply(float* rgba) const {
    if (this->fFlags.unpremul) {
        // I don't know why isfinite(x) stopped working on the Chromecast bots...
        auto is_finite = [](float x) { return x*0 == 0; };

        float invA = sk_ieee_float_divide(1.0f, rgba[3]);
        invA = is_finite(invA) ? invA : 0;
        rgba[0] *= invA;
        rgba[1] *= invA;
        rgba[2] *= invA;
    }
    if (this->fFlags.linearize) {
        rgba[0] = skcms_TransferFunction_eval(&fSrcTF, rgba[0]);
        rgba[1] = skcms_TransferFunction_eval(&fSrcTF, rgba[1]);
        rgba[2] = skcms_TransferFunction_eval(&fSrcTF, rgba[2]);
    }
    if (this->fFlags.gamut_transform) {
        float temp[3] = { rgba[0], rgba[1], rgba[2] };
        for (int i = 0; i < 3; ++i) {
            rgba[i] = fSrcToDstMatrix[    i] * temp[0] +
                      fSrcToDstMatrix[3 + i] * temp[1] +
                      fSrcToDstMatrix[6 + i] * temp[2];
        }
    }
    if (this->fFlags.encode) {
        rgba[0] = skcms_TransferFunction_eval(&fDstTFInv, rgba[0]);
        rgba[1] = skcms_TransferFunction_eval(&fDstTFInv, rgba[1]);
        rgba[2] = skcms_TransferFunction_eval(&fDstTFInv, rgba[2]);
    }
    if (this->fFlags.premul) {
        rgba[0] *= rgba[3];
        rgba[1] *= rgba[3];
        rgba[2] *= rgba[3];
    }
}

void SkColorSpaceXformSteps::apply(SkRasterPipeline* p) const {
    if (this->fFlags.unpremul)        { p->append(SkRasterPipelineOp::unpremul); }
    if (this->fFlags.linearize)       { p->appendTransferFunction(fSrcTF); }
    if (this->fFlags.gamut_transform) { p->append(SkRasterPipelineOp::matrix_3x3, &fSrcToDstMatrix); }
    if (this->fFlags.encode)          { p->appendTransferFunction(fDstTFInv); }
    if (this->fFlags.premul)          { p->append(SkRasterPipelineOp::premul); }
}
