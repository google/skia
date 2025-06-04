/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkColorSpaceXformSteps.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <cmath>
#include <cstring>

// See skia.org/docs/user/color  (== site/docs/user/color.md).

// Compute the Y vector for the HLG OOTF in the primaries of a specified color space. The value
// is specified in Rec2020 primaries in ITU-R BT.2100.
static void set_ootf_Y(const SkColorSpace* cs, float* Y) {
    skcms_Matrix3x3 m;
    cs->gamutTransformTo(
        SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kRec2020).get(),
        &m);
    constexpr float Y_rec2020[3] = {0.262700f, 0.678000f, 0.059300f};
    for (int i = 0; i < 3; ++i) {
        Y[i] = 0.f;
        for (int j = 0; j < 3; ++j) {
            Y[i] += m.vals[j][i] * Y_rec2020[j];
        }
    }
}

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

    skcms_TransferFunction srcTrfn;
    src->transferFn(&srcTrfn);
    skcms_TransferFunction dstTrfn;
    dst->transferFn(&dstTrfn);

    // The scale factor is the amount that values in linear space will be scaled to accommodate
    // peak luminance and HDR reference white luminance.
    float scaleFactor = 1.f;

    switch (skcms_TransferFunction_getType(&srcTrfn)) {
        case skcms_TFType_PQ:
            // PQ is always scaled by a peak luminance of 10,000 nits, then divided by the HDR
            // reference white luminance (a).
            scaleFactor *= 10000.f / srcTrfn.a;
            // Use the default PQish transfer function.
            // TODO(https://issues.skia.org/issues/420956739): Inline the use of this function when
            // the PQish functions are no longer used.
            this->fSrcTF = SkNamedTransferFn::kPQ;
            this->fFlags.linearize = true;
            break;
        case skcms_TFType_HLG:
            // HLG is scaled by the peak luminance (b), then divided by the HDR reference white
            // luminance (a).
            scaleFactor *= srcTrfn.b / srcTrfn.a;
            this->fFlags.linearize = true;
            // Use the HLGish transfer function scaled by 1/12.
            // TODO(https://issues.skia.org/issues/420956739): Inline the use of this function when
            // the HLGish functions are no longer used.
            this->fSrcTF = SkNamedTransferFn::kHLG;
            this->fSrcTF.f = 1/12.f - 1.f;
            // If the system gamma is not 1.0, then compute the parameters for the OOTF.
            if (srcTrfn.c != 1.f) {
                this->fFlags.src_ootf = true;
                this->fSrcOotf[3] = srcTrfn.c - 1.f;
                set_ootf_Y(src, this->fSrcOotf);
            }
            break;
        default:
            this->fFlags.linearize = memcmp(&srcTrfn, &SkNamedTransferFn::kLinear, sizeof(srcTrfn)) != 0;
            if (this->fFlags.linearize) {
              src->transferFn(&this->fSrcTF);
            }
            break;
    }

    switch (skcms_TransferFunction_getType(&dstTrfn)) {
        case skcms_TFType_PQ:
            // This is the inverse of the treatment of source PQ.
            scaleFactor /= 10000.f / dstTrfn.a;
            this->fFlags.encode = true;
            this->fDstTFInv = SkNamedTransferFn::kPQ;
            skcms_TransferFunction_invert(&this->fDstTFInv, &this->fDstTFInv);
            break;
        case skcms_TFType_HLG:
            // This is the inverse of the treatment of source HLG.
            scaleFactor /= dstTrfn.b / dstTrfn.a;
            this->fFlags.encode = true;
            this->fDstTFInv = SkNamedTransferFn::kHLG;
            this->fDstTFInv.f = 1/12.f - 1.f;
            skcms_TransferFunction_invert(&this->fDstTFInv, &this->fDstTFInv);
            if (dstTrfn.c != 1.f) {
                this->fFlags.dst_ootf = true;
                this->fDstOotf[3] = 1/dstTrfn.c - 1.f;
                set_ootf_Y(dst, this->fDstOotf);
            }
            break;
        default:
            this->fFlags.encode = memcmp(&dstTrfn, &SkNamedTransferFn::kLinear, sizeof(dstTrfn)) != 0;
            if (this->fFlags.encode) {
              dst->invTransferFn(&this->fDstTFInv);
            }
            break;
    }

    this->fFlags.unpremul        = srcAT == kPremul_SkAlphaType;
    this->fFlags.gamut_transform = src->toXYZD50Hash() != dst->toXYZD50Hash() ||
                                   scaleFactor != 1.f;
    this->fFlags.premul          = srcAT != kOpaque_SkAlphaType && dstAT == kPremul_SkAlphaType;

    if (this->fFlags.gamut_transform) {
        skcms_Matrix3x3 src_to_dst;  // TODO: switch fSrcToDstMatrix to row-major
        src->gamutTransformTo(dst, &src_to_dst);

        this->fSrcToDstMatrix[0] = src_to_dst.vals[0][0] * scaleFactor;
        this->fSrcToDstMatrix[1] = src_to_dst.vals[1][0] * scaleFactor;
        this->fSrcToDstMatrix[2] = src_to_dst.vals[2][0] * scaleFactor;

        this->fSrcToDstMatrix[3] = src_to_dst.vals[0][1] * scaleFactor;
        this->fSrcToDstMatrix[4] = src_to_dst.vals[1][1] * scaleFactor;
        this->fSrcToDstMatrix[5] = src_to_dst.vals[2][1] * scaleFactor;

        this->fSrcToDstMatrix[6] = src_to_dst.vals[0][2] * scaleFactor;
        this->fSrcToDstMatrix[7] = src_to_dst.vals[1][2] * scaleFactor;
        this->fSrcToDstMatrix[8] = src_to_dst.vals[2][2] * scaleFactor;
    } else {
    #ifdef SK_DEBUG
        skcms_Matrix3x3 srcM, dstM;
        src->toXYZD50(&srcM);
        dst->toXYZD50(&dstM);
        SkASSERT(0 == memcmp(&srcM, &dstM, 9*sizeof(float)) && "Hash collision");
    #endif
    }

    // If the source and destination OOTFs cancel each other out, skip both.
    if ( this->fFlags.src_ootf        &&
        !this->fFlags.gamut_transform &&
         this->fFlags.dst_ootf) {
        // If there is no gamut transform, then the r,g,b coefficients for the
        // OOTFs must be the same.
        SkASSERT(0 == memcmp(&this->fSrcOotf, &this->fDstOotf, 3*sizeof(float)));
        // If the gammas cancel out, then remove the steps.
        if ((this->fSrcOotf[3] + 1.f) * (this->fDstOotf[3] + 1.f) == 1.f) {
            this->fFlags.src_ootf = false;
            this->fFlags.dst_ootf = false;
        }
    }

    // If we linearize then immediately reencode with the same transfer function, skip both.
    if ( this->fFlags.linearize       &&
        !this->fFlags.src_ootf        &&
        !this->fFlags.gamut_transform &&
        !this->fFlags.dst_ootf        &&
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
    if (this->fFlags.src_ootf) {
        const float Y = fSrcOotf[0] * rgba[0] +
                        fSrcOotf[1] * rgba[1] +
                        fSrcOotf[2] * rgba[2];
        const float Y_to_gamma_minus_1 = std::pow(Y, fSrcOotf[3]);
        rgba[0] *= Y_to_gamma_minus_1;
        rgba[1] *= Y_to_gamma_minus_1;
        rgba[2] *= Y_to_gamma_minus_1;
    }
    if (this->fFlags.gamut_transform) {
        float temp[3] = { rgba[0], rgba[1], rgba[2] };
        for (int i = 0; i < 3; ++i) {
            rgba[i] = fSrcToDstMatrix[    i] * temp[0] +
                      fSrcToDstMatrix[3 + i] * temp[1] +
                      fSrcToDstMatrix[6 + i] * temp[2];
        }
    }
    if (this->fFlags.dst_ootf) {
        const float Y = fDstOotf[0] * rgba[0] +
                        fDstOotf[1] * rgba[1] +
                        fDstOotf[2] * rgba[2];
        const float Y_to_gamma_minus_1 = std::pow(Y, fDstOotf[3]);
        rgba[0] *= Y_to_gamma_minus_1;
        rgba[1] *= Y_to_gamma_minus_1;
        rgba[2] *= Y_to_gamma_minus_1;
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
