#include "SkConfig8888.h"
#include "SkMathPriv.h"

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

/**
 * convert_pixel<OUT_CFG, IN_CFG converts a pixel value from one Config8888 to
 * another. It is implemented by first expanding OUT_CFG to r, g, b, a indices
 * and an is_premul bool as params to another template function. Then IN_CFG is
 * expanded via another function call.
 */

template <bool OUT_PM, int OUT_A_IDX, int OUT_R_IDX, int OUT_G_IDX, int OUT_B_IDX,
          bool IN_PM,  int IN_A_IDX,  int IN_R_IDX,  int IN_G_IDX,  int IN_B_IDX>
inline uint32_t convert_pixel(uint32_t pixel) {
    uint32_t a, r, g, b;
    unpack_config8888<IN_A_IDX, IN_R_IDX, IN_G_IDX, IN_B_IDX>(pixel, &a, &r, &g, &b);
    if (IN_PM && !OUT_PM) {
        // We're doing the explicit divide to match WebKit layout
        // test expectations. We can modify and rebaseline if there
        // it can be shown that there is a more performant way to
        // unpremul.
        if (a) {
            r = r * 0xff / a;
            g = g * 0xff / a;
            b = b * 0xff / a;
        } else {
            return 0;
        }
    } else if (!IN_PM && OUT_PM) {
        // This matches WebKit's conversion which we are replacing.
        // We can consider alternative rounding rules for performance.
        r = SkMulDiv255Ceiling(r, a);
        g = SkMulDiv255Ceiling(g, a);
        b = SkMulDiv255Ceiling(b, a);
    }
    return pack_config8888<OUT_A_IDX, OUT_R_IDX, OUT_G_IDX, OUT_B_IDX>(a, r, g, b);
}

template <bool OUT_PM, int OUT_A_IDX, int OUT_R_IDX, int OUT_G_IDX, int OUT_B_IDX, SkCanvas::Config8888 IN_CFG>
inline uint32_t convert_pixel(uint32_t pixel) {
    switch(IN_CFG) {
        case SkCanvas::kNative_Premul_Config8888:
            return convert_pixel<OUT_PM, OUT_A_IDX,       OUT_R_IDX,       OUT_G_IDX,       OUT_B_IDX,
                                 true,  SK_NATIVE_A_IDX,  SK_NATIVE_R_IDX, SK_NATIVE_G_IDX, SK_NATIVE_B_IDX>(pixel);
            break;
        case SkCanvas::kNative_Unpremul_Config8888:
            return convert_pixel<OUT_PM, OUT_A_IDX,       OUT_R_IDX,       OUT_G_IDX,       OUT_B_IDX,
                                 false,  SK_NATIVE_A_IDX, SK_NATIVE_R_IDX, SK_NATIVE_G_IDX, SK_NATIVE_B_IDX>(pixel);
            break;
        case SkCanvas::kBGRA_Premul_Config8888:
            return convert_pixel<OUT_PM, OUT_A_IDX, OUT_R_IDX, OUT_G_IDX, OUT_B_IDX,
                                 true,  3,         2,         1,         0>(pixel);
            break;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            return convert_pixel<OUT_PM, OUT_A_IDX, OUT_R_IDX, OUT_G_IDX, OUT_B_IDX,
                                 false,  3,         2,         1,         0>(pixel);
            break;
        case SkCanvas::kRGBA_Premul_Config8888:
            return convert_pixel<OUT_PM, OUT_A_IDX, OUT_R_IDX, OUT_G_IDX, OUT_B_IDX,
                                 true,  3,         0,         1,         2>(pixel);
            break;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            return convert_pixel<OUT_PM, OUT_A_IDX, OUT_R_IDX, OUT_G_IDX, OUT_B_IDX,
                                 false,  3,         0,         1,         2>(pixel);
            break;
        default:
            SkDEBUGFAIL("Unexpected config8888");
            return 0;
            break;
    }
}

template <SkCanvas::Config8888 OUT_CFG, SkCanvas::Config8888 IN_CFG>
inline uint32_t convert_pixel(uint32_t pixel) {
    switch(OUT_CFG) {
        case SkCanvas::kNative_Premul_Config8888:
            return convert_pixel<true,  SK_NATIVE_A_IDX,  SK_NATIVE_R_IDX, SK_NATIVE_G_IDX, SK_NATIVE_B_IDX, IN_CFG>(pixel);
            break;
        case SkCanvas::kNative_Unpremul_Config8888:
            return convert_pixel<false,  SK_NATIVE_A_IDX,  SK_NATIVE_R_IDX, SK_NATIVE_G_IDX, SK_NATIVE_B_IDX, IN_CFG>(pixel);
            break;
        case SkCanvas::kBGRA_Premul_Config8888:
            return convert_pixel<true, 3, 2, 1, 0, IN_CFG>(pixel);
            break;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            return convert_pixel<false, 3, 2, 1, 0, IN_CFG>(pixel);
            break;
        case SkCanvas::kRGBA_Premul_Config8888:
            return convert_pixel<true, 3, 0, 1, 2, IN_CFG>(pixel);
            break;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            return convert_pixel<false, 3, 0, 1, 2, IN_CFG>(pixel);
            break;
        default:
            SkDEBUGFAIL("Unexpected config8888");
            return 0;
            break;
    }
}

/**
 * SkConvertConfig8888Pixels has 6 * 6 possible combinations of src and dst
 * configs. Each is implemented as an instantiation templated function. Two
 * levels of switch statements are used to select the correct instantiation, one
 * for the src config and one for the dst config.
 */

template <SkCanvas::Config8888 DST_CFG, SkCanvas::Config8888 SRC_CFG>
inline void convert_config8888(uint32_t* dstPixels,
                               size_t dstRowBytes,
                               const uint32_t* srcPixels,
                               size_t srcRowBytes,
                               int width,
                               int height) {
    intptr_t dstPix = reinterpret_cast<intptr_t>(dstPixels);
    intptr_t srcPix = reinterpret_cast<intptr_t>(srcPixels);

    for (int y = 0; y < height; ++y) {
        srcPixels = reinterpret_cast<const uint32_t*>(srcPix);
        dstPixels = reinterpret_cast<uint32_t*>(dstPix);
        for (int x = 0; x < width; ++x) {
            dstPixels[x] = convert_pixel<DST_CFG, SRC_CFG>(srcPixels[x]);
        }
        dstPix += dstRowBytes;
        srcPix += srcRowBytes;
    }
}

template <SkCanvas::Config8888 SRC_CFG>
inline void convert_config8888(uint32_t* dstPixels,
                               size_t dstRowBytes,
                               SkCanvas::Config8888 dstConfig,
                               const uint32_t* srcPixels,
                               size_t srcRowBytes,
                               int width,
                               int height) {
    switch(dstConfig) {
        case SkCanvas::kNative_Premul_Config8888:
            convert_config8888<SkCanvas::kNative_Premul_Config8888, SRC_CFG>(dstPixels, dstRowBytes, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kNative_Unpremul_Config8888:
            convert_config8888<SkCanvas::kNative_Unpremul_Config8888, SRC_CFG>(dstPixels, dstRowBytes, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kBGRA_Premul_Config8888:
            convert_config8888<SkCanvas::kBGRA_Premul_Config8888, SRC_CFG>(dstPixels, dstRowBytes, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            convert_config8888<SkCanvas::kBGRA_Unpremul_Config8888, SRC_CFG>(dstPixels, dstRowBytes, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kRGBA_Premul_Config8888:
            convert_config8888<SkCanvas::kRGBA_Premul_Config8888, SRC_CFG>(dstPixels, dstRowBytes, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            convert_config8888<SkCanvas::kRGBA_Unpremul_Config8888, SRC_CFG>(dstPixels, dstRowBytes, srcPixels, srcRowBytes, width, height);
            break;
        default:
            SkDEBUGFAIL("Unexpected config8888");
            break;
    }
}

}

void SkConvertConfig8888Pixels(uint32_t* dstPixels,
                               size_t dstRowBytes,
                               SkCanvas::Config8888 dstConfig,
                               const uint32_t* srcPixels,
                               size_t srcRowBytes,
                               SkCanvas::Config8888 srcConfig,
                               int width,
                               int height) {
    if (srcConfig == dstConfig) {
        if (srcPixels == dstPixels) {
            return;
        }
        if (dstRowBytes == srcRowBytes &&
            4U * width == srcRowBytes) {
            memcpy(dstPixels, srcPixels, srcRowBytes * height);
            return;
        } else {
            intptr_t srcPix = reinterpret_cast<intptr_t>(srcPixels);
            intptr_t dstPix = reinterpret_cast<intptr_t>(dstPixels);
            for (int y = 0; y < height; ++y) {
                srcPixels = reinterpret_cast<const uint32_t*>(srcPix);
                dstPixels = reinterpret_cast<uint32_t*>(dstPix);
                memcpy(dstPixels, srcPixels, 4 * width);
                srcPix += srcRowBytes;
                dstPix += dstRowBytes;
            }
            return;
        }
    }
    switch(srcConfig) {
        case SkCanvas::kNative_Premul_Config8888:
            convert_config8888<SkCanvas::kNative_Premul_Config8888>(dstPixels, dstRowBytes, dstConfig, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kNative_Unpremul_Config8888:
            convert_config8888<SkCanvas::kNative_Unpremul_Config8888>(dstPixels, dstRowBytes, dstConfig, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kBGRA_Premul_Config8888:
            convert_config8888<SkCanvas::kBGRA_Premul_Config8888>(dstPixels, dstRowBytes, dstConfig, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            convert_config8888<SkCanvas::kBGRA_Unpremul_Config8888>(dstPixels, dstRowBytes, dstConfig, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kRGBA_Premul_Config8888:
            convert_config8888<SkCanvas::kRGBA_Premul_Config8888>(dstPixels, dstRowBytes, dstConfig, srcPixels, srcRowBytes, width, height);
            break;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            convert_config8888<SkCanvas::kRGBA_Unpremul_Config8888>(dstPixels, dstRowBytes, dstConfig, srcPixels, srcRowBytes, width, height);
            break;
        default:
            SkDEBUGFAIL("Unexpected config8888");
            break;
    }
}

uint32_t SkPackConfig8888(SkCanvas::Config8888 config,
                          uint32_t a,
                          uint32_t r,
                          uint32_t g,
                          uint32_t b) {
    switch (config) {
        case SkCanvas::kNative_Premul_Config8888:
        case SkCanvas::kNative_Unpremul_Config8888:
            return pack_config8888<SK_NATIVE_A_IDX,
                                   SK_NATIVE_R_IDX,
                                   SK_NATIVE_G_IDX,
                                   SK_NATIVE_B_IDX>(a, r, g, b);
        case SkCanvas::kBGRA_Premul_Config8888:
        case SkCanvas::kBGRA_Unpremul_Config8888:
            return pack_config8888<3, 2, 1, 0>(a, r, g, b);
        case SkCanvas::kRGBA_Premul_Config8888:
        case SkCanvas::kRGBA_Unpremul_Config8888:
            return pack_config8888<3, 0, 1, 2>(a, r, g, b);
        default:
            SkDEBUGFAIL("Unexpected config8888");
            return 0;
    }
}
