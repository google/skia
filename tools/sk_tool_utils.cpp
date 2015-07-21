/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "sk_tool_utils_flags.h"

#include "Resources.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkShader.h"
#include "SkTestScalerContext.h"
#include "SkTextBlob.h"

DEFINE_bool(portableFonts, false, "Use portable fonts");
DEFINE_bool(resourceFonts, false, "Use resource fonts");

namespace sk_tool_utils {

const char* platform_os_emoji() {
    const char* osName = platform_os_name();
    if (!strcmp(osName, "Android") || !strcmp(osName, "Unbuntu")) {
        return "CBDT";
    }
    if (!strncmp(osName, "Mac", 3)) {
        return "SBIX";
    }
    return "";
}

SkTypeface* emoji_typeface() {
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "CBDT")) {
        return GetResourceAsTypeface("/fonts/Funkster.ttf");
    }
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "SBIX")) {
        return SkTypeface::CreateFromName("Apple Color Emoji", SkTypeface::kNormal);
    }
    return NULL;
}

const char* emoji_sample_text() {
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "CBDT")) {
        return "Hamburgefons";
    } 
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "SBIX")) {
        return "\xF0\x9F\x92\xB0" "\xF0\x9F\x8F\xA1" "\xF0\x9F\x8E\x85"  // 💰🏡🎅
               "\xF0\x9F\x8D\xAA" "\xF0\x9F\x8D\x95" "\xF0\x9F\x9A\x80"  // 🍪🍕🚀
               "\xF0\x9F\x9A\xBB" "\xF0\x9F\x92\xA9" "\xF0\x9F\x93\xB7" // 🚻💩📷
               "\xF0\x9F\x93\xA6" // 📦
               "\xF0\x9F\x87\xBA" "\xF0\x9F\x87\xB8" "\xF0\x9F\x87\xA6"; // 🇺🇸🇦
    }
    return "";
}

const char* platform_os_name() {
    for (int index = 0; index < FLAGS_key.count(); index += 2) {
        if (!strcmp("os", FLAGS_key[index])) {
            return FLAGS_key[index + 1];
        }
    }
    // when running SampleApp or dm without a --key pair, omit the platform name
    return "";
}

const char* platform_extra_config(const char* config) {
    for (int index = 0; index < FLAGS_key.count(); index += 2) {
        if (!strcmp("extra_config", FLAGS_key[index]) && !strcmp(config, FLAGS_key[index + 1])) {
            return config;
        }
    }
    return "";
}

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

SkColor color_to_565(SkColor color) {
    SkPMColor pmColor = SkPreMultiplyColor(color);
    U16CPU color16 = SkPixel32ToPixel16(pmColor);
    return SkPixel16ToColor(color16);
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

SkTypeface* create_portable_typeface_always(const char* name, SkTypeface::Style style) {
    return create_font(name, style);
}

void set_portable_typeface(SkPaint* paint, const char* name, SkTypeface::Style style) {
    SkTypeface* face = create_portable_typeface(name, style);
    SkSafeUnref(paint->setTypeface(face));
}

void set_portable_typeface_always(SkPaint* paint, const char* name, SkTypeface::Style style) {
    SkTypeface* face = create_font(name, style);
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

void add_to_text_blob(SkTextBlobBuilder* builder, const char* text, const SkPaint& origPaint,
                      SkScalar x, SkScalar y) {
    SkPaint paint(origPaint);
    SkTDArray<uint16_t> glyphs;

    size_t len = strlen(text);
    glyphs.append(paint.textToGlyphs(text, len, NULL));
    paint.textToGlyphs(text, len, glyphs.begin());

    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    const SkTextBlobBuilder::RunBuffer& run = builder->allocRun(paint, glyphs.count(), x, y,
                                                                NULL);
    memcpy(run.glyphs, glyphs.begin(), glyphs.count() * sizeof(uint16_t));
}


}  // namespace sk_tool_utils
