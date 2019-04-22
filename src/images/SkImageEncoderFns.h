/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderFns_DEFINED
#define SkImageEncoderFns_DEFINED

#include "skcms.h"
#include "SkColor.h"
#include "SkColorData.h"
#include "SkICC.h"
#include "SkTypes.h"

typedef void (*transform_scanline_proc)(char* dst, const char* src, int width, int bpp);

static inline void transform_scanline_memcpy(char* dst, const char* src, int width, int bpp) {
    memcpy(dst, src, width * bpp);
}

static inline void transform_scanline_A8_to_GrayAlpha(char* dst, const char* src, int width, int) {
    for (int i = 0; i < width; i++) {
        *dst++ = 0;
        *dst++ = *src++;
    }
}


static void skcms(char* dst, const char* src, int n,
                  skcms_PixelFormat srcFmt, skcms_AlphaFormat srcAlpha,
                  skcms_PixelFormat dstFmt, skcms_AlphaFormat dstAlpha) {
    SkAssertResult(skcms_Transform(src, srcFmt, srcAlpha, nullptr,
                                   dst, dstFmt, dstAlpha, nullptr, n));
}

static inline void transform_scanline_gray(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_G_8,     skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_565(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGR_565, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_RGBX(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_888  , skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_BGRX(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGRA_8888, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_888  , skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_444(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_ABGR_4444, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_888  , skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_rgbA(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_bgrA(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGRA_8888, skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_to_premul_legacy(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_PremulAsEncoded);
}

static inline void transform_scanline_BGRA(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGRA_8888, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_4444(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_ABGR_4444, skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_101010x(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_1010102, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_1010102(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_1010102,    skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_1010102_premul(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_1010102,    skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_F16(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_hhhh,       skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_F16_premul(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_hhhh,       skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_F16_to_8888(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_hhhh, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_F16_premul_to_8888(char* dst,
                                                         const char* src,
                                                         int width,
                                                         int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_hhhh, skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_F16_to_premul_8888(char* dst,
                                                         const char* src,
                                                         int width,
                                                         int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_hhhh, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_8888, skcms_AlphaFormat_PremulAsEncoded);
}

static inline void transform_scanline_F32(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_ffff,       skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_F32_premul(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_ffff,       skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline sk_sp<SkData> icc_from_color_space(const SkImageInfo& info) {
    SkColorSpace* cs = info.colorSpace();
    if (!cs) {
        return nullptr;
    }

    skcms_TransferFunction fn;
    skcms_Matrix3x3 toXYZD50;
    if (cs->isNumericalTransferFn(&fn) && cs->toXYZD50(&toXYZD50)) {
        return SkWriteICCProfile(fn, toXYZD50);
    }
    return nullptr;
}

#endif  // SkImageEncoderFns_DEFINED
