/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderFns_DEFINED
#define SkImageEncoderFns_DEFINED

/**
 * Functions to transform scanlines between packed-pixel formats.
 */

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorData.h"
#include "SkICC.h"
#include "SkOpts.h"
#include "SkPreConfig.h"
#include "SkRasterPipeline.h"
#include "SkUnPreMultiply.h"
#include "SkUnPreMultiplyPriv.h"
#include "../jumper/SkJumper.h"

/**
 * Function template for transforming scanlines.
 * Transform 'width' pixels from 'src' buffer into 'dst' buffer,
 * repacking color channel data as appropriate for the given transformation.
 * 'bpp' is bytes per pixel in the 'src' buffer.
 */
typedef void (*transform_scanline_proc)(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                        int width, int bpp, const SkPMColor* colors);

/**
 * Identity transformation: just copy bytes from src to dst.
 */
static inline void transform_scanline_memcpy(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                             int width, int bpp, const SkPMColor*) {
    memcpy(dst, src, width * bpp);
}

static inline void transform_scanline_index8_opaque(char* SK_RESTRICT dst,
                                                    const char* SK_RESTRICT src, int width, int,
                                                    const SkPMColor* colors) {
    for (int i = 0; i < width; i++) {
        const uint32_t c = colors[(uint8_t)*src++];
        dst[0] = SkGetPackedR32(c);
        dst[1] = SkGetPackedG32(c);
        dst[2] = SkGetPackedB32(c);
        dst += 3;
    }
}

static inline void transform_scanline_index8_unpremul(char* SK_RESTRICT dst,
                                                      const char* SK_RESTRICT src, int width, int,
                                                      const SkPMColor* colors) {
    uint32_t* SK_RESTRICT dst32 = (uint32_t*) dst;
    for (int i = 0; i < width; i++) {
        // This function swizzles R and B on platforms where SkPMColor is BGRA.  This is
        // exactly what we want.
        dst32[i] = SkSwizzle_RGBA_to_PMColor(colors[(uint8_t)*src++]);
    }
}

static inline void transform_scanline_gray(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                           int width, int, const SkPMColor* colors) {
    for (int i = 0; i < width; i++) {
        const uint8_t g = (uint8_t) *src++;
        dst[0] = g;
        dst[1] = g;
        dst[2] = g;
        dst += 3;
    }
}

/**
 * Transform from kRGB_565_Config to 3-bytes-per-pixel RGB.
 * Alpha channel data is not present in kRGB_565_Config format, so there is no
 * alpha channel data to preserve.
 */
static inline void transform_scanline_565(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                          int width, int, const SkPMColor*) {
    const uint16_t* srcP = (const uint16_t*)src;
    for (int i = 0; i < width; i++) {
        unsigned c = *srcP++;
        *dst++ = SkPacked16ToR32(c);
        *dst++ = SkPacked16ToG32(c);
        *dst++ = SkPacked16ToB32(c);
    }
}

/**
 * Transform from kAlpha_8_Config to 2-bytes-per-pixel GrayAlpha.
 */
static inline void transform_scanline_A8_to_GrayAlpha(char* SK_RESTRICT dst,
                                                      const char* SK_RESTRICT src,
                                                      int width, int, const SkPMColor*) {
    for (int i = 0; i < width; i++) {
        *dst++ = 0;         // gray (ignored)
        *dst++ = *src++;    // alpha
    }
}

/**
 * Transform from kRGBA_8888_SkColorType to 3-bytes-per-pixel RGB.
 * Alpha channel data is abandoned.
 */
static inline void transform_scanline_RGBX(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                           int width, int, const SkPMColor*) {
    const uint32_t* srcP = (const SkPMColor*)src;
    for (int i = 0; i < width; i++) {
        uint32_t c = *srcP++;
        *dst++ = (c >>  0) & 0xFF;
        *dst++ = (c >>  8) & 0xFF;
        *dst++ = (c >> 16) & 0xFF;
    }
}

/**
 * Transform from kBGRA_8888_SkColorType to 3-bytes-per-pixel RGB.
 * Alpha channel data is abandoned.
 */
static inline void transform_scanline_BGRX(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                           int width, int, const SkPMColor*) {
    const uint32_t* srcP = (const SkPMColor*)src;
    for (int i = 0; i < width; i++) {
        uint32_t c = *srcP++;
        *dst++ = (c >> 16) & 0xFF;
        *dst++ = (c >>  8) & 0xFF;
        *dst++ = (c >>  0) & 0xFF;
    }
}

/**
 * Transform from kARGB_4444_Config to 3-bytes-per-pixel RGB.
 * Alpha channel data, if any, is abandoned.
 */
static inline void transform_scanline_444(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                          int width, int, const SkPMColor*) {
    const SkPMColor16* srcP = (const SkPMColor16*)src;
    for (int i = 0; i < width; i++) {
        SkPMColor16 c = *srcP++;
        *dst++ = SkPacked4444ToR32(c);
        *dst++ = SkPacked4444ToG32(c);
        *dst++ = SkPacked4444ToB32(c);
    }
}

/**
 * Transform from legacy kPremul, kRGBA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static inline void transform_scanline_rgbA(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                           int width, int, const SkPMColor*) {
    SkUnpremultiplyRow<false>((uint32_t*) dst, (const uint32_t*) src, width);
}

/**
 * Transform from legacy kPremul, kBGRA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static inline void transform_scanline_bgrA(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                           int width, int, const SkPMColor*) {
    SkUnpremultiplyRow<true>((uint32_t*) dst, (const uint32_t*) src, width);
}

template <bool kIsRGBA>
static inline void transform_scanline_unpremultiply_sRGB(void* dst, const void* src, int width) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    if (kIsRGBA) {
        p.append(SkRasterPipeline::load_8888, &src_ctx);
    } else {
        p.append(SkRasterPipeline::load_bgra, &src_ctx);
    }

    p.append(SkRasterPipeline::from_srgb);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_8888, &dst_ctx);
    p.run(0,0, width,1);
}

/**
 * Premultiply RGBA to rgbA.
 */
static inline void transform_scanline_to_premul_legacy(char* SK_RESTRICT dst,
                                                       const char* SK_RESTRICT src,
                                                       int width, int, const SkPMColor*) {
    SkOpts::RGBA_to_rgbA((uint32_t*)dst, (const uint32_t*)src, width);
}

/**
 * Premultiply RGBA to rgbA linearly.
 */
static inline void transform_scanline_to_premul_linear(char* SK_RESTRICT dst,
                                                       const char* SK_RESTRICT src,
                                                       int width, int, const SkPMColor*) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_8888, &src_ctx);
    p.append(SkRasterPipeline::from_srgb);
    p.append(SkRasterPipeline::premul);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_8888, &dst_ctx);
    p.run(0,0, width,1);
}

/**
 * Transform from kPremul, kRGBA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static inline void transform_scanline_srgbA(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                            int width, int, const SkPMColor*) {
    transform_scanline_unpremultiply_sRGB<true>(dst, src, width);
}

/**
 * Transform from kPremul, kBGRA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static inline void transform_scanline_sbgrA(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                            int width, int, const SkPMColor*) {
    transform_scanline_unpremultiply_sRGB<false>(dst, src, width);
}

/**
 * Transform from kUnpremul, kBGRA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static inline void transform_scanline_BGRA(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                           int width, int, const SkPMColor*) {
    const uint32_t* srcP = (const SkPMColor*)src;
    for (int i = 0; i < width; i++) {
        uint32_t c = *srcP++;
        *dst++ = (c >> 16) & 0xFF;
        *dst++ = (c >>  8) & 0xFF;
        *dst++ = (c >>  0) & 0xFF;
        *dst++ = (c >> 24) & 0xFF;
    }
}

/**
 * Transform from kARGB_8888_Config to 4-bytes-per-pixel RGBA,
 * with scaling of RGB based on alpha channel.
 */
static inline void transform_scanline_4444(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                           int width, int, const SkPMColor*) {
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

// 888x is opaque RGB in four bytes, with 8 junk bits.  We convert that to 3 byte RGB.
static inline void transform_scanline_888x(char* dst, const char* src,
                                           int width, int, const SkPMColor*) {
    while (width --> 0) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst += 3;
        src += 4;
    }
}

// 101010x is opaque RGB in four bytes, with 2 bits junk.  We convert to 6 byte RGB (big endian).
static inline void transform_scanline_101010x(char* dst, const char* src,
                                              int width, int, const SkPMColor*) {
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

static inline void transform_scanline_1010102(char* dst, const char* src,
                                              int width, int, const SkPMColor*) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_1010102, &src_ctx);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

static inline void transform_scanline_1010102_premul(char* dst, const char* src,
                                                     int width, int, const SkPMColor*) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_1010102, &src_ctx);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

/**
 * Transform from kRGBA_F16 to 8-bytes-per-pixel RGBA.
 */
static inline void transform_scanline_F16(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                          int width, int, const SkPMColor*) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

/**
 * Transform from kPremul, kRGBA_F16 to 8-bytes-per-pixel RGBA.
 */
static inline void transform_scanline_F16_premul(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                                 int width, int, const SkPMColor*) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_u16_be, &dst_ctx);
    p.run(0,0, width,1);
}

/**
 * Transform from kRGBA_F16 to 4-bytes-per-pixel RGBA.
 */
static inline void transform_scanline_F16_to_8888(char* SK_RESTRICT dst,
                                                  const char* SK_RESTRICT src, int width, int,
                                                  const SkPMColor*) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_8888, &dst_ctx);
    p.run(0,0, width,1);
}

/**
 * Transform from kPremul, kRGBA_F16 to 4-bytes-per-pixel RGBA.
 */
static inline void transform_scanline_F16_premul_to_8888(char* SK_RESTRICT dst,
                                                         const char* SK_RESTRICT src, int width,
                                                         int, const SkPMColor*) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_8888, &dst_ctx);
    p.run(0,0, width,1);
}

/**
 * Transform from kUnpremul, kRGBA_F16 to premultiplied rgbA 8888.
 */
static inline void transform_scanline_F16_to_premul_8888(char* SK_RESTRICT dst,
        const char* SK_RESTRICT src, int width, int, const SkPMColor*) {
    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &src_ctx);
    p.append(SkRasterPipeline::clamp_0);  // F16 values may be out of [0,1] range, so clamp.
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::premul);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_8888, &dst_ctx);
    p.run(0,0, width,1);
}

static inline sk_sp<SkData> icc_from_color_space(const SkImageInfo& info) {
    SkColorSpace* cs = info.colorSpace();
    if (!cs) {
        return nullptr;
    }

    sk_sp<SkColorSpace> owned;
    if (kRGBA_F16_SkColorType == info.colorType()) {
        owned = cs->makeSRGBGamma();
        cs = owned.get();
    }

    SkColorSpaceTransferFn fn;
    SkMatrix44 toXYZD50(SkMatrix44::kUninitialized_Constructor);
    if (cs->isNumericalTransferFn(&fn) && cs->toXYZD50(&toXYZD50)) {
        return SkICC::WriteToICC(fn, toXYZD50);
    }

    // TODO: Should we support writing ICC profiles for additional color spaces?
    return nullptr;
}

#endif  // SkImageEncoderFns_DEFINED
