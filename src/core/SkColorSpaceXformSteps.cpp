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

skvm::Color SkColorSpaceXformSteps::program(skvm::Builder* p, skvm::Uniforms* uniforms,
                                            skvm::Color c) const {
    if (flags.unpremul) {
        c = p->unpremul(c);
    }
    if (flags.linearize) {
        c = skvm::transfer_fn(p, uniforms, srcTF, c);
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
        c = skvm::transfer_fn(p, uniforms, dstTFInv, c);
    }
    if (flags.premul) {
        c = p->premul(c);
    }
    return c;
}

namespace skvm {

HSLA rgb_to_hsl(Builder* p, Color c) {
    auto mx = p->max(p->max(c.r,c.g),c.b),
         mn = p->min(p->min(c.r,c.g),c.b),
          d = p->sub(mx,mn),
      d_rcp = p->div(p->splat(1.0f),d),
     g_lt_b = p->select(p->lt(c.g,c.b), p->splat(6.0f), p->splat(0.0f));

    auto diffm = [&](auto a, auto b) { return p->mul(p->sub(a,b), d_rcp); };

    auto h = p->mul(p->splat(1/6.0f),
                    p->select(p->eq(mx,mn),   p->splat(0.0f),
                    p->select(p->eq(mx, c.r), p->add(diffm(c.g,c.b), g_lt_b),
                    p->select(p->eq(mx, c.g), p->add(diffm(c.b,c.r), p->splat(2.0f)),
                                              p->add(diffm(c.r,c.g), p->splat(4.0f))))));

    auto sum = p->add(mx,mn);
    auto   l = p->mul(sum, p->splat(0.5f));
    auto   s = p->select(p->eq(mx,mn), p->splat(0.0f),
                                       p->div(d,
                                              p->select(p->gt(l,p->splat(0.5f)), p->sub(p->splat(2.0f),sum),
                                                                                 sum)));
    return {h, s, l, c.a};
}

Color hsl_to_rgb(Builder* p, HSLA c) {
    // See GrRGBToHSLFilterEffect.fp

    auto h = c.h,
         s = c.s,
         l = c.l,
         x = p->mul(p->sub(p->splat(1.0f), p->abs(p->sub(p->add(l,l),
                                                         p->splat(1.0f)))),
                    s);

    auto hue_to_rgb = [&](auto hue) {
        auto q = p->sub(p->abs(p->mad(p->fract(hue),p->splat(6.0f), p->splat(-3.0f))), p->splat(1.0f));
        return p->mad(p->sub(p->clamp01(q), p->splat(0.5f)), x, l);
    };

    return {
        hue_to_rgb(p->add(h, p->splat(0.0f/3.0f))),
        hue_to_rgb(p->add(h, p->splat(2.0f/3.0f))),
        hue_to_rgb(p->add(h, p->splat(1.0f/3.0f))),
        c.a
    };
}

Color transfer_fn(Builder* p, Uniforms* uniforms, const skcms_TransferFunction& tf, Color c) {
    F32 G = p->uniformF(uniforms->pushF(tf.g)),
        A = p->uniformF(uniforms->pushF(tf.a)),
        B = p->uniformF(uniforms->pushF(tf.b)),
        C = p->uniformF(uniforms->pushF(tf.c)),
        D = p->uniformF(uniforms->pushF(tf.d)),
        E = p->uniformF(uniforms->pushF(tf.e)),
        F = p->uniformF(uniforms->pushF(tf.f));

    auto apply = [&](F32 v) -> F32 {
        // Strip off the sign bit and save it for later.
        I32 bits = p->bit_cast(v),
            sign = p->bit_and(bits,p->splat(0x80000000));
        v = p->bit_cast(p->bit_xor(bits, sign));

        switch (classify_transfer_fn(tf)) {
            case Bad_TF: SkASSERT(false); break;

            case sRGBish_TF:
                v = p->select(p->lte(v,D), p->mad(C, v, F)
                                         , p->add(p->approx_powf(p->mad(A, v, B), G), E));
                break;

            case PQish_TF:
                v = p->approx_powf(p->div(p->max(p->mad(B, p->approx_powf(v, C), A), p->splat(0.f)),
                                                 p->mad(E, p->approx_powf(v, C), D)),
                                   F);
                break;

            case HLGish_TF: {
                auto vA = p->mul(v,A);
                v = p->select(p->lte(vA,p->splat(1.0f)), p->approx_powf(vA, B)
                                                       , p->approx_exp(p->mad(p->sub(v,E),C, D)));
            } break;

            case HLGinvish_TF:
                v = p->select(p->lte(v,p->splat(1.0f)), p->mul(A, p->approx_powf(v, B))
                                                      , p->mad(C, p->approx_log(p->sub(v,D)), E));
                break;
        }

        // Re-apply the original sign bit on our way out the door.
        return p->bit_cast(p->bit_or(sign, p->bit_cast(v)));
    };

    return {apply(c.r), apply(c.g), apply(c.b), c.a};
}

}   // skvm namespace
