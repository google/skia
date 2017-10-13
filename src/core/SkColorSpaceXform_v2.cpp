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

// Returns a trasfer function for the given color channel,
//   - either by returning a table pointer and filling in table_size,
//   - or by returning nullptr and filling parametric.
static const float* table_or_parametric(const SkGammas* gammas,
                                        int channel,
                                        int* table_size,
                                        SkColorSpaceTransferFn* parametric) {
    SkASSERT(channel < gammas->channels());

    switch (gammas->type(channel)) {
        case kNone_Type:

        case kNamed_Type:

        case kValue_Type:

        case kTable_Type:

        case kParam_Type:

    }
}

static bool build_pipeline(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                           SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT,
                           SkJumper_MemoryCtx* dstCtx,
                           SkJumper_MemoryCtx* srcCtx,
                           SkRasterPipeline* p, float gamutTransform[12]) {
    // 1. Load source data.
    SkRasterPipeline::StockStage load_src;
    switch (srcCT) {
        case ColorType::Alpha_8:           load_src = SkRasterPipeline::load_a8;         break;
        case ColorType::Gray_8:            load_src = SkRasterPipeline::load_g8;         break;
        case ColorType::BGR_565:           load_src = SkRasterPipeline::load_565;        break;
        case ColorType::ABGR_4444:         load_src = SkRasterPipeline::load_4444;       break;
        case ColorType::RGBA_8888:         load_src = SkRasterPipeline::load_8888;       break;
        case ColorType::BGRA_8888:         load_src = SkRasterPipeline::load_bgra;       break;
        case ColorType::RGBA_F16:          load_src = SkRasterPipeline::load_f16;        break;
        case ColorType::RGBA_F32:          load_src = SkRasterPipeline::load_f32;        break;
        case ColorType::Clamped_RGBA_F16:  load_src = SkRasterPipeline::load_f16;        break;
        case ColorType::Clamped_RGBA_F32:  load_src = SkRasterPipeline::load_f32;        break;
        case ColorType::RGB_BE16:          load_src = SkRasterPipeline::load_rgb_u16_be; break;
        case ColorType::RGBA_BE16:         load_src = SkRasterPipeline::load_u16_be;     break;
        case ColorType::CMYK_8888:         load_src = SkRasterPipeline::load_8888;       break;
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
            // Our colors are a,tf(r*a),..., so nothing to do before linearization,
            // except remember that the values we'll be gamut transforming are premultiplied.
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
    if (!toXYZ) {  // TODO: support A2B sources.
        return false;
    }
    auto srcXYZ = (SkColorSpace_XYZ*)srcCS;
    auto srcGammas = srcXYZ->gammas();

    switch (srcXYZ->gammaNamed()) {
        // In these first three cases, all three transfer functions are the same.
        case kLinear_SkGammaNamed:
            break;
        case kSRGB_SkGammaNamed:
            p->append_from_srgb(colorsArePremul ? kPremul_SkAlphaType : kUnpremul_SkAlphaType);
            break;
        case k2Dot2Curve_SkGammaNamed:
            srcTF[0].fG = 2.2f;
            p->append(SkRasterPipeline::gamma, &srcTF[0].fG);
            break;

        case kNonStandard_SkGammaNamed:
            // The transfer functions are different per-channel, or are tables, or both.
            if (srcGammas->channels() != 3) {
                return false;
            }

    }

    // 4. Transform to destination gamut.
    const SkMatrix44* fromXYZ = as_CSB(dstCS)->fromXYZD50();
    if (!fromXYZ) {
        return false;  // We only support XYZ-type destinations.
    }
    SkMatrix44 srcToDst(*fromXYZ, *toXYZ);
    gamutTransform[ 0] = srcToDst.get(0,0);
    gamutTransform[ 1] = srcToDst.get(1,0);
    gamutTransform[ 2] = srcToDst.get(2,0);
    gamutTransform[ 3] = srcToDst.get(0,1);
    gamutTransform[ 4] = srcToDst.get(1,1);
    gamutTransform[ 5] = srcToDst.get(2,1);
    gamutTransform[ 6] = srcToDst.get(0,2);
    gamutTransform[ 7] = srcToDst.get(1,2);
    gamutTransform[ 8] = srcToDst.get(2,2);
    gamutTransform[ 9] = srcToDst.get(0,3);
    gamutTransform[10] = srcToDst.get(1,3);
    gamutTransform[11] = srcToDst.get(2,3);

    // 5. Apply destination transfer function and any premultiplication.
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


    // 6. Store to destination.
    SkRasterPipeline::StockStage store_dst;
    switch (dstCT) {
        case ColorType::Alpha_8:          store_dst = SkRasterPipeline::store_a8;   break;
        case ColorType::Gray_8:           p->append(SkRasterPipeline::luminance_to_alpha);
                                          store_dst = SkRasterPipeline::store_a8;   break;
        case ColorType::BGR_565:          store_dst = SkRasterPipeline::store_565;  break;
        case ColorType::ABGR_4444:        store_dst = SkRasterPipeline::store_4444; break;
        case ColorType::RGBA_8888:        store_dst = SkRasterPipeline::store_8888; break;
        case ColorType::BGRA_8888:        store_dst = SkRasterPipeline::store_bgra; break;
        case ColorType::RGBA_F16:         store_dst = SkRasterPipeline::store_f16;  break;
        case ColorType::RGBA_F32:         store_dst = SkRasterPipeline::store_f32;  break;
        case ColorType::Clamped_RGBA_F16: store_dst = SkRasterPipeline::store_f16;  break;
        case ColorType::Clamped_RGBA_F32: store_dst = SkRasterPipeline::store_f32;  break;
        case ColorType::RGB_BE16:  return false;
        case ColorType::RGBA_BE16: return false;
        case ColorType::CMYK_8888: return false;
    }
    if (dstCT != ColorType::RGBA_F16 && dstCT != ColorType::RGBA_F32) {
        p->append(SkRasterPipeline::clamp_0);
        p->append(SkRasterPipeline::clamp_1);
    }
    p->append(store_dst, dstCtx);

    return true;
}

static int stride(size_t rowBytes, ColorType ct) {
    size_t stride = 0;
    switch (ct) {
        case ColorType::Alpha_8:
        case ColorType::Gray_8:            stride = rowBytes/ 1; break;

        case ColorType::BGR_565:
        case ColorType::ABGR_4444:         stride = rowBytes/ 2; break;

        case ColorType::RGBA_8888:
        case ColorType::BGRA_8888:
        case ColorType::CMYK_8888:         stride = rowBytes/ 4; break;

        case ColorType::RGB_BE16:          stride = rowBytes/ 6; break;

        case ColorType::RGBA_F16:
        case ColorType::RGBA_BE16:
        case ColorType::Clamped_RGBA_F16:  stride = rowBytes/ 8; break;

        case ColorType::RGBA_F32:
        case ColorType::Clamped_RGBA_F32:  stride = rowBytes/16; break;
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
                       &p,
                       &fGamutTransform)) {
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
    float gamutTransform[12];
    if (build_pipeline(dstCS, dstCT, dstAT,
                       srcCS, srcCT, srcAT,
                       &dstCtx,
                       &srcCtx,
                       &p,
                       &gamutTransform)) {
        p.run(0,0, SkToSizeT(w), SkToSizeT(h));
        return true;
    }
    return false;
}
