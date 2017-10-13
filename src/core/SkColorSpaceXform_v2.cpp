/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform_v2.h"
#include "SkRasterPipeline.h"

bool SkColorSpaceXform_v2::Apply(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                                 SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT,
                                 void* dst,       size_t dstRB,
                                 const void* src, size_t srcRB,
                                 int w, int h) {
    // TODO: a smarter implementation could streamline a couple things:
    //  1) no need to ref and unref the color spaces;
    //  2) use SkRasterPipeline::run() instead of compile().
    return SkColorSpaceXform_v2(dstCS, dstCT, dstAT,
                                srcCS, srcCT, srcAT)(dst,dstRB, src,srcRB, w,h);
}

SkColorSpaceXform_v2::SkColorSpaceXform_v2(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                                           SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT)
    : fDstCS(sk_ref_sp(dstCS))
    , fSrcCS(sk_ref_sp(srcCS))
{
    SkRasterPipeline_<256> p;

    // ...

    fRun = p.compile();
}

bool SkColorSpaceXform_v2::operator()(void*       dst, size_t dstRB,
                                      const void* src, size_t srcRB,
                                      int w, int h) {
    if (fRun) {
        fDst.ptr      = dst;
        fDst.rowBytes = dstRB;

        fSrc.ptr      = src;
        fSrc.rowBytes = srcRB;

        fRun(0,0, SkToSizeT(w),SkToSizeT(h));
        return true;
    }
    return false;
}
