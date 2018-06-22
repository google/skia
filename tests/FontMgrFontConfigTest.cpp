/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fontconfig/fontconfig.h>
#include "Resources.h"
#include "SkCanvas.h"
#include "SkFontMgr.h"
#include "SkFontMgr_fontconfig.h"
#include "SkTypeface.h"
#include "Test.h"

static bool bitmap_compare(const SkBitmap& ref, const SkBitmap& test) {
    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            SkColor refColor = ref.getColor(x, y);
            if (refColor != testColor) {
                return false;
            }
        }
    }
    return true;
}

DEF_TEST(FontMgrFontConfig, reporter) {
    FcConfig* config = FcConfigCreate();
    SkString distortablePath = GetResourcePath("fonts/Distortable.ttf");
    FcConfigAppFontAddFile(
        config, reinterpret_cast<const FcChar8*>(distortablePath.c_str()));
    FcConfigBuildFonts(config);

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config));
    sk_sp<SkTypeface> typeface(fontMgr->legacyMakeTypeface("Distortable", SkFontStyle()));

    SkBitmap bitmapStream;
    bitmapStream.allocN32Pixels(64, 64);
    SkCanvas canvasStream(bitmapStream);
    canvasStream.drawColor(SK_ColorWHITE);

    SkBitmap bitmapClone;
    bitmapClone.allocN32Pixels(64, 64);
    SkCanvas canvasClone(bitmapClone);
    canvasStream.drawColor(SK_ColorWHITE);

    SkPaint paintStream;
    paintStream.setColor(SK_ColorGRAY);
    paintStream.setTextSize(SkIntToScalar(20));
    paintStream.setAntiAlias(true);
    paintStream.setLCDRenderText(true);

    SkPaint paintClone;
    paintClone.setColor(SK_ColorGRAY);
    paintClone.setTextSize(SkIntToScalar(20));
    paintClone.setAntiAlias(true);
    paintClone.setLCDRenderText(true);

    std::unique_ptr<SkStreamAsset> distortableStream(
        GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortableStream) {
        return;
    }

    const char* text = "abc";
    const size_t textLen = strlen(text);
    SkPoint point = SkPoint::Make(20.0f, 20.0f);
    SkFourByteTag tag = SkSetFourByteTag('w', 'g', 'h', 't');

    for (int i = 0; i < 10; ++i) {
        SkScalar styleValue =
            SkDoubleToScalar(0.5 + i * ((2.0 - 0.5) / 10));
        SkFontArguments::VariationPosition::Coordinate
            coordinates[] = {{tag, styleValue}};
        SkFontArguments::VariationPosition
            position = {coordinates, SK_ARRAY_COUNT(coordinates)};

        paintStream.setTypeface(sk_sp<SkTypeface>(
            fontMgr->makeFromStream(distortableStream->duplicate(),
                                    SkFontArguments().setVariationDesignPosition(position))));
        paintClone.setTypeface(sk_sp<SkTypeface>(
            typeface->makeClone(SkFontArguments().setVariationDesignPosition(position))));

        canvasStream.drawColor(SK_ColorWHITE);
        canvasStream.drawText(text, textLen, point.fX, point.fY, paintStream);

        canvasClone.drawColor(SK_ColorWHITE);
        canvasClone.drawText(text, textLen, point.fX, point.fY, paintClone);

        bool success = bitmap_compare(bitmapStream, bitmapClone);
        REPORTER_ASSERT(reporter, success);
    }
}