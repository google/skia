
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvas.h"
#include "SkColorPriv.h"

namespace {

/**
  Copies all pixels from a bitmap to a dst ptr with a given rowBytes and
  Config8888. The bitmap must have kARGB_8888_Config.
 */
inline void SkCopyBitmapToConfig8888(uint32_t* dstPixels,
                                     size_t dstRowBytes,
                                     SkCanvas::Config8888 dstConfig8888,
                                     const SkBitmap& srcBmp);

/**
 * Copies all pixels in a bitmap to a dst ptr with row bytes. The src bitmap
 * is assumed to have pixels and be kARGB_8888_Config. No conversion is applied
 */
inline void SkCopyARGB8888BitmapTo(uint32_t* dstPixels,
                                   size_t dstRowBytes,
                                   const SkBitmap& srcBmp);

/**
  Copies over all pixels in a bitmap from a src ptr with a given rowBytes and
  Config8888. The bitmap must have pixels and be kARGB_8888_Config.
 */
inline void SkCopyConfig8888ToBitmap(const SkBitmap& dstBmp,
                                     const uint32_t* srcPixels,
                                     size_t srcRowBytes,
                                     SkCanvas::Config8888 srcConfig8888);

}

///////////////////////////////////////////////////////////////////////////////
// Implementation

namespace {

template <int A_IDX, int R_IDX, int G_IDX, int B_IDX>
inline uint32_t pack_config8888(uint32_t a, uint32_t r,
                                uint32_t g, uint32_t b) {
#ifdef SK_CPU_LENDIAN
    return (a << (A_IDX * 8)) | (r << (R_IDX * 8)) |
           (g << (G_IDX * 8)) | (b << (B_IDX * 8));
#else
    return (a << ((3-A_IDX) * 8)) | (r << ((3-R_IDX) * 8)) |
           (g << ((3-G_IDX) * 8)) | (b << ((3-B_IDX) * 8));
#endif
}

template <int A_IDX, int R_IDX, int G_IDX, int B_IDX>
inline void unpack_config8888(uint32_t color,
                              uint32_t* a, uint32_t* r,
                              uint32_t* g, uint32_t* b) {
#ifdef SK_CPU_LENDIAN
    *a = (color >> (A_IDX * 8)) & 0xff;
    *r = (color >> (R_IDX * 8)) & 0xff;
    *g = (color >> (G_IDX * 8)) & 0xff;
    *b = (color >> (B_IDX * 8)) & 0xff;
#else
    *a = (color >> ((3 - A_IDX) * 8)) & 0xff;
    *r = (color >> ((3 - R_IDX) * 8)) & 0xff;
    *g = (color >> ((3 - G_IDX) * 8)) & 0xff;
    *b = (color >> ((3 - B_IDX) * 8)) & 0xff;
#endif
}

template <bool UNPM, int A_IDX, int R_IDX, int G_IDX, int B_IDX>
inline void bitmap_copy_to_config8888(uint32_t* dstPixels,
                                      size_t dstRowBytes,
                                      const SkBitmap& srcBmp) {
    SkASSERT(SkBitmap::kARGB_8888_Config == srcBmp.config());
    SkAutoLockPixels alp(srcBmp);
    int w = srcBmp.width();
    int h = srcBmp.height();
    size_t srcRowBytes = srcBmp.rowBytes();

    intptr_t src = reinterpret_cast<intptr_t>(srcBmp.getPixels());
    intptr_t dst = reinterpret_cast<intptr_t>(dstPixels);

    for (int y = 0; y < h; ++y) {
        const SkPMColor* srcRow = reinterpret_cast<SkPMColor*>(src);
        uint32_t* dstRow  = reinterpret_cast<uint32_t*>(dst);
        for (int x = 0; x < w; ++x) {
            SkPMColor pmcolor = srcRow[x];
            if (UNPM) {
                U8CPU a, r, g, b;
                a = SkGetPackedA32(pmcolor);
                if (a) {
                    // We're doing the explicit divide to match WebKit layout
                    // test expectations. We can modify and rebaseline if there
                    // it can be shown that there is a more performant way to
                    // unpremul.
                    r = SkGetPackedR32(pmcolor) * 0xff / a;
                    g = SkGetPackedG32(pmcolor) * 0xff / a;
                    b = SkGetPackedB32(pmcolor) * 0xff / a;
                    dstRow[x] = pack_config8888<A_IDX, R_IDX,
                                                G_IDX, B_IDX>(a, r, g, b);
                } else {
                    dstRow[x] = 0;
                }
            } else {
                dstRow[x] = pack_config8888<A_IDX, R_IDX,
                                            G_IDX, B_IDX>(
                                                   SkGetPackedA32(pmcolor),
                                                   SkGetPackedR32(pmcolor),
                                                   SkGetPackedG32(pmcolor),
                                                   SkGetPackedB32(pmcolor));
            }
        }
        dst += dstRowBytes;
        src += srcRowBytes;
    }
}

template <bool PM, int A_IDX, int R_IDX, int G_IDX, int B_IDX>
inline void config8888_copy_to_bitmap(const SkBitmap& dstBmp,
                                      const uint32_t* srcPixels,
                                      size_t srcRowBytes) {
    SkASSERT(SkBitmap::kARGB_8888_Config == dstBmp.config());
    SkAutoLockPixels alp(dstBmp);
    int w = dstBmp.width();
    int h = dstBmp.height();
    size_t dstRowBytes = dstBmp.rowBytes();

    intptr_t src = reinterpret_cast<intptr_t>(srcPixels);
    intptr_t dst = reinterpret_cast<intptr_t>(dstBmp.getPixels());

    for (int y = 0; y < h; ++y) {
        const uint32_t* srcRow  = reinterpret_cast<uint32_t*>(src);
        SkPMColor* dstRow = reinterpret_cast<SkPMColor*>(dst);
        for (int x = 0; x < w; ++x) {
            uint32_t c8888 = srcRow[x];
            uint32_t a, r, g, b;
            unpack_config8888<A_IDX, R_IDX, G_IDX, B_IDX>(c8888, &a, &r,
                                                                 &g, &b);
            if (PM) {
                // This matches WebKit's conversion which we are replacing.
                // We can consider alternative rounding rules for performance.
                r = SkMulDiv255Ceiling(r, a);
                g = SkMulDiv255Ceiling(g, a);
                b = SkMulDiv255Ceiling(b, a);
            }
            // NoCheck: https://bugs.webkit.org/show_bug.cgi?id=74025
            dstRow[x] = SkPackARGB32NoCheck(a, r, g, b);
        }
        src += srcRowBytes;
        dst += dstRowBytes;
    }
}

#ifdef SK_CPU_LENDIAN
    static const int SK_NATIVE_A_IDX = SK_A32_SHIFT / 8;
    static const int SK_NATIVE_R_IDX = SK_R32_SHIFT / 8;
    static const int SK_NATIVE_G_IDX = SK_G32_SHIFT / 8;
    static const int SK_NATIVE_B_IDX = SK_B32_SHIFT / 8;
#else
    static const int SK_NATIVE_A_IDX = 3 - (SK_A32_SHIFT / 8);
    static const int SK_NATIVE_R_IDX = 3 - (SK_R32_SHIFT / 8);
    static const int SK_NATIVE_G_IDX = 3 - (SK_G32_SHIFT / 8);
    static const int SK_NATIVE_B_IDX = 3 - (SK_B32_SHIFT / 8);
#endif

inline void SkCopyBitmapToConfig8888(uint32_t* dstPixels,
                                     size_t dstRowBytes,
                                     SkCanvas::Config8888 dstConfig8888,
                                     const SkBitmap& srcBmp) {
    switch (dstConfig8888) {
        case SkCanvas::kNative_Premul_Config8888:
            bitmap_copy_to_config8888<false,
                                      SK_NATIVE_A_IDX, SK_NATIVE_R_IDX,
                                      SK_NATIVE_G_IDX, SK_NATIVE_B_IDX>(
                                            dstPixels,
                                            dstRowBytes,
                                            srcBmp);
            break;
        case SkCanvas::kNative_Unpremul_Config8888:
            bitmap_copy_to_config8888<true,
                                      SK_NATIVE_A_IDX, SK_NATIVE_R_IDX,
                                      SK_NATIVE_G_IDX, SK_NATIVE_B_IDX>(
                                            dstPixels,
                                            dstRowBytes,
                                            srcBmp);
            break;
        case SkCanvas::kBGRA_Premul_Config8888:
            bitmap_copy_to_config8888<false, 3, 2, 1, 0> (
                                    dstPixels, dstRowBytes, srcBmp);
            break;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            bitmap_copy_to_config8888<true, 3, 2, 1, 0> (
                                    dstPixels, dstRowBytes, srcBmp);
            break;
        case SkCanvas::kRGBA_Premul_Config8888:
            bitmap_copy_to_config8888<false, 3, 0, 1, 2> (
                                    dstPixels, dstRowBytes, srcBmp);
            break;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            bitmap_copy_to_config8888<true, 3, 0, 1, 2> (
                                    dstPixels, dstRowBytes, srcBmp);
            break;
        default:
            SkDEBUGFAIL("unexpected Config8888");
            break;
    }
}

inline void SkCopyConfig8888ToBitmap(const SkBitmap& dstBmp,
                                     const uint32_t* srcPixels,
                                     size_t srcRowBytes,
                                     SkCanvas::Config8888 srcConfig8888) {
    switch (srcConfig8888) {
        case SkCanvas::kNative_Premul_Config8888:
            config8888_copy_to_bitmap<false,
                                      SK_NATIVE_A_IDX, SK_NATIVE_R_IDX,
                                      SK_NATIVE_G_IDX, SK_NATIVE_B_IDX>(
                                            dstBmp,
                                            srcPixels,
                                            srcRowBytes);
            break;
        case SkCanvas::kNative_Unpremul_Config8888:
            config8888_copy_to_bitmap<true,
                                      SK_NATIVE_A_IDX, SK_NATIVE_R_IDX,
                                      SK_NATIVE_G_IDX, SK_NATIVE_B_IDX>(
                                            dstBmp,
                                            srcPixels,
                                            srcRowBytes);
            break;
        case SkCanvas::kBGRA_Premul_Config8888:
            config8888_copy_to_bitmap<false, 3, 2, 1, 0> (
                                    dstBmp, srcPixels, srcRowBytes);
            break;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            config8888_copy_to_bitmap<true, 3, 2, 1, 0> (
                                    dstBmp, srcPixels, srcRowBytes);
            break;
        case SkCanvas::kRGBA_Premul_Config8888:
            config8888_copy_to_bitmap<false, 3, 0, 1, 2> (
                                    dstBmp, srcPixels, srcRowBytes);
            break;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            config8888_copy_to_bitmap<true, 3, 0, 1, 2> (
                                    dstBmp, srcPixels, srcRowBytes);
            break;
        default:
            SkDEBUGFAIL("unexpected Config8888");
            break;
    }
}

inline void SkCopyARGB8888BitmapTo(uint32_t* dstPixels,
                                   size_t dstRowBytes,
                                   const SkBitmap& srcBmp) {
    SkASSERT(SkBitmap::kARGB_8888_Config == srcBmp.config());

    SkAutoLockPixels alp(srcBmp);

    int w = srcBmp.width();
    int h = srcBmp.height();
    size_t srcRowBytes = srcBmp.rowBytes();

    size_t tightRowBytes = w * 4;

    char* src = reinterpret_cast<char*>(srcBmp.getPixels());
    char* dst = reinterpret_cast<char*>(dstPixels);

    if (tightRowBytes == srcRowBytes &&
        tightRowBytes == dstRowBytes) {
        memcpy(dst, src, tightRowBytes * h);
    } else {
        for (int y = 0; y < h; ++y) {
            memcpy(dst, src, tightRowBytes);
            dst += dstRowBytes;
            src += srcRowBytes;
        }
    }
}

}
