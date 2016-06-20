/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpts_DEFINED
#define SkOpts_DEFINED

#include "SkTextureCompressor.h"
#include "SkTypes.h"
#include "SkXfermode.h"

struct ProcCoeff;

namespace SkOpts {
    // Call to replace pointers to portable functions with pointers to CPU-specific functions.
    // Thread-safe and idempotent.
    // Called by SkGraphics::Init().
    void Init();

    // Declare function pointers here...

    // May return nullptr if we haven't specialized the given Mode.
    extern SkXfermode* (*create_xfermode)(const ProcCoeff&, SkXfermode::Mode);

    typedef void (*BoxBlur)(const SkPMColor*, int, const SkIRect& srcBounds, SkPMColor*, int, int, int, int, int);
    extern BoxBlur box_blur_xx, box_blur_xy, box_blur_yx;

    typedef void (*Morph)(const SkPMColor*, SkPMColor*, int, int, int, int, int);
    extern Morph dilate_x, dilate_y, erode_x, erode_y;

    typedef bool (*TextureCompressor)(uint8_t* dst, const uint8_t* src,
                                      int width, int height, size_t rowBytes);
    extern TextureCompressor (*texture_compressor)(SkColorType, SkTextureCompressor::Format);
    extern bool (*fill_block_dimensions)(SkTextureCompressor::Format, int* x, int* y);

    extern void (*blit_mask_d32_a8)(SkPMColor*, size_t, const SkAlpha*, size_t, SkColor, int, int);
    extern void (*blit_row_color32)(SkPMColor*, const SkPMColor*, int, SkPMColor);
    extern void (*blit_row_s32a_opaque)(SkPMColor*, const SkPMColor*, int, U8CPU);

    // This function is an optimized version of SkColorCubeFilter::filterSpan
    extern void (*color_cube_filter_span)(const SkPMColor[],
                                          int,
                                          SkPMColor[],
                                          const int * [2],
                                          const SkScalar * [2],
                                          int,
                                          const SkColor*);

    // Swizzle input into some sort of 8888 pixel, {premul,unpremul} x {rgba,bgra}.
    typedef void (*Swizzle_8888)(uint32_t*, const void*, int);
    extern Swizzle_8888 RGBA_to_BGRA,          // i.e. just swap RB
                        RGBA_to_rgbA,          // i.e. just premultiply
                        RGBA_to_bgrA,          // i.e. swap RB and premultiply
                        RGB_to_RGB1,           // i.e. insert an opaque alpha
                        RGB_to_BGR1,           // i.e. swap RB and insert an opaque alpha
                        gray_to_RGB1,          // i.e. expand to color channels + an opaque alpha
                        grayA_to_RGBA,         // i.e. expand to color channels
                        grayA_to_rgbA,         // i.e. expand to color channels and premultiply
                        inverted_CMYK_to_RGB1, // i.e. convert color space
                        inverted_CMYK_to_BGR1; // i.e. convert color space

    extern void (*half_to_float)(float[], const uint16_t[], int);
    extern void (*float_to_half)(uint16_t[], const float[], int);

    // Blend ndst src pixels over dst, where both src and dst point to sRGB pixels (RGBA or BGRA).
    // If nsrc < ndst, we loop over src to create a pattern.
    extern void (*srcover_srgb_srgb)(uint32_t* dst, const uint32_t* src, int ndst, int nsrc);

    // Color xform RGB1 pixels.  Does not change byte ordering.
    extern void (*color_xform_RGB1_srgb_to_2dot2) (uint32_t* dst, const uint32_t* src, int len,
                                                   const float srcToDstMatrix[16]);
    extern void (*color_xform_RGB1_2dot2_to_2dot2)(uint32_t* dst, const uint32_t* src, int len,
                                                   const float srcToDstMatrix[16]);
    extern void (*color_xform_RGB1_srgb_to_srgb) (uint32_t* dst, const uint32_t* src, int len,
                                                  const float srcToDstMatrix[16]);
    extern void (*color_xform_RGB1_2dot2_to_srgb)(uint32_t* dst, const uint32_t* src, int len,
                                                  const float srcToDstMatrix[16]);
}

#endif//SkOpts_DEFINED
