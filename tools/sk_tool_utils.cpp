#include "sk_tool_utils.h"

namespace sk_tool_utils {

const char* colortype_name(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return "Unknown";
        case kAlpha_8_SkColorType:      return "Alpha_8";
        case kIndex_8_SkColorType:      return "Index_8";
        case kARGB_4444_SkColorType:    return "ARGB_4444";
        case kRGB_565_SkColorType:      return "RGB_565";
        case kRGBA_8888_SkColorType:    return "RGBA_8888";
        case kBGRA_8888_SkColorType:    return "BGRA_8888";
        default:
            SkASSERT(false);
            return "unexpected colortype";
    }
}

void config8888_to_imagetypes(SkCanvas::Config8888 config, SkColorType* ct, SkAlphaType* at) {
    switch (config) {
        case SkCanvas::kNative_Premul_Config8888:
            *ct = kPMColor_SkColorType;
            *at = kPremul_SkAlphaType;
            break;
        case SkCanvas::kNative_Unpremul_Config8888:
            *ct = kPMColor_SkColorType;
            *at = kUnpremul_SkAlphaType;
            break;
        case SkCanvas::kBGRA_Premul_Config8888:
            *ct = kBGRA_8888_SkColorType;
            *at = kPremul_SkAlphaType;
            break;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            *ct = kBGRA_8888_SkColorType;
            *at = kUnpremul_SkAlphaType;
            break;
        case SkCanvas::kRGBA_Premul_Config8888:
            *ct = kRGBA_8888_SkColorType;
            *at = kPremul_SkAlphaType;
            break;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            *ct = kRGBA_8888_SkColorType;
            *at = kUnpremul_SkAlphaType;
            break;
        default:
            SkASSERT(0);
    }
}

void write_pixels(SkCanvas* canvas, const SkBitmap& bitmap, int x, int y,
                  SkColorType colorType, SkAlphaType alphaType) {
    SkBitmap tmp(bitmap);
    tmp.lockPixels();

    SkImageInfo info = tmp.info();
    info.fColorType = colorType;
    info.fAlphaType = alphaType;

    canvas->writePixels(info, tmp.getPixels(), tmp.rowBytes(), x, y);
}

}
