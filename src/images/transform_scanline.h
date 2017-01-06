/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * Functions to transform scanlines between packed-pixel formats.
 */

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkPreConfig.h"
#include "SkRasterPipeline.h"
#include "SkUnPreMultiply.h"

/**
 * Function template for transforming scanlines.
 * Transform 'width' pixels from 'src' buffer into 'dst' buffer,
 * repacking color channel data as appropriate for the given transformation.
 * 'bpp' is bytes per pixel in the 'src' buffer.
 */
typedef void (*transform_scanline_proc)(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                        int width, int bpp);

/**
 * Identity transformation: just copy bytes from src to dst.
 */
static void transform_scanline_memcpy(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                      int width, int bpp) {
    memcpy(dst, src, width * bpp);
}

/**
 * Transform from kRGB_565_Config to 3-bytes-per-pixel RGB.
 * Alpha channel data is not present in kRGB_565_Config format, so there is no
 * alpha channel data to preserve.
 */
static void transform_scanline_565(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                   int width, int) {
    const uint16_t* srcP = (const uint16_t*)src;
    for (int i = 0; i < width; i++) {
        unsigned c = *srcP++;
        *dst++ = SkPacked16ToR32(c);
        *dst++ = SkPacked16ToG32(c);
        *dst++ = SkPacked16ToB32(c);
    }
}

/**
 * Transform from kRGBA_8888_SkColorType to 3-bytes-per-pixel RGB.
 * Alpha channel data is abandoned.
 */
static void transform_scanline_RGBX(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                    int width, int) {
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
static void transform_scanline_BGRX(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                    int width, int) {
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
static void transform_scanline_444(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                   int width, int) {
    const SkPMColor16* srcP = (const SkPMColor16*)src;
    for (int i = 0; i < width; i++) {
        SkPMColor16 c = *srcP++;
        *dst++ = SkPacked4444ToR32(c);
        *dst++ = SkPacked4444ToG32(c);
        *dst++ = SkPacked4444ToB32(c);
    }
}

template <bool kIsRGBA>
static inline void transform_scanline_unpremultiply(char* SK_RESTRICT dst,
                                                    const char* SK_RESTRICT src, int width, int) {
    const uint32_t* srcP = (const SkPMColor*)src;
    const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();

    for (int i = 0; i < width; i++) {
        uint32_t c = *srcP++;
        unsigned r, g, b, a;
        if (kIsRGBA) {
            r = (c >>  0) & 0xFF;
            g = (c >>  8) & 0xFF;
            b = (c >> 16) & 0xFF;
            a = (c >> 24) & 0xFF;
        } else {
            r = (c >> 16) & 0xFF;
            g = (c >>  8) & 0xFF;
            b = (c >>  0) & 0xFF;
            a = (c >> 24) & 0xFF;
        }

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

/**
 * Transform from legacy kPremul, kRGBA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static void transform_scanline_rgbA(char* SK_RESTRICT dst, const char* SK_RESTRICT src, int width,
                                    int bpp) {
    transform_scanline_unpremultiply<true>(dst, src, width, bpp);
}

/**
 * Transform from legacy kPremul, kBGRA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static void transform_scanline_bgrA(char* SK_RESTRICT dst, const char* SK_RESTRICT src, int width,
                                    int bpp) {
    transform_scanline_unpremultiply<false>(dst, src, width, bpp);
}

template <bool kIsRGBA>
static inline void transform_scanline_unpremultiply_sRGB(void* dst, const void* src, int width, int)
{
    SkRasterPipeline p;
    p.append(SkRasterPipeline::load_8888, &src);
    if (!kIsRGBA) {
        p.append(SkRasterPipeline::swap_rb);
    }

    p.append_from_srgb(kPremul_SkAlphaType);
    p.append(SkRasterPipeline::unpremul);
    p.append(SkRasterPipeline::to_srgb);
    p.append(SkRasterPipeline::store_8888, &dst);
    p.run(0, 0, width);
}

/**
 * Transform from kPremul, kRGBA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static void transform_scanline_srgbA(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                     int width, int bpp) {
    transform_scanline_unpremultiply_sRGB<true>(dst, src, width, bpp);
}

/**
 * Transform from kPremul, kBGRA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static void transform_scanline_sbgrA(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                     int width, int bpp) {
    transform_scanline_unpremultiply_sRGB<false>(dst, src, width, bpp);
}

/**
 * Transform from kUnpremul, kBGRA_8888_SkColorType to 4-bytes-per-pixel unpremultiplied RGBA.
 */
static void transform_scanline_BGRA(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                    int width, int) {
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
static void transform_scanline_4444(char* SK_RESTRICT dst, const char* SK_RESTRICT src,
                                    int width, int) {
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
