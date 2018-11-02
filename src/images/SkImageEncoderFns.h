/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderFns_DEFINED
#define SkImageEncoderFns_DEFINED

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorData.h"
#include "SkICC.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"
#include "SkTypes.h"
#include "SkUnPreMultiply.h"
#include "SkUnPreMultiplyPriv.h"

typedef void (*transform_scanline_proc)(char* dst, const char* src, int width, int bpp);

static inline void transform_scanline_memcpy(char* dst, const char* src, int width, int bpp) {
    memcpy(dst, src, width * bpp);
}

static inline void transform_scanline_gray(char* dst, const char* src, int width, int) {
    for (int i = 0; i < width; i++) {
        const uint8_t g = (uint8_t) *src++;
        dst[0] = g;
        dst[1] = g;
        dst[2] = g;
        dst += 3;
    }
}

static inline void transform_scanline_565(char* dst, const char* src, int width, int) {
    const uint16_t* srcP = (const uint16_t*)src;
    for (int i = 0; i < width; i++) {
        unsigned c = *srcP++;
        *dst++ = SkPacked16ToR32(c);
        *dst++ = SkPacked16ToG32(c);
        *dst++ = SkPacked16ToB32(c);
    }
}

static inline void transform_scanline_A8_to_GrayAlpha(char* dst, const char* src, int width, int) {
    for (int i = 0; i < width; i++) {
        *dst++ = 0;         // gray (ignored)
        *dst++ = *src++;    // alpha
    }
}

static inline void transform_scanline_RGBX(char* dst, const char* src, int width, int) {
    const uint32_t* srcP = (const SkPMColor*)src;
    for (int i = 0; i < width; i++) {
        uint32_t c = *srcP++;
        *dst++ = (c >>  0) & 0xFF;
        *dst++ = (c >>  8) & 0xFF;
        *dst++ = (c >> 16) & 0xFF;
    }
}

static inline void transform_scanline_BGRX(char* dst, const char* src, int width, int) {
    const uint32_t* srcP = (const SkPMColor*)src;
    for (int i = 0; i < width; i++) {
        uint32_t c = *srcP++;
        *dst++ = (c >> 16) & 0xFF;
        *dst++ = (c >>  8) & 0xFF;
        *dst++ = (c >>  0) & 0xFF;
    }
}

static inline void transform_scanline_444(char* dst, const char* src, int width, int) {
    const SkPMColor16* srcP = (const SkPMColor16*)src;
    for (int i = 0; i < width; i++) {
        SkPMColor16 c = *srcP++;
        *dst++ = SkPacked4444ToR32(c);
        *dst++ = SkPacked4444ToG32(c);
        *dst++ = SkPacked4444ToB32(c);
    }
}

static inline void transform_scanline_rgbA(char* dst, const char* src, int width, int) {
    SkUnpremultiplyRow<false>((uint32_t*) dst, (const uint32_t*) src, width);
}

static inline void transform_scanline_bgrA(char* dst, const char* src, int width, int) {
    SkUnpremultiplyRow<true>((uint32_t*) dst, (const uint32_t*) src, width);
}

static inline void transform_scanline_to_premul_legacy(char* dst, const char* src, int width, int) {
    SkOpts::RGBA_to_rgbA((uint32_t*)dst, (const uint32_t*)src, width);
}

static inline void transform_scanline_BGRA(char* dst, const char* src, int width, int) {
    const uint32_t* srcP = (const SkPMColor*)src;
    for (int i = 0; i < width; i++) {
        uint32_t c = *srcP++;
        *dst++ = (c >> 16) & 0xFF;
        *dst++ = (c >>  8) & 0xFF;
        *dst++ = (c >>  0) & 0xFF;
        *dst++ = (c >> 24) & 0xFF;
    }
}

static inline void transform_scanline_4444(char* dst, const char* src, int width, int) {
    const SkPMColor16* srcP = (const SkPMColor16*)src;
    const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();

    for (int i = 0; i < width; i++) {
        SkPMColor16 c = *srcP++;
        unsigned a = SkPacked4444ToA32(c);
        unsigned r = SkPacked4444ToR32(c);
        unsigned g = SkPacked4444ToG32(c);
        unsigned b = SkPacked4444ToB32(c);

        if (0 != a && 255 != a) {
            SkUnPreMultiply::Scale scale = table[a];
            r = SkUnPreMultiply::ApplyScale(scale, r);
            g = SkUnPreMultiply::ApplyScale(scale, g);
            b = SkUnPreMultiply::ApplyScale(scale, b);
        }
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
        *dst++ = a;
    }
}

static inline void transform_scanline_888x(char* dst, const char* src, int width, int) {
    while (width --> 0) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst += 3;
        src += 4;
    }
}

static inline void transform_scanline_101010x(char* dst, const char* src, int width, int) {
    auto d = (      uint16_t*)dst;
    auto s = (const uint32_t*)src;
    while (width --> 0) {
        uint32_t r = (*s >>  0) & 1023,
                 g = (*s >> 10) & 1023,
                 b = (*s >> 20) & 1023;

        // Scale 10-bit unorms to 16-bit by replicating the most significant bits.
        r = (r << 6) | (r >> 4);
        g = (g << 6) | (g >> 4);
        b = (b << 6) | (b >> 4);

        // Store big-endian.
        d[0] = (r >> 8) | (r << 8);
        d[1] = (g >> 8) | (g << 8);
        d[2] = (b >> 8) | (b << 8);

        d += 3;  // 3 channels
        s += 1;  // 1 whole pixel
    }
}

static inline void transform_scanline_1010102(char* dst, const char* src, int width, int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_1010102, &src_ctx);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_1010102_premul(char* dst, const char* src, int width, int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_1010102, &src_ctx);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_F16(char* dst, const char* src, int width, int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_F16_premul(char* dst, const char* src, int width, int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_F16_to_8888(char* dst, const char* src, int width, int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::store_8888, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_F16_premul_to_8888(char* dst,
                                                         const char* src,
                                                         int width,
                                                         int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::store_8888, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_F16_to_premul_8888(char* dst,
                                                         const char* src,
                                                         int width,
                                                         int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::premul);
    p.append(SkRasterPipeline::store_8888, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_F32(char* dst, const char* src, int width, int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f32, &src_ctx);
    p.append(SkRasterPipeline::clamp_0);  // F32 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_F32_premul(char* dst, const char* src, int width, int) {
    SkRasterPipeline_MemoryCtx src_ctx = { (void*)src, 0 },
                               dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f32, &src_ctx);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::clamp_0);  // F32 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

static inline sk_sp<SkData> icc_from_color_space(const SkImageInfo& info) {
    SkColorSpace* cs = info.colorSpace();
    if (!cs) {
        return nullptr;
    }

    SkColorSpaceTransferFn fn;
    SkMatrix44 toXYZD50;
    if (cs->isNumericalTransferFn(&fn) && cs->toXYZD50(&toXYZD50)) {
        return SkICC::WriteToICC(fn, toXYZD50);
    }
    return nullptr;
}

#endif  // SkImageEncoderFns_DEFINED
