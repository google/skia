/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/fonts/FontToolUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"  // IWYU pragma: keep
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkMutex.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontMgrPriv.h"
#include "src/core/SkOSFile.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/fonts/TestFontMgr.h"

#if defined(SK_BUILD_FOR_WIN) && (defined(SK_FONTMGR_GDI_AVAILABLE) || \
                                  defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE))
#include "include/ports/SkTypeface_win.h"
#endif

#if defined(SK_BUILD_FOR_ANDROID) && defined(SK_FONTMGR_ANDROID_AVAILABLE)
#include "include/ports/SkFontMgr_android.h"
#endif

#if defined(SK_FONTMGR_CORETEXT_AVAILABLE) && (defined(SK_BUILD_FOR_IOS) || \
                                               defined(SK_BUILD_FOR_MAC))
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif

#if defined(SK_FONTMGR_FREETYPE_DIRECTORY_AVAILABLE)
#include "include/ports/SkFontMgr_directory.h"
#endif

#if defined(SK_FONTMGR_FREETYPE_EMPTY_AVAILABLE)
#include "include/ports/SkFontMgr_empty.h"
#endif

namespace ToolUtils {

static DEFINE_bool(nativeFonts,
                   true,
                   "If true, use native font manager and rendering. "
                   "If false, fonts will draw as portably as possible.");
#if defined(SK_BUILD_FOR_WIN)
static DEFINE_bool(gdi, false, "Use GDI instead of DirectWrite for font rendering.");
#endif

sk_sp<SkTypeface> PlanetTypeface() {
    static const sk_sp<SkTypeface> planetTypeface = []() {
        const char* filename;
#if defined(SK_BUILD_FOR_WIN)
        filename = "fonts/planetcolr.ttf";
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        filename = "fonts/planetsbix.ttf";
#else
        filename = "fonts/planetcbdt.ttf";
#endif
        sk_sp<SkTypeface> typeface = CreateTypefaceFromResource(filename);
        if (typeface) {
            return typeface;
        }
        return CreateTestTypeface("Planet", SkFontStyle());
    }();
    return planetTypeface;
}

sk_sp<SkTypeface> EmojiTypeface() {
    static const sk_sp<SkTypeface> emojiTypeface = []() {
        const char* filename;
#if defined(SK_BUILD_FOR_WIN)
        filename = "fonts/colr.ttf";
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        filename = "fonts/sbix.ttf";
#else
        filename = "fonts/cbdt.ttf";
#endif
        sk_sp<SkTypeface> typeface = CreateTypefaceFromResource(filename);
        if (typeface) {
            return typeface;
        }
        return CreateTestTypeface("Emoji", SkFontStyle());
    }();
    return emojiTypeface;
}

sk_sp<SkTypeface> SampleUserTypeface() {
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

sk_sp<SkTypeface> CreatePortableTypeface(const char* name, SkFontStyle style) {
    static sk_sp<SkFontMgr> portableFontMgr = MakePortableFontMgr();
    SkASSERT_RELEASE(portableFontMgr);
    sk_sp<SkTypeface> face = portableFontMgr->legacyMakeTypeface(name, style);
    SkASSERT_RELEASE(face);
    return face;
}

sk_sp<SkTypeface> DefaultPortableTypeface() {
    // At last check, the default typeface is a serif font.
    sk_sp<SkTypeface> face = CreatePortableTypeface(nullptr, SkFontStyle());
    SkASSERT_RELEASE(face);
    return face;
}

SkFont DefaultPortableFont() {
    return SkFont(DefaultPortableTypeface(), 12);
}

SkBitmap CreateStringBitmap(int w, int h, SkColor c, int x, int y, int textSize,
                            const char* str) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(w, h);
    SkCanvas canvas(bitmap);

    SkPaint paint;
    paint.setColor(c);

    SkFont font(DefaultPortableTypeface(), textSize);

    canvas.clear(0x00000000);
    canvas.drawSimpleText(str,
                          strlen(str),
                          SkTextEncoding::kUTF8,
                          SkIntToScalar(x),
                          SkIntToScalar(y),
                          font,
                          paint);

    // Tag data as sRGB (without doing any color space conversion). Color-space aware configs
    // will process this correctly but legacy configs will render as if this returned N32.
    SkBitmap result;
    result.setInfo(SkImageInfo::MakeS32(w, h, kPremul_SkAlphaType));
    result.setPixelRef(sk_ref_sp(bitmap.pixelRef()), 0, 0);
    return result;
}

sk_sp<SkImage> CreateStringImage(int w, int h, SkColor c, int x, int y, int textSize,
                                 const char* str) {
    return CreateStringBitmap(w, h, c, x, y, textSize, str).asImage();
}

#ifndef SK_FONT_FILE_PREFIX
#  if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#    define SK_FONT_FILE_PREFIX "/System/Library/Fonts/"
#  else
#    define SK_FONT_FILE_PREFIX "/usr/share/fonts/"
#  endif
#endif

sk_sp<SkFontMgr> TestFontMgr() {
    static sk_sp<SkFontMgr> mgr;
    static SkOnce once;
    once([] {
        if (!FLAGS_nativeFonts) {
            mgr = MakePortableFontMgr();
        }
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_GDI_AVAILABLE)
        else if (FLAGS_gdi) {
            mgr = SkFontMgr_New_GDI();
        }
#endif
        else {
#if defined(SK_BUILD_FOR_ANDROID) && defined(SK_FONTMGR_ANDROID_AVAILABLE)
            mgr = SkFontMgr_New_Android(nullptr);
#elif defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
            mgr = SkFontMgr_New_DirectWrite();
#elif defined(SK_FONTMGR_CORETEXT_AVAILABLE) && (defined(SK_BUILD_FOR_IOS) || \
                                                defined(SK_BUILD_FOR_MAC))
            mgr = SkFontMgr_New_CoreText(nullptr);
#elif defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
            mgr = SkFontMgr_New_FontConfig(nullptr);
#elif defined(SK_FONTMGR_FREETYPE_DIRECTORY_AVAILABLE)
            // In particular, this is used on ChromeOS, which is Linux-like but doesn't have
            // FontConfig.
            mgr = SkFontMgr_New_Custom_Directory(SK_FONT_FILE_PREFIX);
#elif defined(SK_FONTMGR_FREETYPE_EMPTY_AVAILABLE)
            mgr = SkFontMgr_New_Custom_Empty();
#else
            mgr = SkFontMgr::RefEmpty();
#endif
        }
        SkASSERT_RELEASE(mgr);
    });
    return mgr;
}

bool FontMgrIsGDI() {
    if (!FLAGS_nativeFonts) {
        return false;
    }
#if defined(SK_BUILD_FOR_WIN)
    if (FLAGS_gdi) {
        return true;
    }
#endif
    return false;
}

void UsePortableFontMgr() { FLAGS_nativeFonts = false; }

sk_sp<SkTypeface> DefaultTypeface() {
    return CreateTestTypeface(nullptr, SkFontStyle());
}

sk_sp<SkTypeface> CreateTestTypeface(const char* name, SkFontStyle style) {
    sk_sp<SkFontMgr> fm = TestFontMgr();
    SkASSERT_RELEASE(fm);
    sk_sp<SkTypeface> face = fm->legacyMakeTypeface(name, style);
    if (face) {
        return face;
    }
    return CreatePortableTypeface(name, style);
}

sk_sp<SkTypeface> CreateTypefaceFromResource(const char* resource, int ttcIndex) {
    sk_sp<SkFontMgr> fm = TestFontMgr();
    SkASSERT_RELEASE(fm);
    return fm->makeFromStream(GetResourceAsStream(resource), ttcIndex);
}

SkFont DefaultFont() {
    return SkFont(DefaultTypeface(), 12);
}

#if !defined(SK_DISABLE_LEGACY_FONTMGR_FACTORY)
void SetDefaultFontMgr() {
    if (!FLAGS_nativeFonts) {
        gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;
    }
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_GDI_AVAILABLE)
    if (FLAGS_gdi) {
        gSkFontMgr_DefaultFactory = &SkFontMgr_New_GDI;
    }
#endif
}
#endif

}  // namespace ToolUtils
