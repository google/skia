/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform_v2.h"
#include "SkRasterPipeline.h"
#include "../jumper/SkJumper.h"

using AlphaType = SkColorSpaceXform_v2::AlphaType;
using ColorType = SkColorSpaceXform_v2::ColorType;

static bool build_pipeline(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                           SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT,
                           SkJumper_MemoryCtx* dstCtx,
                           SkJumper_MemoryCtx* srcCtx,
                           SkRasterPipeline* p) {
    // 1. Load src data.
    SkRasterPipeline::StockStage load_src;
    switch (srcCT) {
        case ColorType::Alpha_8:   load_src = SkRasterPipeline::load_a8;         break;
        case ColorType::Gray_8:    load_src = SkRasterPipeline::load_g8;         break;
        case ColorType::BGR_565:   load_src = SkRasterPipeline::load_565;        break;
        case ColorType::ABGR_4444: load_src = SkRasterPipeline::load_4444;       break;
        case ColorType::RGBA_8888: load_src = SkRasterPipeline::load_8888;       break;
        case ColorType::BGRA_8888: load_src = SkRasterPipeline::load_bgra;       break;
        case ColorType::RGBA_F16:  load_src = SkRasterPipeline::load_f16;        break;
        case ColorType::RGBA_F32:  load_src = SkRasterPipeline::load_f32;        break;
        case ColorType::RGB_BE16:  load_src = SkRasterPipeline::load_rgb_u16_be; break;
        case ColorType::RGBA_BE16: load_src = SkRasterPipeline::load_u16_be;     break;
        case ColorType::CMYK_8888: load_src = SkRasterPipeline::load_8888;       break;
    }
    p->append(load_src, srcCtx);

    if (srcAT == AlphaType::Opaque) {
        // A roundabout way to say, set alpha to 1.0.
        p->append(SkRasterPipeline::move_src_dst);
        p->append(SkRasterPipeline::black_color);
        p->append(SkRasterPipeline::plus_);
    }

    return true;
}

static int stride(size_t rowBytes, ColorType ct) {
    size_t stride = 0;
    switch (ct) {
        case ColorType::Alpha_8:
        case ColorType::Gray_8:    stride = rowBytes/ 1; break;

        case ColorType::BGR_565:
        case ColorType::ABGR_4444: stride = rowBytes/ 2; break;

        case ColorType::RGBA_8888:
        case ColorType::BGRA_8888:
        case ColorType::CMYK_8888: stride = rowBytes/ 4; break;

        case ColorType::RGB_BE16:  stride = rowBytes/ 6; break;

        case ColorType::RGBA_F16:
        case ColorType::RGBA_BE16: stride = rowBytes/ 8; break;

        case ColorType::RGBA_F32:  stride = rowBytes/16; break;
    }
    return SkToInt(stride);
}

SkColorSpaceXform_v2::SkColorSpaceXform_v2(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                                           SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT)
    : fDstCS(sk_ref_sp(dstCS))
    , fSrcCS(sk_ref_sp(srcCS))
    , fDstCT(dstCT)
    , fSrcCT(srcCT)
{
    SkRasterPipeline_<256> p;
    if (build_pipeline(dstCS, dstCT, dstAT,
                       srcCS, srcCT, srcAT,
                       reinterpret_cast<SkJumper_MemoryCtx*>(&fDstCtx),
                       reinterpret_cast<SkJumper_MemoryCtx*>(&fSrcCtx),
                       &p)) {
        fRun = p.compile();
    }
}

bool SkColorSpaceXform_v2::operator()(void*       dst, size_t dstRB,
                                      const void* src, size_t srcRB,
                                      int w, int h) {
    if (fRun) {
        fDstCtx.ptr    = dst;
        fDstCtx.stride = stride(dstRB, fDstCT);

        fSrcCtx.ptr    = src;
        fSrcCtx.stride = stride(srcRB, fSrcCT);

        fRun(0,0, SkToSizeT(w),SkToSizeT(h));
        return true;
    }
    return false;
}

bool SkColorSpaceXform_v2::Apply(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                                 SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT,
                                 void* dst,       size_t dstRB,
                                 const void* src, size_t srcRB,
                                 int w, int h) {

    SkRasterPipeline_<256> p;
    SkJumper_MemoryCtx dstCtx = { (void*)dst, stride(dstRB, dstCT) },
                       srcCtx = { (void*)src, stride(srcRB, srcCT) };
    if (build_pipeline(dstCS, dstCT, dstAT,
                       srcCS, srcCT, srcAT,
                       &dstCtx,
                       &srcCtx,
                       &p)) {
        p.run(0,0, SkToSizeT(w), SkToSizeT(h));
        return true;
    }
    return false;
}
