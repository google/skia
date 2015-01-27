/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "sk_tool_utils_flags.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkShader.h"
#include "SkTestScalerContext.h"

DEFINE_bool(portableFonts, false, "Use portable fonts");
DEFINE_bool(resourceFonts, false, "Use resource fonts");

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

SkTypeface* create_portable_typeface(const char* name, SkTypeface::Style style) {
    SkTypeface* face;
    if (FLAGS_portableFonts) {
        face = create_font(name, style);
    } else if (FLAGS_resourceFonts) {
        face = resource_font(name, style);
    } else {
        face = SkTypeface::CreateFromName(name, style);
    }
    return face;
}

void set_portable_typeface(SkPaint* paint, const char* name, SkTypeface::Style style) {
    SkTypeface* face = create_portable_typeface(name, style);
    SkSafeUnref(paint->setTypeface(face));
}

void write_pixels(SkCanvas* canvas, const SkBitmap& bitmap, int x, int y,
                  SkColorType colorType, SkAlphaType alphaType) {
    SkBitmap tmp(bitmap);
    tmp.lockPixels();

    const SkImageInfo info = SkImageInfo::Make(tmp.width(), tmp.height(), colorType, alphaType);

    canvas->writePixels(info, tmp.getPixels(), tmp.rowBytes(), x, y);
}

SkShader* create_checkerboard_shader(SkColor c1, SkColor c2, int size) {
    SkBitmap bm;
    bm.allocN32Pixels(2 * size, 2 * size);
    bm.eraseColor(c1);
    bm.eraseArea(SkIRect::MakeLTRB(0, 0, size, size), c2);
    bm.eraseArea(SkIRect::MakeLTRB(size, size, 2 * size, 2 * size), c2);
    return SkShader::CreateBitmapShader(
            bm, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);
}

void draw_checkerboard(SkCanvas* canvas, SkColor c1, SkColor c2, int size) {
    SkPaint paint;
    paint.setShader(create_checkerboard_shader(c1, c2, size))->unref();
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas->drawPaint(paint);
}

}  // namespace sk_tool_utils
