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

static void blit_mask_d32_a8(SkPMColor* dst, size_t dstRB,
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

}  // SK_OPTS_NS

#endif//SkBlitMask_opts_DEFINED
