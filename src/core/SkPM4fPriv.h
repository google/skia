/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4fPriv_DEFINED
#define SkPM4fPriv_DEFINED

#include "../jumper/SkJumper.h"
#include "SkArenaAlloc.h"
#include "SkColorData.h"
#include "SkColorSpace.h"
#include "SkColorSpaceXformSteps.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"

static inline Sk4f set_alpha(const Sk4f& px, float alpha) {
    return { px[0], px[1], px[2], alpha };
}

static inline float get_alpha(const Sk4f& px) {
    return px[3];
}


static inline Sk4f Sk4f_fromL32(uint32_t px) {
    return SkNx_cast<float>(Sk4b::Load(&px)) * (1/255.0f);
}

static inline Sk4f Sk4f_fromS32(uint32_t px) {
    return { sk_linear_from_srgb[(px >>  0) & 0xff],
             sk_linear_from_srgb[(px >>  8) & 0xff],
             sk_linear_from_srgb[(px >> 16) & 0xff],
                    (1/255.0f) * (px >> 24)          };
}

static inline uint32_t Sk4f_toL32(const Sk4f& px) {
    uint32_t l32;
    SkNx_cast<uint8_t>(Sk4f_round(px * 255.0f)).store(&l32);
    return l32;
}

static inline uint32_t Sk4f_toS32(const Sk4f& px) {
    Sk4i  rgb = sk_linear_to_srgb(px),
         srgb = { rgb[0], rgb[1], rgb[2], (int)(255.0f * px[3] + 0.5f) };

    uint32_t s32;
    SkNx_cast<uint8_t>(srgb).store(&s32);
    return s32;
}


// SkColor handling:
//   SkColor has an ordering of (b, g, r, a) if cast to an Sk4f, so the code swizzles r and b to
// produce the needed (r, g, b, a) ordering.
static inline Sk4f Sk4f_from_SkColor(SkColor color) {
    return swizzle_rb(Sk4f_fromS32(color));
}

static inline void assert_unit(float x) {
    SkASSERT(0 <= x && x <= 1);
}

static inline float exact_srgb_to_linear(float srgb) {
    assert_unit(srgb);
    float linear;
    if (srgb <= 0.04045) {
        linear = srgb / 12.92f;
    } else {
        linear = powf((srgb + 0.055f) / 1.055f, 2.4f);
    }
    assert_unit(linear);
    return linear;
}

static inline void transform_colorspace(SkRasterPipeline* p,
                                        SkArenaAlloc* alloc,
                                        SkColorSpace* src,
                                        SkColorSpace* dst,
                                        SkAlphaType srcAT) {
    sk_sp<SkColorSpace> sRGB;
    if (!src || !dst) {
        sRGB = SkColorSpace::MakeSRGB();
        if (!src) { src = sRGB.get(); }
        if (!dst) { dst = sRGB.get(); }
    }

    SkColorSpaceXformSteps steps(src, srcAT, dst);

    if (steps.unpremul) { p->append(SkRasterPipeline::unpremul); }
    if (steps.linearize) {
        if (src->gammaCloseToSRGB()) {
            p->append(SkRasterPipeline::from_srgb);
        } else {
            auto tf = alloc->make<SkColorSpaceTransferFn>(steps.srcTF);
            p->append(SkRasterPipeline::parametric_r, tf);
            p->append(SkRasterPipeline::parametric_g, tf);
            p->append(SkRasterPipeline::parametric_b, tf);
        }
    }
    if (steps.gamut_transform) {
        auto m = memcpy(alloc->makeArrayDefault<float>(9), steps.src_to_dst_matrix, 36);
        p->append(SkRasterPipeline::matrix_3x3, m);
    }
    if (steps.encode) {
        if (dst->gammaCloseToSRGB()) {
            p->append(SkRasterPipeline::to_srgb);
        } else {
            auto tf = alloc->make<SkColorSpaceTransferFn>(steps.dstTFInv);
            p->append(SkRasterPipeline::parametric_r, tf);
            p->append(SkRasterPipeline::parametric_g, tf);
            p->append(SkRasterPipeline::parametric_b, tf);
        }
    }
    if (steps.premul) { p->append(SkRasterPipeline::premul); }
}

// I'm being a bit careful here to avoid older methods that assume SkColor4f is linear.

static inline SkColor4f to_colorspace(SkColor4f color, SkColorSpace* src, SkColorSpace* dst) {
    SkColor4f result;

    SkJumper_MemoryCtx src_ctx = { &color , 0 },
                       dst_ctx = { &result, 0 };

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);
    p.append(SkRasterPipeline::load_f32, &src_ctx);
    transform_colorspace(&p,&alloc,
                         src,dst,kOpaque_SkAlphaType/*a lie, but maps unpremul to unpremul*/);
    p.append(SkRasterPipeline::store_f32, &dst_ctx);
    p.run(0,0,1,1);

    return result;
}

static inline SkColor4f SkColor4f_from_SkColor(SkColor color, SkColorSpace* dst) {
    SkColor4f result;

    SkJumper_MemoryCtx src_ctx = { &color , 0 },
                       dst_ctx = { &result, 0 };

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);
    p.append(SkRasterPipeline::load_bgra, &src_ctx);
    transform_colorspace(&p,&alloc,
                         nullptr,dst,kOpaque_SkAlphaType/*a lie, but maps unpremul to unpremul*/);
    p.append(SkRasterPipeline::store_f32, &dst_ctx);
    p.run(0,0,1,1);

    return result;
}

static inline SkPM4f SkPM4f_from_SkColor(SkColor color, SkColorSpace* dst) {
    SkPM4f result;

    SkJumper_MemoryCtx src_ctx = { &color , 0 },
                       dst_ctx = { &result, 0 };

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);
    p.append(SkRasterPipeline::load_bgra, &src_ctx);
    transform_colorspace(&p,&alloc,
                         nullptr,dst,kUnpremul_SkAlphaType);
    p.append(SkRasterPipeline::store_f32, &dst_ctx);
    p.run(0,0,1,1);

    return result;
}
#endif
