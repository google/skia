/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkFontMgr_android.h"
#include "include/private/SkFixed.h"
#include "src/core/SkOSFile.h"
#include "src/ports/SkFontMgr_android_parser.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

#include <cmath>
#include <cstdio>

DECLARE_bool(verboseFontMgr);

int CountFallbacks(SkTDArray<FontFamily*> fontFamilies) {
    int countOfFallbackFonts = 0;
    for (int i = 0; i < fontFamilies.count(); i++) {
        if (fontFamilies[i]->fIsFallbackFont) {
            countOfFallbackFonts++;
        }
    }
    return countOfFallbackFonts;
}

//https://tools.ietf.org/html/rfc5234#appendix-B.1
static bool isALPHA(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

//https://tools.ietf.org/html/rfc5234#appendix-B.1
static bool isDIGIT(int c) {
    return ('0' <= c && c <= '9');
}

static void ValidateLoadedFonts(SkTDArray<FontFamily*> fontFamilies, const char* firstExpectedFile,
                                skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, fontFamilies[0]->fNames.count() == 5);
    REPORTER_ASSERT(reporter, !strcmp(fontFamilies[0]->fNames[0].c_str(), "sans-serif"));
    REPORTER_ASSERT(reporter,
                    !strcmp(fontFamilies[0]->fFonts[0].fFileName.c_str(), firstExpectedFile));
    REPORTER_ASSERT(reporter, !fontFamilies[0]->fIsFallbackFont);

    // Check that the languages are all sane.
    for (const auto& fontFamily : fontFamilies) {
        for (const auto& lang : fontFamily->fLanguages) {
            const SkString& langString = lang.getTag();
            for (size_t i = 0; i < langString.size(); ++i) {
                int c = langString[i];
                REPORTER_ASSERT(reporter, isALPHA(c) || isDIGIT(c) || '-' == c);
            }
        }
    }

    // All file names in the test configuration files start with a capital letter.
    // This is not a general requirement, but it is true of all the test configuration data.
    // Verifying ensures the filenames have been read sanely and have not been 'sliced'.
    for (int i = 0; i < fontFamilies.count(); ++i) {
        FontFamily& family = *fontFamilies[i];
        for (int j = 0; j < family.fFonts.count(); ++j) {
            FontFileInfo& file = family.fFonts[j];
            REPORTER_ASSERT(reporter, !file.fFileName.isEmpty() &&
                                      file.fFileName[0] >= 'A' &&
                                      file.fFileName[0] <= 'Z');
        }
    }
}

static void DumpFiles(const FontFamily& fontFamily) {
    for (int j = 0; j < fontFamily.fFonts.count(); ++j) {
        const FontFileInfo& ffi = fontFamily.fFonts[j];
        SkDebugf("  file (%d) %s#%d", ffi.fWeight, ffi.fFileName.c_str(), ffi.fIndex);
        for (const auto& coordinate : ffi.fVariationDesignPosition) {
            SkDebugf(" @'%c%c%c%c'=%f",
                        (coordinate.axis >> 24) & 0xFF,
                        (coordinate.axis >> 16) & 0xFF,
                        (coordinate.axis >>  8) & 0xFF,
                        (coordinate.axis      ) & 0xFF,
                        coordinate.value);
        }
        SkDebugf("\n");
    }
}

static void DumpLoadedFonts(SkTDArray<FontFamily*> fontFamilies, const char* label) {
    if (!FLAGS_verboseFontMgr) {
        return;
    }

    SkDebugf("\n--- Dumping %s\n", label);
    for (int i = 0; i < fontFamilies.count(); ++i) {
        SkDebugf("Family %d:\n", i);
        switch(fontFamilies[i]->fVariant) {
            case kElegant_FontVariant: SkDebugf("  elegant\n"); break;
            case kCompact_FontVariant: SkDebugf("  compact\n"); break;
            default: break;
        }
        SkDebugf("  basePath %s\n", fontFamilies[i]->fBasePath.c_str());
        if (!fontFamilies[i]->fLanguages.empty()) {
            SkDebugf("  language");
            for (const auto& lang : fontFamilies[i]->fLanguages) {
                SkDebugf(" %s", lang.getTag().c_str());
            }
            SkDebugf("\n");
        }
        for (int j = 0; j < fontFamilies[i]->fNames.count(); ++j) {
            SkDebugf("  name %s\n", fontFamilies[i]->fNames[j].c_str());
        }
        DumpFiles(*fontFamilies[i]);
        for (const auto& [unused, fallbackFamily] : fontFamilies[i]->fallbackFamilies) {
            SkDebugf("  Fallback for: %s\n", fallbackFamily->fFallbackFor.c_str());
            DumpFiles(*fallbackFamily);
        }
    }
    SkDebugf("\n\n");
}

template <int N, typename T> static double test_parse_fixed_r(skiatest::Reporter* reporter,
                                                              double low, double high, double inc)
{
    double SK_FixedMax_double = nextafter(1 << (sizeof(T) * CHAR_BIT - N - 1), 0.0);
    double SK_FixedEpsilon_double = (1.0 / (1 << N));
    double maxError = 0;
    char buffer[64];
    for (double f = low; f < high; f += inc) {
        SkString s;
        // 'sprintf' formatting as expected depends on the current locale being "C".
        // We currently expect tests and tools to run in the "C" locale.
        sprintf(buffer, "%.20f", f);
        T fix;
        bool b = parse_fixed<N>(buffer, &fix);
        if (b) {
            double f2 = fix * SK_FixedEpsilon_double;
            double error = fabs(f - f2);
            REPORTER_ASSERT(reporter,  error <= SK_FixedEpsilon_double);
            maxError = std::max(maxError, error);
        } else {
            REPORTER_ASSERT(reporter, f < -SK_FixedMax_double || SK_FixedMax_double < f);
        }
    }

    //SkDebugf("maxError: %.20f\n", maxError);
    return maxError;
}

static void test_parse_fixed(skiatest::Reporter* reporter) {
    test_parse_fixed_r<27, int32_t>(reporter, -8.1, -7.9, 0.000001);
    test_parse_fixed_r<27, int32_t>(reporter, -0.1, 0.1, 0.000001);
    test_parse_fixed_r<27, int32_t>(reporter, 7.9, 8.1, 0.000001);
    test_parse_fixed_r<16, int32_t>(reporter, -0.125, 0.125, 1.0 / (1 << 19));
    test_parse_fixed_r<16, int32_t>(reporter, -32768.125, -32766.875, 1.0 / (1 << 17));
    test_parse_fixed_r<16, int32_t>(reporter, 32766.875, 32768.125, 1.0 / (1 << 17));
    test_parse_fixed_r<16, int32_t>(reporter, -1.1, 1.1, 0.0001);

    SkFixed fix;
    REPORTER_ASSERT(reporter, !parse_fixed<27>("-17.1", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>("32768", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>("", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>(".", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>("123.", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>("a", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>(".123a", &fix));
}

DEF_TEST(FontMgrAndroidParser, reporter) {
    test_parse_fixed(reporter);

    bool resourcesMissing = false;

    SkTDArray<FontFamily*> preV17FontFamilies;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(preV17FontFamilies,
        SkString("/custom/font/path/"),
        GetResourcePath("android_fonts/pre_v17/system_fonts.xml").c_str(),
        GetResourcePath("android_fonts/pre_v17/fallback_fonts.xml").c_str());

    if (preV17FontFamilies.count() > 0) {
        REPORTER_ASSERT(reporter, preV17FontFamilies.count() == 14);
        REPORTER_ASSERT(reporter, CountFallbacks(preV17FontFamilies) == 10);

        DumpLoadedFonts(preV17FontFamilies, "pre version 17");
        ValidateLoadedFonts(preV17FontFamilies, "Roboto-Regular.ttf", reporter);
    } else {
        resourcesMissing = true;
    }
    preV17FontFamilies.deleteAll();


    SkTDArray<FontFamily*> v17FontFamilies;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(v17FontFamilies,
        SkString("/custom/font/path/"),
        GetResourcePath("android_fonts/v17/system_fonts.xml").c_str(),
        GetResourcePath("android_fonts/v17/fallback_fonts.xml").c_str(),
        GetResourcePath("android_fonts/v17").c_str());

    if (v17FontFamilies.count() > 0) {
        REPORTER_ASSERT(reporter, v17FontFamilies.count() == 56);
        REPORTER_ASSERT(reporter, CountFallbacks(v17FontFamilies) == 46);

        DumpLoadedFonts(v17FontFamilies, "version 17");
        ValidateLoadedFonts(v17FontFamilies, "Roboto-Regular.ttf", reporter);
    } else {
        resourcesMissing = true;
    }
    v17FontFamilies.deleteAll();


    SkTDArray<FontFamily*> v22FontFamilies;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(v22FontFamilies,
        SkString("/custom/font/path/"),
        GetResourcePath("android_fonts/v22/fonts.xml").c_str(),
        nullptr);

    if (v22FontFamilies.count() > 0) {
        REPORTER_ASSERT(reporter, v22FontFamilies.count() == 54);
        REPORTER_ASSERT(reporter, CountFallbacks(v22FontFamilies) == 42);

        DumpLoadedFonts(v22FontFamilies, "version 22");
        ValidateLoadedFonts(v22FontFamilies, "Roboto-Thin.ttf", reporter);
    } else {
        resourcesMissing = true;
    }
    v22FontFamilies.deleteAll();

    if (resourcesMissing) {
        SkDebugf("---- Resource files missing for FontConfigParser test\n");
    }
}

DEF_TEST(FontMgrAndroidLegacyMakeTypeface, reporter) {
    constexpr char fontsXmlFilename[] = "fonts/fonts.xml";
    SkString basePath = GetResourcePath("fonts/");
    SkString fontsXml = GetResourcePath(fontsXmlFilename);

    if (!sk_exists(fontsXml.c_str())) {
        ERRORF(reporter, "file missing: %s\n", fontsXmlFilename);
        return;
    }

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = fontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> fm(SkFontMgr_New_Android(&custom));
    sk_sp<SkTypeface> t(fm->legacyMakeTypeface("non-existent-font", SkFontStyle()));
    REPORTER_ASSERT(reporter, nullptr == t);
}

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

DEF_TEST(FontMgrAndroidSystemVariableTypeface, reporter) {
    constexpr char fontsXmlFilename[] = "fonts/fonts.xml";
    SkString basePath = GetResourcePath("fonts/");
    SkString fontsXml = GetResourcePath(fontsXmlFilename);

    if (!sk_exists(fontsXml.c_str())) {
        ERRORF(reporter, "file missing: %s\n", fontsXmlFilename);
        return;
    }

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = fontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_Android(&custom));
    // "sans-serif" in "fonts/fonts.xml" is "fonts/Distortable.ttf"
    sk_sp<SkTypeface> typeface(fontMgr->legacyMakeTypeface("sans-serif", SkFontStyle()));

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
    paint.setAntiAlias(true);
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
            position = {coordinates, SK_ARRAY_COUNT(coordinates)};

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

DEF_TEST(FontMgrAndroidSystemFallbackFor, reporter) {
    constexpr char fontsXmlFilename[] = "fonts/fonts.xml";
    SkString basePath = GetResourcePath("fonts/");
    SkString fontsXml = GetResourcePath(fontsXmlFilename);

    if (!sk_exists(fontsXml.c_str())) {
        ERRORF(reporter, "file missing: %s\n", fontsXmlFilename);
        return;
    }

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = fontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_Android(&custom));
    // "sans-serif" in "fonts/fonts.xml" is "fonts/Distortable.ttf", which doesn't have a '!'
    // but "TestTTC" has a bold font which does have '!' and is marked as fallback for "sans-serif"
    // and should take precedence over the same font marked as normal weight next to it.
    sk_sp<SkTypeface> typeface(fontMgr->matchFamilyStyleCharacter(
        "sans-serif", SkFontStyle(), nullptr, 0, '!'));

    REPORTER_ASSERT(reporter, typeface->fontStyle() == SkFontStyle::Bold());
}
