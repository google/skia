/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformSteps.h"
#include "SkColorSpacePriv.h"
#include "SkRasterPipeline.h"
#include "../../third_party/skcms/skcms.h"

// TODO: explain

SkColorSpaceXformSteps::SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType srcAT,
                                               SkColorSpace* dst, SkAlphaType dstAT) {
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

    this->flags.unpremul        = srcAT == kPremul_SkAlphaType;
    this->flags.linearize       = !src->gammaIsLinear();
    this->flags.gamut_transform = src->toXYZD50Hash() != dst->toXYZD50Hash();
    this->flags.encode          = !dst->gammaIsLinear();
    this->flags.premul          = srcAT != kOpaque_SkAlphaType && dstAT == kPremul_SkAlphaType;

    if (this->flags.gamut_transform) {
        float row_major[9];  // TODO: switch src_to_dst_matrix to row-major
        src->gamutTransformTo(dst, row_major);

        this->src_to_dst_matrix[0] = row_major[0];
        this->src_to_dst_matrix[1] = row_major[3];
        this->src_to_dst_matrix[2] = row_major[6];

        this->src_to_dst_matrix[3] = row_major[1];
        this->src_to_dst_matrix[4] = row_major[4];
        this->src_to_dst_matrix[5] = row_major[7];

        this->src_to_dst_matrix[6] = row_major[2];
        this->src_to_dst_matrix[7] = row_major[5];
        this->src_to_dst_matrix[8] = row_major[8];
    } else {
    #ifdef SK_DEBUG
        skcms_Matrix3x3 srcM, dstM;
        src->toXYZD50(&srcM);
        dst->toXYZD50(&dstM);
        SkASSERT(0 == memcmp(&srcM, &dstM, 9*sizeof(float)) && "Hash collision");
    #endif
    }

    // Fill out all the transfer functions we'll use.
    src->   transferFn(&this->srcTF   .g);
    dst->invTransferFn(&this->dstTFInv.g);

    this->srcTF_is_sRGB = src->gammaCloseToSRGB();
    this->dstTF_is_sRGB = dst->gammaCloseToSRGB();

    // If we linearize then immediately reencode with the same transfer function, skip both.
    if ( this->flags.linearize       &&
        !this->flags.gamut_transform &&
         this->flags.encode          &&
         src->transferFnHash() == dst->transferFnHash())
    {
    #ifdef SK_DEBUG
        float dstTF[7];
        dst->transferFn(dstTF);
        for (int i = 0; i < 7; i++) {
            SkASSERT( (&srcTF.g)[i] == dstTF[i] && "Hash collision" );
        }
    #endif
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
        skcms_TransferFunction tf;
        memcpy(&tf, &srcTF, 7*sizeof(float));

        rgba[0] = skcms_TransferFunction_eval(&tf, rgba[0]);
        rgba[1] = skcms_TransferFunction_eval(&tf, rgba[1]);
        rgba[2] = skcms_TransferFunction_eval(&tf, rgba[2]);
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
        skcms_TransferFunction tf;
        memcpy(&tf, &dstTFInv, 7*sizeof(float));

        rgba[0] = skcms_TransferFunction_eval(&tf, rgba[0]);
        rgba[1] = skcms_TransferFunction_eval(&tf, rgba[1]);
        rgba[2] = skcms_TransferFunction_eval(&tf, rgba[2]);
    }
    if (flags.premul) {
        rgba[0] *= rgba[3];
        rgba[1] *= rgba[3];
        rgba[2] *= rgba[3];
    }
}

void SkColorSpaceXformSteps::apply(SkRasterPipeline* p, bool src_is_normalized) const {
#if defined(SK_LEGACY_SRGB_STAGE_CHOICE)
    src_is_normalized = true;
#endif
    if (flags.unpremul) { p->append(SkRasterPipeline::unpremul); }
    if (flags.linearize) {
        if (src_is_normalized && srcTF_is_sRGB) {
            p->append(SkRasterPipeline::from_srgb);
        } else if (srcTF.a == 1 &&
                   srcTF.b == 0 &&
                   srcTF.c == 0 &&
                   srcTF.d == 0 &&
                   srcTF.e == 0 &&
                   srcTF.f == 0) {
            p->append(SkRasterPipeline::gamma, &srcTF.g);
        } else {
            p->append(SkRasterPipeline::parametric, &srcTF);
        }
    }
    if (flags.gamut_transform) {
        p->append(SkRasterPipeline::matrix_3x3, &src_to_dst_matrix);
    }
    if (flags.encode) {
        if (src_is_normalized && dstTF_is_sRGB) {
            p->append(SkRasterPipeline::to_srgb);
        } else if (dstTFInv.a == 1 &&
                   dstTFInv.b == 0 &&
                   dstTFInv.c == 0 &&
                   dstTFInv.d == 0 &&
                   dstTFInv.e == 0 &&
                   dstTFInv.f == 0) {
            p->append(SkRasterPipeline::gamma, &dstTFInv.g);
        } else {
            p->append(SkRasterPipeline::parametric, &dstTFInv);
        }
    }
    if (flags.premul) { p->append(SkRasterPipeline::premul); }
}

//////////////

bool sk_can_use_legacy_blits(SkColorSpace* src, SkColorSpace* dst) {
    // When considering legacy blits, we only supported premul, so set those here
    SkAlphaType srcAT = kPremul_SkAlphaType;
    SkAlphaType dstAT = kPremul_SkAlphaType;

    return SkColorSpaceXformSteps(src, srcAT, dst, dstAT).flags.mask() == 0;
}
