/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/ToolUtils.h"

#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkMutex.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/base/SkUTF.h"
#include "src/core/SkOSFile.h"
#include "tools/Resources.h"
#include "tools/fonts/TestFontMgr.h"

namespace ToolUtils {

sk_sp<SkTypeface> planet_typeface() {
    static const sk_sp<SkTypeface> planetTypeface = []() {
        const char* filename;
#if defined(SK_BUILD_FOR_WIN)
        filename = "fonts/planetcolr.ttf";
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        filename = "fonts/planetsbix.ttf";
#else
        filename = "fonts/planetcbdt.ttf";
#endif
        sk_sp<SkTypeface> typeface = MakeResourceAsTypeface(filename);
        if (typeface) {
            return typeface;
        }
        return SkTypeface::MakeFromName("Planet", SkFontStyle());
    }();
    return planetTypeface;
}

sk_sp<SkTypeface> emoji_typeface() {
    static const sk_sp<SkTypeface> emojiTypeface = []() {
        const char* filename;
#if defined(SK_BUILD_FOR_WIN)
        filename = "fonts/colr.ttf";
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        filename = "fonts/sbix.ttf";
#else
        filename = "fonts/cbdt.ttf";
#endif
        sk_sp<SkTypeface> typeface = MakeResourceAsTypeface(filename);
        if (typeface) {
            return typeface;
        }
        return SkTypeface::MakeFromName("Emoji", SkFontStyle());
    }();
    return emojiTypeface;
}

sk_sp<SkTypeface> sample_user_typeface() {
    SkCustomTypefaceBuilder builder;
    SkFont font;
    const float upem = 200;

    {
        SkFontMetrics metrics;
        metrics.fFlags = 0;
        metrics.fTop = -200;
        metrics.fAscent = -150;
        metrics.fDescent = 50;
        metrics.fBottom = -75;
        metrics.fLeading = 10;
        metrics.fAvgCharWidth = 150;
        metrics.fMaxCharWidth = 300;
        metrics.fXMin = -20;
        metrics.fXMax = 290;
        metrics.fXHeight = -100;
        metrics.fCapHeight = 0;
        metrics.fUnderlineThickness = 5;
        metrics.fUnderlinePosition = 2;
        metrics.fStrikeoutThickness = 5;
        metrics.fStrikeoutPosition = -50;
        builder.setMetrics(metrics, 1.0f/upem);
    }
    builder.setFontStyle(SkFontStyle(367, 3, SkFontStyle::kOblique_Slant));

    const SkMatrix scale = SkMatrix::Scale(1.0f/upem, 1.0f/upem);
    for (SkGlyphID index = 0; index <= 67; ++index) {
        SkScalar width;
        width = 100;

        builder.setGlyph(index, width/upem, SkPath::Circle(50, -50, 75).makeTransform(scale));
    }

    return builder.detach();
}

static sk_sp<SkTypeface> create_font(const char* name, SkFontStyle style) {
    static sk_sp<SkFontMgr> portableFontMgr = MakePortableFontMgr();
    return portableFontMgr->legacyMakeTypeface(name, style);
}

sk_sp<SkTypeface> create_portable_typeface(const char* name, SkFontStyle style) {
    return create_font(name, style);
}
}  // namespace ToolUtils
