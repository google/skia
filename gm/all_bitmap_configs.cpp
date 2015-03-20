/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "SkSurface.h"
#include "Resources.h"
#include "gm.h"

static SkBitmap copy_bitmap(const SkBitmap& src, SkColorType colorType) {
    SkBitmap copy;
    src.copyTo(&copy, colorType);
    copy.setImmutable();
    return copy;
}

// Make either A8 or gray8 bitmap.
static SkBitmap make_bitmap(bool alpha) {
    SkBitmap bm;
    SkImageInfo info = alpha ? SkImageInfo::MakeA8(128, 128)
                             : SkImageInfo::Make(128, 128, kGray_8_SkColorType,
                                                 kOpaque_SkAlphaType);
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);
    uint8_t spectrum[256];
    for (int y = 0; y < 256; ++y) {
        spectrum[y] = y;
    }
    for (int y = 0; y < 128; ++y) {
        // Shift over one byte each scanline.
        memcpy(bm.getAddr8(0, y), &spectrum[y], 128);
    }
    bm.setImmutable();
    return bm;
}

static void draw(SkCanvas* canvas,
                 const SkPaint& p,
                 const SkBitmap& src,
                 SkColorType colorType,
                 const char text[]) {
    SkASSERT(src.colorType() == colorType);
    canvas->drawBitmap(src, 0.0f, 0.0f);
    canvas->drawText(text, strlen(text), 0.0f, 12.0f, p);
}

#define SCALE 128
DEF_SIMPLE_GM(all_bitmap_configs, canvas, SCALE, 6 * SCALE) {
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);
    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&p, NULL, SkTypeface::kBold);

    sk_tool_utils::draw_checkerboard(canvas, SK_ColorLTGRAY, SK_ColorWHITE, 8);

    SkBitmap bitmap;
    if (GetResourceAsBitmap("color_wheel.png", &bitmap)) {
        bitmap.setImmutable();
        draw(canvas, p, bitmap, kN32_SkColorType, "Native 32");

        canvas->translate(0.0f, SkIntToScalar(SCALE));
        SkBitmap copy565 = copy_bitmap(bitmap, kRGB_565_SkColorType);
        p.setColor(SK_ColorRED);
        draw(canvas, p, copy565, kRGB_565_SkColorType, "RGB 565");
        p.setColor(SK_ColorBLACK);

        canvas->translate(0.0f, SkIntToScalar(SCALE));
        SkBitmap copy4444 = copy_bitmap(bitmap, kARGB_4444_SkColorType);
        draw(canvas, p, copy4444, kARGB_4444_SkColorType, "ARGB 4444");
    } else {
        canvas->translate(0.0f, SkIntToScalar(2 * SCALE));
    }

    canvas->translate(0.0f, SkIntToScalar(SCALE));
    SkBitmap bitmapIndexed;
    if (GetResourceAsBitmap("color_wheel.gif", &bitmapIndexed)) {
        bitmapIndexed.setImmutable();
        draw(canvas, p, bitmapIndexed, kIndex_8_SkColorType, "Index 8");
    }

    canvas->translate(0.0f, SkIntToScalar(SCALE));
    SkBitmap bitmapA8 = make_bitmap(true);
    draw(canvas, p, bitmapA8, kAlpha_8_SkColorType, "Alpha 8");

    p.setColor(SK_ColorRED);
    canvas->translate(0.0f, SkIntToScalar(SCALE));
    SkBitmap bitmapG8 = make_bitmap(false);
    draw(canvas, p, bitmapG8, kGray_8_SkColorType, "Gray 8");
}
