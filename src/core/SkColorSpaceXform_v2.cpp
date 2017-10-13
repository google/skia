/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../jumper/SkJumper.h"
#include "SkColorSpaceXform_v2.h"
#include "SkColorSpace_A2B.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
#include "SkRasterPipeline.h"

using AlphaType = SkColorSpaceXform_v2::AlphaType;
using ColorType = SkColorSpaceXform_v2::ColorType;

// Append stages to set a = 1.0f, i.e. force opaque.
static void set_alpha_1(SkRasterPipeline* p) {
    // TODO: There's no single stage to set a = 1.0f, but maybe there should be.
    p->append(SkRasterPipeline::move_src_dst);
    p->append(SkRasterPipeline::black_color);
    p->append(SkRasterPipeline::plus_);
}

static bool build_pipeline(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                           SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT,
                           SkJumper_MemoryCtx* dstCtx,
                           SkJumper_MemoryCtx* srcCtx,
                           SkRasterPipeline* p) {
    // 1. Load source data.
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

    // 2. Handle anything else we may need to do based on source alpha type.
    bool colorsArePremul;
    switch (srcAT) {
        case AlphaType::Unpremul:
            // Our colors are a,tf(r),..., so nothing to do before linearization.
            colorsArePremul = false;
            break;

        case AlphaType::LinearPremul:
            // Our colors are a,tf(r*a),..., so nothing to do before linearization.
            colorsArePremul = true;
            break;

        case AlphaType::NonlinearPremul:
            // Our colors are a,tf(r)*a),..., so we need to unpremul before linearizing.
            p->append(SkRasterPipeline::unpremul);
            colorsArePremul = false;
            break;

        case AlphaType::Opaque:
            // Our colors are ?,tf(r),...,  so let's just make sure we set alpha == 1.0.
            set_alpha_1(p);
            colorsArePremul = false;
            break;
    }

    // 3. Linearize source colors.

    const SkMatrix44* toXYZ = as_CSB(srcCS)->toXYZD50();
    if (!toXYZ) {
        // TODO: A2B sources
        return false;
    }
    auto srcXYZ = (SkColorSpace_XYZ*)srcCS;

    // ...

    // ?. Handle anything else we may need to do based on destination alpha type.
    switch (dstAT) {
        case AlphaType::Unpremul:
            break;

        case AlphaType::LinearPremul:
            break;

        case AlphaType::NonlinearPremul:
            break;

        case AlphaType::Opaque:
            set_alpha_1(p);
            break;
    }

    // ?. Store to destination.
    SkRasterPipeline::StockStage store_dst;
    switch (dstCT) {
        case ColorType::Alpha_8:   store_dst = SkRasterPipeline::store_a8;   break;
        case ColorType::Gray_8:    p->append(SkRasterPipeline::luminance_to_alpha);  // TODO: better
                                   store_dst = SkRasterPipeline::store_a8;   break;
        case ColorType::BGR_565:   store_dst = SkRasterPipeline::store_565;  break;
        case ColorType::ABGR_4444: store_dst = SkRasterPipeline::store_4444; break;
        case ColorType::RGBA_8888: store_dst = SkRasterPipeline::store_8888; break;
        case ColorType::BGRA_8888: store_dst = SkRasterPipeline::store_bgra; break;
        case ColorType::RGBA_F16:  store_dst = SkRasterPipeline::store_f16;  break;
        case ColorType::RGBA_F32:  store_dst = SkRasterPipeline::store_f32;  break;
        case ColorType::RGB_BE16:  return false;
        case ColorType::RGBA_BE16: return false;
        case ColorType::CMYK_8888: return false;
    }
    p->append(store_dst, dstCtx);

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
