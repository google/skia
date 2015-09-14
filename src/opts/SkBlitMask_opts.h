/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_opts_DEFINED
#define SkBlitMask_opts_DEFINED

#include "Sk4px.h"
#include "SkPx.h"

namespace SK_OPTS_NS {

template <typename Fn>
static void blit_mask_d32_a8(const Fn& fn, SkPMColor* dst, size_t dstRB,
                                           const SkAlpha* mask, size_t maskRB,
                                           int w, int h) {
    while (h --> 0) {
        int n = w, N = SkPx::N;
        while (n >= N) {
            fn(SkPx::LoadN(dst), SkPx::Alpha::LoadN(mask)).storeN(dst);
            dst += N; mask += N; n -= N;
        }
        if (n > 0) {
            fn(SkPx::Load(n, dst), SkPx::Alpha::Load(n, mask)).store(n, dst);
            dst += n; mask += n;
        }
        dst  +=  dstRB / sizeof(*dst)  - w;
        mask += maskRB / sizeof(*mask) - w;
    }
}

static void blit_mask_d32_a8(SkPMColor* dst, size_t dstRB,
                             const SkAlpha* mask, size_t maskRB,
                             SkColor color, int w, int h) {
    auto s = SkPx::Dup(SkPreMultiplyColor(color));

    if (color == SK_ColorBLACK) {
        auto fn = [](const SkPx& d, const SkPx::Alpha& aa) {
            //   = (s + d(1-sa))aa + d(1-aa)
            //   = s*aa + d(1-sa*aa)
            //   ~~~>
            // a = 1*aa + d(1-1*aa) = aa + d(1-aa)
            // c = 0*aa + d(1-1*aa) =      d(1-aa)
            return d.approxMulDiv255(aa.inv()).addAlpha(aa);
        };
        blit_mask_d32_a8(fn, dst, dstRB, mask, maskRB, w, h);
    } else if (SkColorGetA(color) == 0xFF) {
        auto fn = [&](const SkPx& d, const SkPx::Alpha& aa) {
            //  = (s + d(1-sa))aa + d(1-aa)
            //  = s*aa + d(1-sa*aa)
            //   ~~~>
            //  = s*aa + d(1-aa)
            return s.approxMulDiv255(aa) + d.approxMulDiv255(aa.inv());
        };
        blit_mask_d32_a8(fn, dst, dstRB, mask, maskRB, w, h);
    } else {
        auto fn = [&](const SkPx& d, const SkPx::Alpha& aa) {
            //  = (s + d(1-sa))aa + d(1-aa)
            //  = s*aa + d(1-sa*aa)
            auto left  = s.approxMulDiv255(aa),
                 right = d.approxMulDiv255(left.alpha().inv());
            return left + right;  // This does not overflow (exhaustively checked).
        };
        blit_mask_d32_a8(fn, dst, dstRB, mask, maskRB, w, h);
    }
}

}  // SK_OPTS_NS

#endif//SkBlitMask_opts_DEFINED
