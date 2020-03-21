/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "src/core/SkVM.h"

// We're basing our implemenation of non-separable blend modes on
//   https://www.w3.org/TR/compositing-1/#blendingnonseparable.
// and
//   https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
// They're equivalent, but ES' math has been better simplified.
//
// Anything extra we add beyond that is to make the math work with premul inputs.

static skvm::F32 sat(skvm::Builder* p, skvm::F32 r, skvm::F32 g, skvm::F32 b) {
    return p->sub(p->max(r, p->max(g, b)), p->min(r, p->min(g, b)));
}

static skvm::F32 lum(skvm::Builder* p, skvm::F32 r, skvm::F32 g, skvm::F32 b) {
    return p->mad(r, p->splat(0.30f),
           p->mad(g, p->splat(0.59f),
           p->mul(b, p->splat(0.11f))));
}

static void set_sat(skvm::Builder* p, skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 s) {
    auto mn  = p->min(*r, p->min(*g, *b)),
         mx  = p->max(*r, p->max(*g, *b)),
         sat = p->sub(mx, mn);

    // Map min channel to 0, max channel to s, and scale the middle proportionally.
    auto scale = [&](auto c) {
        auto zero = p->splat(0.0f);
        return p->select(p->eq(sat, zero), zero, p->div(p->mul(p->sub(c, mn), s), sat));
    };
    *r = scale(*r);
    *g = scale(*g);
    *b = scale(*b);
}

static void set_lum(skvm::Builder* p, skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 lu) {
    auto diff = p->sub(lu, lum(p, *r, *g, *b));
    *r = p->add(*r, diff);
    *g = p->add(*g, diff);
    *b = p->add(*b, diff);
}

static void clip_color(skvm::Builder* p, skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 a) {
    auto mn = p->min(*r, p->min(*g, *b)),
         mx = p->max(*r, p->max(*g, *b)),
         lu = lum(p, *r, *g, *b);

    auto clip = [&](auto c) {
        c = p->select(p->gte(mn, p->splat(0.0f)),
                      c,
                      p->add(lu, p->div(p->mul(p->sub(c, lu), lu),            p->sub(lu, mn))));
        c = p->select(p->gt (mx, a),
                      p->add(lu, p->div(p->mul(p->sub(c, lu), p->sub(a, lu)), p->sub(mx, lu))),
                      c);
        return p->max(c, p->splat(0.0f));  // Sometimes without this we may dip just a little negative.
    };
    *r = clip(*r);
    *g = clip(*g);
    *b = clip(*b);
}

skvm::Color skvm::Builder::blend(SkBlendMode mode, Color src, Color dst) {
    auto mma = [this](skvm::F32 x, skvm::F32 y, skvm::F32 z, skvm::F32 w) {
        return mad(x,y, mul(z,w));
    };

    auto inv = [this](skvm::F32 x) { return sub(splat(1.0f), x); };

    auto two = [this](skvm::F32 x) { return add(x, x); };

    auto apply_rgba = [&](auto fn) {
        return Color {
            fn(src.r, dst.r),
            fn(src.g, dst.g),
            fn(src.b, dst.b),
            fn(src.a, dst.a),
        };
    };

    auto apply_rgb_srcover_a = [&](auto fn) {
        return Color {
            fn(src.r, dst.r),
            fn(src.g, dst.g),
            fn(src.b, dst.b),
            mad(dst.a, inv(src.a), src.a),   // srcover for alpha
        };
    };

    auto non_sep = [&](auto R, auto G, auto B) {
        return Color{
            add(mma(src.r, inv(dst.a), dst.r, inv(src.a)), R),
            add(mma(src.g, inv(dst.a), dst.g, inv(src.a)), G),
            add(mma(src.b, inv(dst.a), dst.b, inv(src.a)), B),
            mad(dst.a, inv(src.a), src.a),  // srcover
        };
    };

    switch (mode) {
        default: SkASSERT(false); /*but also, for safety, fallthrough*/

        case SkBlendMode::kClear: return {
            splat(0.0f),
            splat(0.0f),
            splat(0.0f),
            splat(0.0f),
        };

        case SkBlendMode::kSrc: return src;
        case SkBlendMode::kDst: return dst;

        case SkBlendMode::kDstOver: std::swap(src, dst); // fall-through
        case SkBlendMode::kSrcOver:
            return apply_rgba([&](auto s, auto d) {
                return mad(d, inv(src.a), s);
            });

        case SkBlendMode::kDstIn: std::swap(src, dst); // fall-through
        case SkBlendMode::kSrcIn:
            return apply_rgba([&](auto s, auto d) {
                return mul(s, dst.a);
            });

        case SkBlendMode::kDstOut: std::swap(src, dst); // fall-through
        case SkBlendMode::kSrcOut:
            return apply_rgba([&](auto s, auto d) {
                return mul(s, inv(dst.a));
            });

        case SkBlendMode::kDstATop: std::swap(src, dst); // fall-through
        case SkBlendMode::kSrcATop:
            return apply_rgba([&](auto s, auto d) {
                return mma(s, dst.a,  d, inv(src.a));
            });

        case SkBlendMode::kXor:
            return apply_rgba([&](auto s, auto d) {
                return mma(s, inv(dst.a),  d, inv(src.a));
            });

        case SkBlendMode::kPlus:
            return apply_rgba([&](auto s, auto d) {
                return min(add(s, d), splat(1.0f));
            });

        case SkBlendMode::kModulate:
            return apply_rgba([&](auto s, auto d) {
                return mul(s, d);
            });

        case SkBlendMode::kScreen:
            // (s+d)-(s*d) gave us trouble with our "r,g,b <= after blending" asserts.
            // It's kind of plausible that s + (d - sd) keeps more precision?
            return apply_rgba([&](auto s, auto d) {
                return add(s, sub(d, mul(s, d)));
            });

        case SkBlendMode::kDarken:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                return add(s, sub(d, max(mul(s, dst.a),
                                         mul(d, src.a))));
            });

        case SkBlendMode::kLighten:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                return add(s, sub(d, min(mul(s, dst.a),
                                         mul(d, src.a))));
            });

        case SkBlendMode::kDifference:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                return add(s, sub(d, two(min(mul(s, dst.a),
                                             mul(d, src.a)))));
            });

        case SkBlendMode::kExclusion:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                return add(s, sub(d, two(mul(s, d))));
            });

        case SkBlendMode::kColorBurn:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                auto mn   = min(dst.a,
                                div(mul(sub(dst.a, d), src.a), s)),
                     burn = mad(src.a, sub(dst.a, mn), mma(s, inv(dst.a), d, inv(src.a)));
                return select(eq(d, dst.a),       mad(s, inv(dst.a), d),
                       select(eq(s, splat(0.0f)), mul(d, inv(src.a)), burn));
            });

        case SkBlendMode::kColorDodge:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                auto tmp = mad(src.a, min(dst.a,
                                          div(mul(d, src.a), sub(src.a, s))),
                               mma(s, inv(dst.a), d, inv(src.a)));
                return select(eq(d, splat(0.0f)), mul(s, inv(dst.a)),
                       select(eq(s, src.a),       mad(d, inv(src.a), s), tmp));
            });

        case SkBlendMode::kHardLight:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                return add(mma(s, inv(dst.a), d, inv(src.a)),
                           select(lte(two(s), src.a),
                                  two(mul(s, d)),
                                  sub(mul(src.a, dst.a), two(mul(sub(dst.a, d), sub(src.a, s))))));
            });

        case SkBlendMode::kOverlay:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                return add(mma(s, inv(dst.a), d, inv(src.a)),
                           select(lte(two(d), dst.a),
                                  two(mul(s, d)),
                                  sub(mul(src.a, dst.a), two(mul(sub(dst.a, d), sub(src.a, s))))));
            });

        case SkBlendMode::kMultiply:
            return apply_rgba([&](auto s, auto d) {
                return add(mma(s, inv(dst.a), d, inv(src.a)), mul(s, d));
            });

        case SkBlendMode::kSoftLight:
            return apply_rgb_srcover_a([&](auto s, auto d) {
                auto  m = select(gt(dst.a, splat(0.0f)), div(d, dst.a), splat(0.0f)),
                     s2 = two(s),
                     m4 = two(two(m));

                     // The logic forks three ways:
                     //    1. dark src?
                     //    2. light src, dark dst?
                     //    3. light src, light dst?

                     // Used in case 1
                auto darkSrc = mul(d, mad(sub(s2, src.a), inv(m), src.a)),
                     // Used in case 2
                     darkDst = mad(mad(m4, m4, m4), sub(m, splat(1.0f)), mul(splat(7.0f), m)),
                     // Used in case 3.
                     liteDst = sub(sqrt(m), m),
                     // Used in 2 or 3?
                     liteSrc = mad(mul(dst.a, sub(s2, src.a)),
                                   select(lte(two(two(d)), dst.a), darkDst, liteDst),
                                   mul(d, src.a));
                return mad(s, inv(dst.a), mad(d,
                                              inv(src.a),
                                              select(lte(s2, src.a), darkSrc, liteSrc)));


            });

        case SkBlendMode::kHue: {
            skvm::F32 R = mul(src.r, src.a),
                      G = mul(src.g, src.a),
                      B = mul(src.b, src.a);

            set_sat(this, &R, &G, &B, mul(sat(this, dst.r, dst.g, dst.b), src.a));
            set_lum(this, &R, &G, &B, mul(lum(this, dst.r, dst.g, dst.b), src.a));
            clip_color(this, &R, &G, &B, mul(src.a, dst.a));

            return non_sep(R, G, B);
        }

        case SkBlendMode::kSaturation: {
            skvm::F32 R = mul(dst.r, src.a),
                      G = mul(dst.g, src.a),
                      B = mul(dst.b, src.a);

            set_sat(this, &R, &G, &B, mul(sat(this, src.r, src.g, src.b), dst.a));
            set_lum(this, &R, &G, &B, mul(lum(this, dst.r, dst.g, dst.b), src.a));
            clip_color(this, &R, &G, &B, mul(src.a, dst.a));

            return non_sep(R, G, B);
        }

        case SkBlendMode::kColor: {
            skvm::F32 R = mul(src.r, dst.a),
                      G = mul(src.g, dst.a),
                      B = mul(src.b, dst.a);

            set_lum(this, &R, &G, &B, mul(lum(this, dst.r, dst.g, dst.b), src.a));
            clip_color(this, &R, &G, &B, mul(src.a, dst.a));

            return non_sep(R, G, B);
        }

        case SkBlendMode::kLuminosity: {
            skvm::F32 R = mul(dst.r, src.a),
                      G = mul(dst.g, src.a),
                      B = mul(dst.b, src.a);

            set_lum(this, &R, &G, &B, mul(lum(this, src.r, src.g, src.b), dst.a));
            clip_color(this, &R, &G, &B, mul(src.a, dst.a));

            return non_sep(R, G, B);
        }
    }
}
