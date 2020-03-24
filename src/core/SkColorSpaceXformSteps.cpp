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

// TODO(mtklein): explain the logic of this file

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

    this->srcTF_is_sRGB = src->gammaCloseToSRGB();
    this->dstTF_is_sRGB = dst->gammaCloseToSRGB();

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

        float invA = is_finite(1.0f / rgba[3]) ? 1.0f / rgba[3] : 0;
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

void SkColorSpaceXformSteps::apply(SkRasterPipeline* p, bool src_is_normalized) const {
#if defined(SK_LEGACY_SRGB_STAGE_CHOICE)
    src_is_normalized = true;
#endif
    if (flags.unpremul) { p->append(SkRasterPipeline::unpremul); }
    if (flags.linearize) {
        if (src_is_normalized && srcTF_is_sRGB) {
            p->append(SkRasterPipeline::from_srgb);
        } else {
            p->append_transfer_function(srcTF);
        }
    }
    if (flags.gamut_transform) {
        p->append(SkRasterPipeline::matrix_3x3, &src_to_dst_matrix);
    }
    if (flags.encode) {
        if (src_is_normalized && dstTF_is_sRGB) {
            p->append(SkRasterPipeline::to_srgb);
        } else {
            p->append_transfer_function(dstTFInv);
        }
    }
    if (flags.premul) { p->append(SkRasterPipeline::premul); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static skvm::F32 strip_sign(skvm::Builder* p, skvm::F32 x, skvm::I32* sign) {
    skvm::I32 bits = p->bit_cast(x);
    *sign = p->bit_and(bits, p->splat(0x80000000));
    return p->bit_cast(p->bit_xor(bits, *sign));
}

static skvm::F32 apply_sign(skvm::Builder* p, skvm::F32 x, skvm::I32 sign) {
    return p->bit_cast(p->bit_or(sign, p->bit_cast(x)));
}

namespace skvm {
    struct TransferFunction {
        F32 g, a,b,c,d,e,f;

        TransferFunction(Builder* p, Uniforms* u, const skcms_TransferFunction& tf)
            : g(p->uniformF(u->pushF(tf.g)))
            , a(p->uniformF(u->pushF(tf.a)))
            , b(p->uniformF(u->pushF(tf.b)))
            , c(p->uniformF(u->pushF(tf.c)))
            , d(p->uniformF(u->pushF(tf.d)))
            , e(p->uniformF(u->pushF(tf.e)))
            , f(p->uniformF(u->pushF(tf.f)))
        {}

        F32 noop(Builder* p, F32 v) const { return v; }

        F32 parametric(Builder* p, F32 v) const {
            return p->select(p->lte(v,d), p->mad(c, v, f)
                                        , p->add(p->approx_powf(p->mad(a, v, b), g), e));
        }

        F32 PQish(Builder* p, F32 v) const {
            return p->approx_powf(p->div(p->max(p->mad(b, p->approx_powf(v, c), a), p->splat(0.0f)),
                                                p->mad(e, p->approx_powf(v, c), d)),
                                  f);
        }

        F32 HLGish(Builder* p, F32 v) const {
            auto va = p->mul(v,a);
            return p->select(p->lte(va,p->splat(1.0f)), p->approx_powf(va, b)
                                                      , p->approx_exp(p->mad(p->sub(v,e),c, d)));
        }

        F32 HLGinvish(Builder* p, F32 v) const {
            return p->select(p->lte(v,p->splat(1.0f)), p->mul(a, p->approx_powf(v, b))
                                                     , p->mad(c, p->approx_log(p->sub(v,d)), e));
        }
    };
}

static skvm::Color apply_transfer_function(skvm::Builder* p, skvm::Uniforms* uniforms,
                                           const skcms_TransferFunction& tf, skvm::Color c) {
    skvm::TransferFunction vtf(p, uniforms, tf);

    auto fn = &skvm::TransferFunction::noop;

    switch (classify_transfer_fn(tf)) {
        case sRGBish_TF:   fn = &skvm::TransferFunction::parametric; break;
        case PQish_TF:     fn = &skvm::TransferFunction::PQish;      break;
        case HLGish_TF:    fn = &skvm::TransferFunction::HLGish;     break;
        case HLGinvish_TF: fn = &skvm::TransferFunction::HLGinvish;  break;
        case Bad_TF: break;
    }

    auto apply = [&](skvm::F32 v) {
        skvm::I32 sign;
        v = strip_sign(p, v, &sign);
        v = (vtf.*fn)(p, v);
        return apply_sign(p, v, sign);
    };
    return {apply(c.r), apply(c.g), apply(c.b), c.a};
}

skvm::Color SkColorSpaceXformSteps::program(skvm::Builder* p, skvm::Uniforms* uniforms,
                                            skvm::Color c) const {
    if (flags.unpremul) {
        c = p->unpremul(c);
    }
    if (flags.linearize) {
        c = apply_transfer_function(p, uniforms, srcTF, c);
    }
    if (flags.gamut_transform) {
        skvm::F32 m[9];
        for (int i = 0; i < 9; ++i) {
            m[i] = p->uniformF(uniforms->pushF(src_to_dst_matrix[i]));
        }
        auto R = p->mad(c.r,m[0], p->mad(c.g,m[3], p->mul(c.b,m[6]))),
             G = p->mad(c.r,m[1], p->mad(c.g,m[4], p->mul(c.b,m[7]))),
             B = p->mad(c.r,m[2], p->mad(c.g,m[5], p->mul(c.b,m[8])));
        c = {R, G, B, c.a};
    }
    if (flags.encode) {
        c = apply_transfer_function(p, uniforms, dstTFInv, c);
    }
    if (flags.premul) {
        c = p->premul(c);
    }
    return c;
}
