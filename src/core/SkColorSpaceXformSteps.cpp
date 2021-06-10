/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/third_party/skcms/skcms.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkVM.h"

// See skia.org/user/color  (== site/user/color.md).

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

    this->flags.unpremul        = srcAT == kPremul_SkAlphaType;
    this->flags.linearize       = !src->gammaIsLinear();
    this->flags.gamut_transform = src->toXYZD50Hash() != dst->toXYZD50Hash();
    this->flags.encode          = !dst->gammaIsLinear();
    this->flags.premul          = srcAT != kOpaque_SkAlphaType && dstAT == kPremul_SkAlphaType;

    if (this->flags.gamut_transform) {
        skcms_Matrix3x3 src_to_dst;  // TODO: switch src_to_dst_matrix to row-major
        src->gamutTransformTo(dst, &src_to_dst);

        this->src_to_dst_matrix[0] = src_to_dst.vals[0][0];
        this->src_to_dst_matrix[1] = src_to_dst.vals[1][0];
        this->src_to_dst_matrix[2] = src_to_dst.vals[2][0];

        this->src_to_dst_matrix[3] = src_to_dst.vals[0][1];
        this->src_to_dst_matrix[4] = src_to_dst.vals[1][1];
        this->src_to_dst_matrix[5] = src_to_dst.vals[2][1];

        this->src_to_dst_matrix[6] = src_to_dst.vals[0][2];
        this->src_to_dst_matrix[7] = src_to_dst.vals[1][2];
        this->src_to_dst_matrix[8] = src_to_dst.vals[2][2];
    } else {
    #ifdef SK_DEBUG
        skcms_Matrix3x3 srcM, dstM;
        src->toXYZD50(&srcM);
        dst->toXYZD50(&dstM);
        SkASSERT(0 == memcmp(&srcM, &dstM, 9*sizeof(float)) && "Hash collision");
    #endif
    }

    // Fill out all the transfer functions we'll use.
    src->   transferFn(&this->srcTF   );
    dst->invTransferFn(&this->dstTFInv);

    // If we linearize then immediately reencode with the same transfer function, skip both.
    if ( this->flags.linearize       &&
        !this->flags.gamut_transform &&
         this->flags.encode          &&
         src->transferFnHash() == dst->transferFnHash())
    {
    #ifdef SK_DEBUG
        skcms_TransferFunction dstTF;
        dst->transferFn(&dstTF);
        for (int i = 0; i < 7; i++) {
            SkASSERT( (&srcTF.g)[i] == (&dstTF.g)[i] && "Hash collision" );
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

        float invA = sk_ieee_float_divide(1.0f, rgba[3]);
        invA = is_finite(invA) ? invA : 0;
        rgba[0] *= invA;
        rgba[1] *= invA;
        rgba[2] *= invA;
    }
    if (flags.linearize) {
        rgba[0] = skcms_TransferFunction_eval(&srcTF, rgba[0]);
        rgba[1] = skcms_TransferFunction_eval(&srcTF, rgba[1]);
        rgba[2] = skcms_TransferFunction_eval(&srcTF, rgba[2]);
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
        rgba[0] = skcms_TransferFunction_eval(&dstTFInv, rgba[0]);
        rgba[1] = skcms_TransferFunction_eval(&dstTFInv, rgba[1]);
        rgba[2] = skcms_TransferFunction_eval(&dstTFInv, rgba[2]);
    }
    if (flags.premul) {
        rgba[0] *= rgba[3];
        rgba[1] *= rgba[3];
        rgba[2] *= rgba[3];
    }
}

void SkColorSpaceXformSteps::apply(SkRasterPipeline* p) const {
    if (flags.unpremul)        { p->append(SkRasterPipeline::unpremul); }
    if (flags.linearize)       { p->append_transfer_function(srcTF); }
    if (flags.gamut_transform) { p->append(SkRasterPipeline::matrix_3x3, &src_to_dst_matrix); }
    if (flags.encode)          { p->append_transfer_function(dstTFInv); }
    if (flags.premul)          { p->append(SkRasterPipeline::premul); }
}

skvm::Color sk_program_transfer_fn(skvm::Builder* p, skvm::Uniforms* uniforms,
                                   const skcms_TransferFunction& tf, skvm::Color c) {
    skvm::F32 G = p->uniformF(uniforms->pushF(tf.g)),
              A = p->uniformF(uniforms->pushF(tf.a)),
              B = p->uniformF(uniforms->pushF(tf.b)),
              C = p->uniformF(uniforms->pushF(tf.c)),
              D = p->uniformF(uniforms->pushF(tf.d)),
              E = p->uniformF(uniforms->pushF(tf.e)),
              F = p->uniformF(uniforms->pushF(tf.f));

    auto apply = [&](skvm::F32 v) -> skvm::F32 {
        // Strip off the sign bit and save it for later.
        skvm::I32 bits = pun_to_I32(v),
                  sign = bits & 0x80000000;
        v = pun_to_F32(bits ^ sign);

        switch (classify_transfer_fn(tf)) {
            case Bad_TF: SkASSERT(false); break;

            case sRGBish_TF:
                v = select(v <= D, C*v + F
                                 , approx_powf(A*v + B, G) + E);
                break;

            case PQish_TF: {
                skvm::F32 vC = approx_powf(v, C);
                v = approx_powf(max(B * vC + A, 0.0f) / (E * vC + D), F);
            } break;

            case HLGish_TF: {
                skvm::F32 vA = v*A,
                           K = F + 1.0f;
                v = K*select(vA <= 1.0f, approx_powf(vA, B)
                                       , approx_exp((v-E) * C + D));
            } break;

            case HLGinvish_TF:
                skvm::F32 K = F + 1.0f;
                v /= K;
                v = select(v <= 1.0f, A * approx_powf(v, B)
                                    , C * approx_log(v-D) + E);
                break;
        }

        // Re-apply the original sign bit on our way out the door.
        return pun_to_F32(sign | pun_to_I32(v));
    };

    return {apply(c.r), apply(c.g), apply(c.b), c.a};
}

skvm::Color SkColorSpaceXformSteps::program(skvm::Builder* p, skvm::Uniforms* uniforms,
                                            skvm::Color c) const {
    if (flags.unpremul) {
        c = unpremul(c);
    }
    if (flags.linearize) {
        c = sk_program_transfer_fn(p, uniforms, srcTF, c);
    }
    if (flags.gamut_transform) {
        auto m = [&](int index) {
            return p->uniformF(uniforms->pushF(src_to_dst_matrix[index]));
        };
        auto R = c.r * m(0) + c.g * m(3) + c.b * m(6),
             G = c.r * m(1) + c.g * m(4) + c.b * m(7),
             B = c.r * m(2) + c.g * m(5) + c.b * m(8);
        c = {R, G, B, c.a};
    }
    if (flags.encode) {
        c = sk_program_transfer_fn(p, uniforms, dstTFInv, c);
    }
    if (flags.premul) {
        c = premul(c);
    }
    return c;
}
