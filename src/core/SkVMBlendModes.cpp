/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "src/core/SkVM.h"

bool skvm::BlendModeSupported(SkBlendMode mode) {
    switch (mode) {
        default: break;
        // our todo list
        case SkBlendMode::kHue:
        case SkBlendMode::kSaturation:
        case SkBlendMode::kColor:
        case SkBlendMode::kLuminosity:
            return false;
    }
    return true;
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
    }
}
