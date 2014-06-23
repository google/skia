/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "../src/fonts/SkTestScalerContext.h"

#include "SkBitmap.h"
#include "SkCanvas.h"

namespace sk_tool_utils {

bool gEnablePortableTypeface = false;

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

SkPaint::FontMetrics create_font(SkTDArray<SkPath*>& , SkTDArray<SkFixed>& );

void set_portable_typeface(SkPaint* paint, SkTypeface::Style style) {
    if (gEnablePortableTypeface) {
        SkSafeUnref(paint->setTypeface(CreateTestTypeface(create_font, style)));
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

}  // namespace sk_tool_utils
