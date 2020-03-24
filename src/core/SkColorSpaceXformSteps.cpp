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

static skvm::Color from_srgb(skvm::Builder* p, skvm::Color c) {
    auto fn = [p](skvm::F32 s) {
        skvm::I32 sign;
        s = strip_sign(p, s, &sign);

        auto lo = p->mul(s, p->splat(1/12.92f));
        auto hi = p->mad(p->mul(s,s),
                         p->mad(s, p->splat(0.3000f), p->splat(0.6975f)),
                         p->splat(0.0025f));

        return apply_sign(p, p->select(p->lt(s, p->splat(0.055f)), lo, hi), sign);
    };
    return {fn(c.r), fn(c.g), fn(c.b), c.a};
}

static skvm::Color to_srgb(skvm::Builder* p, skvm::Color c) {
    auto fn = [p](skvm::F32 s) {
        skvm::I32 sign;
        s = strip_sign(p, s, &sign);

#if 0
        // We tweak c and d for each instruction set to make sure fn(1) is exactly 1.
    #if defined(JUMPER_IS_AVX512)
        const float c = 1.130026340485f,
                    d = 0.141387879848f;
    #elif defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41) || \
          defined(JUMPER_IS_AVX ) || defined(JUMPER_IS_HSW )
        const float c = 1.130048394203f,
                    d = 0.141357362270f;
    #elif defined(JUMPER_IS_NEON)
        const float c = 1.129999995232f,
                    d = 0.141381442547f;
    #else
        const float c = 1.129999995232f,
                    d = 0.141377761960f;
    #endif
#endif
        auto  c = p->splat(1.129999995232f);
        auto  d = p->splat(0.141377761960f);

        auto  t = p->div(p->splat(1.0f), p->sqrt(s));
        auto lo = p->mul(s, p->splat(12.92f));
        auto hi = p->div(p->mad(t, p->mad(t, p->splat(-0.0024542345f), p->splat(0.013832027f)), c),
                         p->add(d, s));

        return apply_sign(p, p->select(p->lt(s, p->splat(0.00465985f)), lo, hi), sign);
    };
    return {fn(c.r), fn(c.g), fn(c.b), c.a};
}

static skvm::Color apply_gamma(skvm::Builder* p, skvm::F32 gamma, skvm::Color c) {
    auto fn = [&](auto v) {
        skvm::I32 sign;
        v = strip_sign(p, v, &sign);
        v = p->approx_powf(v, gamma);
        v = apply_sign(p, v, sign);
        return v;
    };
    return {fn(c.r), fn(c.g), fn(c.b), c.a};
}

struct skvm_TransferFunction {
    skvm::F32 g, a,b,c,d,e,f;

    skvm_TransferFunction(skvm::Builder* p, skvm::Uniforms* u, const skcms_TransferFunction& tf)
        : g(p->uniformF(u->pushF(tf.g)))
        , a(p->uniformF(u->pushF(tf.a)))
        , b(p->uniformF(u->pushF(tf.b)))
        , c(p->uniformF(u->pushF(tf.c)))
        , d(p->uniformF(u->pushF(tf.d)))
        , e(p->uniformF(u->pushF(tf.e)))
        , f(p->uniformF(u->pushF(tf.f)))
    {}
};

static skvm::Color apply_parametric(skvm::Builder* p, const skvm_TransferFunction& tf,
                                    skvm::Color c) {
    auto fn = [&](auto v) {
        skvm::I32 sign;
        v = strip_sign(p, v, &sign);
        v = p->select(p->lte(v, tf.d), p->mad(tf.c, v, tf.f)
                                     , p->add(p->approx_powf(p->mad(tf.a, v, tf.b), tf.g), tf.e));
        return apply_sign(p, v, sign);
    };
    return {fn(c.r), fn(c.g), fn(c.b), c.a};
}

#if 0
STAGE(PQish, const skcms_TransferFunction* ctx) {
    auto fn = [&](F v) {
        U32 sign;
        v = strip_sign(v, &sign);

        F r = approx_powf(max(mad(ctx->b, approx_powf(v, ctx->c), ctx->a), 0)
                           / (mad(ctx->e, approx_powf(v, ctx->c), ctx->d)),
                        ctx->f);

        return apply_sign(r, sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(HLGish, const skcms_TransferFunction* ctx) {
    auto fn = [&](F v) {
        U32 sign;
        v = strip_sign(v, &sign);

        const float R = ctx->a, G = ctx->b,
                    a = ctx->c, b = ctx->d, c = ctx->e;

        F r = if_then_else(v*R <= 1, approx_powf(v*R, G)
                                   , approx_exp((v-c)*a) + b);

        return apply_sign(r, sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(HLGinvish, const skcms_TransferFunction* ctx) {
    auto fn = [&](F v) {
        U32 sign;
        v = strip_sign(v, &sign);

        const float R = ctx->a, G = ctx->b,
                    a = ctx->c, b = ctx->d, c = ctx->e;

        F r = if_then_else(v <= 1, R * approx_powf(v, G)
                                 , a * approx_log(v - b) + c);

        return apply_sign(r, sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}
#endif

static skvm::Color apply_transfer_function(skvm::Builder* p, skvm::Uniforms* uniforms,
                                           const skcms_TransferFunction& tf, skvm::Color c) {
    switch (classify_transfer_fn(tf)) {
        default:
        case Bad_TF: SkASSERT(false); c = {}; break;

        case TFKind::sRGBish_TF:
            if (tf.a == 1 && tf.b == 0 && tf.c == 0 && tf.d == 0 && tf.e == 0 && tf.f == 0) {
                c = apply_gamma(p, p->uniformF(uniforms->pushF(tf.g)), c);
            } else {
                c = apply_parametric(p, skvm_TransferFunction(p, uniforms, tf), c);
            }
            break;
#if 0
        case PQish_TF:     this->unchecked_append(PQish,     ctx); break;
        case HLGish_TF:    this->unchecked_append(HLGish,    ctx); break;
        case HLGinvish_TF: this->unchecked_append(HLGinvish, ctx); break;
#endif
    }
    return c;
}

skvm::Color SkColorSpaceXformSteps::apply(skvm::Builder* p, skvm::Uniforms* uniforms,
                                          skvm::Color c, bool src_is_normalized) const {
#if defined(SK_LEGACY_SRGB_STAGE_CHOICE)
    src_is_normalized = true;
#endif
    if (flags.unpremul) { c = p->unpremul(c); }
    if (flags.linearize) {
        if (src_is_normalized && srcTF_is_sRGB) {
            c = from_srgb(p, c);
        } else {
            c = apply_transfer_function(p, uniforms, srcTF, c);
        }
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
        if (src_is_normalized && dstTF_is_sRGB) {
            c = to_srgb(p, c);
        } else {
            c = apply_transfer_function(p, uniforms, dstTFInv, c);
        }
    }
    if (flags.premul) { c = p->premul(c); }
    return c;
}
