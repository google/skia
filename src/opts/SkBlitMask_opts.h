/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_opts_DEFINED
#define SkBlitMask_opts_DEFINED

#include "Sk4px.h"

namespace SK_OPTS_NS {

static void blit_mask_d32_a8_general(SkPMColor* dst, size_t dstRB,
                                     const SkAlpha* mask, size_t maskRB,
                                     SkColor color, int w, int h) {
    auto s = Sk4px::DupPMColor(SkPreMultiplyColor(color));
    auto fn = [&](const Sk4px& d, const Sk4px& aa) {
        //  = (s + d(1-sa))aa + d(1-aa)
        //  = s*aa + d(1-sa*aa)
        auto left  = s.approxMulDiv255(aa),
             right = d.approxMulDiv255(left.alphas().inv());
        return left + right;  // This does not overflow (exhaustively checked).
    };
    while (h --> 0) {
        Sk4px::MapDstAlpha(w, dst, mask, fn);
        dst  +=  dstRB / sizeof(*dst);
        mask += maskRB / sizeof(*mask);
    }
}

// As above, but made slightly simpler by requiring that color is opaque.
static void blit_mask_d32_a8_opaque(SkPMColor* dst, size_t dstRB,
                                    const SkAlpha* mask, size_t maskRB,
                                    SkColor color, int w, int h) {
    SkASSERT(SkColorGetA(color) == 0xFF);
    auto s = Sk4px::DupPMColor(SkPreMultiplyColor(color));
    auto fn = [&](const Sk4px& d, const Sk4px& aa) {
        //  = (s + d(1-sa))aa + d(1-aa)
        //  = s*aa + d(1-sa*aa)
        //   ~~~>
        //  = s*aa + d(1-aa)
        return s.approxMulDiv255(aa) + d.approxMulDiv255(aa.inv());
    };
    while (h --> 0) {
        Sk4px::MapDstAlpha(w, dst, mask, fn);
        dst  +=  dstRB / sizeof(*dst);
        mask += maskRB / sizeof(*mask);
    }
}

// Same as _opaque, but assumes color == SK_ColorBLACK, a very common and even simpler case.
static void blit_mask_d32_a8_black(SkPMColor* dst, size_t dstRB,
                                   const SkAlpha* mask, size_t maskRB,
                                   int w, int h) {
    auto fn = [](const Sk4px& d, const Sk4px& aa) {
        //   = (s + d(1-sa))aa + d(1-aa)
        //   = s*aa + d(1-sa*aa)
        //   ~~~>
        // a = 1*aa + d(1-1*aa) = aa + d(1-aa)
        // c = 0*aa + d(1-1*aa) =      d(1-aa)
        return aa.zeroColors() + d.approxMulDiv255(aa.inv());
    };
    while (h --> 0) {
        Sk4px::MapDstAlpha(w, dst, mask, fn);
        dst  +=  dstRB / sizeof(*dst);
        mask += maskRB / sizeof(*mask);
    }
}

static void blit_mask_d32_a8(SkPMColor* dst, size_t dstRB,
                             const SkAlpha* mask, size_t maskRB,
                             SkColor color, int w, int h) {
    if (color == SK_ColorBLACK) {
        blit_mask_d32_a8_black(dst, dstRB, mask, maskRB, w, h);
    } else if (SkColorGetA(color) == 0xFF) {
        blit_mask_d32_a8_opaque(dst, dstRB, mask, maskRB, color, w, h);
    } else {
        blit_mask_d32_a8_general(dst, dstRB, mask, maskRB, color, w, h);
    }
}

}  // SK_OPTS_NS

#endif//SkBlitMask_opts_DEFINED
