/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkFontMgr_fontconfig.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <fontconfig/fontconfig.h>

#include <array>
#include <memory>

namespace {

bool bitmap_compare(const SkBitmap& ref, const SkBitmap& test) {
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

FcConfig* build_fontconfig_with_fontfile(const char* fontFilename) {
    FcConfig* config = FcConfigCreate();

    // FontConfig may modify the passed path (make absolute or other).
    FcConfigSetSysRoot(config, reinterpret_cast<const FcChar8*>(GetResourcePath("").c_str()));
    // FontConfig will lexically compare paths against its version of the sysroot.
    SkString fontFilePath(reinterpret_cast<const char*>(FcConfigGetSysRoot(config)));
    fontFilePath += fontFilename;
    FcConfigAppFontAddFile(config, reinterpret_cast<const FcChar8*>(fontFilePath.c_str()));

    FcConfigBuildFonts(config);
    return config;
}

}  // namespace

DEF_TEST(FontMgrFontConfig, reporter) {
    FcConfig* config = build_fontconfig_with_fontfile("/fonts/Distortable.ttf");

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config));
    sk_sp<SkTypeface> typeface(fontMgr->legacyMakeTypeface("Distortable", SkFontStyle()));
    if (!typeface) {
        ERRORF(reporter, "Could not find typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    SkBitmap bitmapStream;
    bitmapStream.allocN32Pixels(64, 64);
    SkCanvas canvasStream(bitmapStream);
    canvasStream.drawColor(SK_ColorWHITE);

    SkBitmap bitmapClone;
    bitmapClone.allocN32Pixels(64, 64);
    SkCanvas canvasClone(bitmapClone);
    canvasStream.drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorGRAY);

    constexpr float kTextSize = 20;

    std::unique_ptr<SkStreamAsset> distortableStream(
        GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortableStream) {
        return;
    }

    SkPoint point = SkPoint::Make(20.0f, 20.0f);
    SkFourByteTag tag = SkSetFourByteTag('w', 'g', 'h', 't');

    for (int i = 0; i < 10; ++i) {
        SkScalar styleValue =
            SkDoubleToScalar(0.5 + i * ((2.0 - 0.5) / 10));
        SkFontArguments::VariationPosition::Coordinate
            coordinates[] = {{tag, styleValue}};
        SkFontArguments::VariationPosition
            position = {coordinates, std::size(coordinates)};

        SkFont fontStream(
            fontMgr->makeFromStream(distortableStream->duplicate(),
                                    SkFontArguments().setVariationDesignPosition(position)),
            kTextSize);
        fontStream.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkFont fontClone(
            typeface->makeClone(SkFontArguments().setVariationDesignPosition(position)), kTextSize);
        fontClone.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        constexpr char text[] = "abc";

        canvasStream.drawColor(SK_ColorWHITE);
        canvasStream.drawString(text, point.fX, point.fY, fontStream, paint);

        canvasClone.drawColor(SK_ColorWHITE);
        canvasClone.drawString(text, point.fX, point.fY, fontClone, paint);

        bool success = bitmap_compare(bitmapStream, bitmapClone);
        REPORTER_ASSERT(reporter, success);
    }
}
