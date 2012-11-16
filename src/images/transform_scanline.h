
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
#include "SkUnPreMultiply.h"

/**
 * Function template for transforming scanlines.
 * Transform 'width' pixels from 'src' buffer into 'dst' buffer,
 * repacking color channel data as appropriate for the given transformation.
 */
typedef void (*transform_scanline_proc)(const char* SK_RESTRICT src,
                                        int width, char* SK_RESTRICT dst);

/**
 * Identity transformation: just copy bytes from src to dst.
 */
static void transform_scanline_memcpy(const char* SK_RESTRICT src, int width,
                                      char* SK_RESTRICT dst) {
    memcpy(dst, src, width);
}

/**
 * Transform from kRGB_565_Config to 3-bytes-per-pixel RGB.
 * Alpha channel data is not present in kRGB_565_Config format, so there is no
 * alpha channel data to preserve.
 */
static void transform_scanline_565(const char* SK_RESTRICT src, int width,
                                   char* SK_RESTRICT dst) {
    const uint16_t* SK_RESTRICT srcP = (const uint16_t*)src;
    for (int i = 0; i < width; i++) {
        unsigned c = *srcP++;
        *dst++ = SkPacked16ToR32(c);
        *dst++ = SkPacked16ToG32(c);
        *dst++ = SkPacked16ToB32(c);
    }
}

/**
 * Transform from kARGB_8888_Config to 3-bytes-per-pixel RGB.
 * Alpha channel data, if any, is abandoned.
 */
static void transform_scanline_888(const char* SK_RESTRICT src, int width,
                                   char* SK_RESTRICT dst) {
    const SkPMColor* SK_RESTRICT srcP = (const SkPMColor*)src;
    for (int i = 0; i < width; i++) {
        SkPMColor c = *srcP++;
        *dst++ = SkGetPackedR32(c);
        *dst++ = SkGetPackedG32(c);
        *dst++ = SkGetPackedB32(c);
    }
}

/**
 * Transform from kARGB_4444_Config to 3-bytes-per-pixel RGB.
 * Alpha channel data, if any, is abandoned.
 */
static void transform_scanline_444(const char* SK_RESTRICT src, int width,
                                   char* SK_RESTRICT dst) {
    const SkPMColor16* SK_RESTRICT srcP = (const SkPMColor16*)src;
    for (int i = 0; i < width; i++) {
        SkPMColor16 c = *srcP++;
        *dst++ = SkPacked4444ToR32(c);
        *dst++ = SkPacked4444ToG32(c);
        *dst++ = SkPacked4444ToB32(c);
    }
}

/**
 * Transform from kARGB_8888_Config to 4-bytes-per-pixel RGBA.
 * (This would be the identity transformation, except for byte-order and
 * scaling of RGB based on alpha channel).
 */
static void transform_scanline_8888(const char* SK_RESTRICT src, int width,
                                    char* SK_RESTRICT dst) {
    const SkPMColor* SK_RESTRICT srcP = (const SkPMColor*)src;
    const SkUnPreMultiply::Scale* SK_RESTRICT table =
                                              SkUnPreMultiply::GetScaleTable();

    for (int i = 0; i < width; i++) {
        SkPMColor c = *srcP++;
        unsigned a = SkGetPackedA32(c);
        unsigned r = SkGetPackedR32(c);
        unsigned g = SkGetPackedG32(c);
        unsigned b = SkGetPackedB32(c);

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
 * Transform from kARGB_8888_Config to 4-bytes-per-pixel RGBA,
 * with scaling of RGB based on alpha channel.
 */
static void transform_scanline_4444(const char* SK_RESTRICT src, int width,
                                    char* SK_RESTRICT dst) {
    const SkPMColor16* SK_RESTRICT srcP = (const SkPMColor16*)src;
    const SkUnPreMultiply::Scale* SK_RESTRICT table =
                                              SkUnPreMultiply::GetScaleTable();

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
