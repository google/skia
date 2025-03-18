/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderFns_DEFINED
#define SkImageEncoderFns_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkICC.h"
#include "include/private/SkExif.h"
#include "modules/skcms/skcms.h"

#include <cstring>
#include <optional>

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

static inline void transform_scanline_bgr_101010x(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGRA_1010102, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_bgra_1010102(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGRA_1010102,    skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_bgr_101010x_xr(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGR_101010x_XR, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_161616BE,   skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_bgra_10101010_xr(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGRA_10101010_XR, skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_16161616BE,  skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_bgra_1010102_premul(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGRA_1010102,    skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_bgra_10101010_xr_premul(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_BGRA_10101010_XR, skcms_AlphaFormat_PremulAsEncoded,
          skcms_PixelFormat_RGBA_16161616BE,  skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_F16(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_hhhh,       skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul);
}

static inline void transform_scanline_F16F16F16x(char* dst, const char* src, int width, int) {
    skcms(dst, src, width,
          skcms_PixelFormat_RGBA_hhhh,      skcms_AlphaFormat_Unpremul,
          skcms_PixelFormat_RGB_161616BE,   skcms_AlphaFormat_Unpremul);
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

static inline sk_sp<SkData> icc_from_color_space(const SkColorSpace* cs,
                                                 const skcms_ICCProfile* profile,
                                                 const char* profile_description) {
    // TODO(ccameron): Remove this check.
    if (!cs) {
        return nullptr;
    }

    if (profile) {
        return SkWriteICCProfile(profile, profile_description);
    }

    skcms_Matrix3x3 toXYZD50;
    if (cs->toXYZD50(&toXYZD50)) {
        skcms_TransferFunction fn;
        cs->transferFn(&fn);
        return SkWriteICCProfile(fn, toXYZD50);
    }
    return nullptr;
}

static inline sk_sp<SkData> icc_from_color_space(const SkImageInfo& info,
                                                 const skcms_ICCProfile* profile,
                                                 const char* profile_description) {
    return icc_from_color_space(info.colorSpace(), profile, profile_description);
}

static inline sk_sp<SkData> exif_from_origin(const SkEncodedOrigin origin) {
    SkExif::Metadata metadata;
    metadata.fOrigin = origin;
    return SkExif::WriteExif(metadata);
}

#endif  // SkImageEncoderFns_DEFINED
