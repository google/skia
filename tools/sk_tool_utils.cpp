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
#include "SkPoint3.h"
#include "SkShader.h"
#include "SkTestScalerContext.h"
#include "SkTextBlob.h"

DEFINE_bool(portableFonts, false, "Use portable fonts");

namespace sk_tool_utils {

/* these are the default fonts chosen by Chrome for serif, sans-serif, and monospace */
static const char* gStandardFontNames[][3] = {
    { "Times", "Helvetica", "Courier" }, // Mac
    { "Times New Roman", "Helvetica", "Courier" }, // iOS
    { "Times New Roman", "Arial", "Courier New" }, // Win
    { "Times New Roman", "Arial", "Monospace" }, // Ubuntu
    { "serif", "sans-serif", "monospace" }, // Android
    { "Tinos", "Arimo", "Cousine" } // ChromeOS
};

const char* platform_font_name(const char* name) {
    SkString platform = major_platform_os_name();
    int index;
    if (!strcmp(name, "serif")) {
        index = 0;
    } else if (!strcmp(name, "san-serif")) {
        index = 1;
    } else if (!strcmp(name, "monospace")) {
        index = 2;
    } else {
        return name;
    }
    if (platform.equals("Mac")) {
        return gStandardFontNames[0][index];
    }
    if (platform.equals("iOS")) {
        return gStandardFontNames[1][index];
    }
    if (platform.equals("Win")) {
        return gStandardFontNames[2][index];
    }
    if (platform.equals("Ubuntu")) {
        return gStandardFontNames[3][index];
    }
    if (platform.equals("Android")) {
        return gStandardFontNames[4][index];
    }
    if (platform.equals("ChromeOS")) {
        return gStandardFontNames[5][index];
    }
    return name;
}

const char* platform_os_emoji() {
    const char* osName = platform_os_name();
    if (!strcmp(osName, "Android") || !strcmp(osName, "Ubuntu")) {
        return "CBDT";
    }
    if (!strncmp(osName, "Mac", 3)) {
        return "SBIX";
    }
    return "";
}

void emoji_typeface(SkAutoTUnref<SkTypeface>* tf) {
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "CBDT")) {
        tf->reset(GetResourceAsTypeface("/fonts/Funkster.ttf"));
        return;
    }
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "SBIX")) {
        tf->reset(SkTypeface::CreateFromName("Apple Color Emoji", SkTypeface::kNormal));
        return;
    }
    tf->reset(NULL);
    return;
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

// omit version number in returned value
SkString major_platform_os_name() {
    SkString name;
    for (int index = 0; index < FLAGS_key.count(); index += 2) {
        if (!strcmp("os", FLAGS_key[index])) {
            const char* platform = FLAGS_key[index + 1];
            const char* end = platform;
            while (*end && (*end < '0' || *end > '9')) {
                ++end;
            }
            name.append(platform, end - platform);
            break;
        }
    }
    return name;
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
    return create_font(name, style);
}

void set_portable_typeface(SkPaint* paint, const char* name, SkTypeface::Style style) {
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

static inline void norm_to_rgb(SkBitmap* bm, int x, int y, const SkVector3& norm) {
    SkASSERT(SkScalarNearlyEqual(norm.length(), 1.0f));
    unsigned char r = static_cast<unsigned char>((0.5f * norm.fX + 0.5f) * 255);
    unsigned char g = static_cast<unsigned char>((-0.5f * norm.fY + 0.5f) * 255);
    unsigned char b = static_cast<unsigned char>((0.5f * norm.fZ + 0.5f) * 255);
    *bm->getAddr32(x, y) = SkPackARGB32(0xFF, r, g, b);
}

void create_hemi_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center = SkPoint::Make(dst.fLeft + (dst.width() / 2.0f),
                                         dst.fTop + (dst.height() / 2.0f));
    const SkPoint halfSize = SkPoint::Make(dst.width() / 2.0f, dst.height() / 2.0f);

    SkVector3 norm;

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            norm.fX = (x + 0.5f - center.fX) / halfSize.fX;
            norm.fY = (y + 0.5f - center.fY) / halfSize.fY;
            
            SkScalar tmp = norm.fX * norm.fX + norm.fY * norm.fY;
            if (tmp >= 1.0f) {
                norm.set(0.0f, 0.0f, 1.0f);
            } else {
                norm.fZ = sqrt(1.0f - tmp);
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

void create_frustum_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center = SkPoint::Make(dst.fLeft + (dst.width() / 2.0f),
                                         dst.fTop + (dst.height() / 2.0f));

    SkIRect inner = dst;
    inner.inset(dst.width()/4, dst.height()/4);

    SkPoint3 norm;
    const SkPoint3 left =  SkPoint3::Make(-SK_ScalarRoot2Over2, 0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 up =    SkPoint3::Make(0.0f, -SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);
    const SkPoint3 right = SkPoint3::Make(SK_ScalarRoot2Over2,  0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 down =  SkPoint3::Make(0.0f,  SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            if (inner.contains(x, y)) {
                norm.set(0.0f, 0.0f, 1.0f);
            } else {
                SkScalar locX = x + 0.5f - center.fX;
                SkScalar locY = y + 0.5f - center.fY;

                if (locX >= 0.0f) {
                    if (locY > 0.0f) {
                        norm = locX >= locY ? right : down;   // LR corner
                    } else {
                        norm = locX > -locY ? right : up;     // UR corner
                    }    
                } else {
                    if (locY > 0.0f) {
                        norm = -locX > locY ? left : down;    // LL corner
                    } else {
                        norm = locX > locY ? up : left;       // UL corner
                    }    
                }
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

void create_tetra_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center = SkPoint::Make(dst.fLeft + (dst.width() / 2.0f),
                                         dst.fTop + (dst.height() / 2.0f));

    static const SkScalar k1OverRoot3 = 0.5773502692f;

    SkPoint3 norm;
    const SkPoint3 leftUp =  SkPoint3::Make(-k1OverRoot3, -k1OverRoot3, k1OverRoot3);
    const SkPoint3 rightUp = SkPoint3::Make(k1OverRoot3,  -k1OverRoot3, k1OverRoot3);
    const SkPoint3 down =  SkPoint3::Make(0.0f,  SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            SkScalar locX = x + 0.5f - center.fX;
            SkScalar locY = y + 0.5f - center.fY;

            if (locX >= 0.0f) {
                if (locY > 0.0f) {
                    norm = locX >= locY ? rightUp : down;   // LR corner
                } else {
                    norm = rightUp;
                }    
            } else {
                if (locY > 0.0f) {
                    norm = -locX > locY ? leftUp : down;    // LL corner
                } else {
                    norm = leftUp;
                }    
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

}  // namespace sk_tool_utils
