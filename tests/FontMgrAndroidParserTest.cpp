/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCommandLineFlags.h"
#include "SkFontMgr_android_parser.h"
#include "Test.h"

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

void ValidateLoadedFonts(SkTDArray<FontFamily*> fontFamilies, const char* firstExpectedFile,
                         skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, fontFamilies[0]->fNames.count() == 5);
    REPORTER_ASSERT(reporter, !strcmp(fontFamilies[0]->fNames[0].c_str(), "sans-serif"));
    REPORTER_ASSERT(reporter,
                    !strcmp(fontFamilies[0]->fFonts[0].fFileName.c_str(), firstExpectedFile));
    REPORTER_ASSERT(reporter, !fontFamilies[0]->fIsFallbackFont);

    // Check that the languages are all sane.
    for (int i = 0; i < fontFamilies.count(); ++i) {
        const SkString& lang = fontFamilies[i]->fLanguage.getTag();
        for (size_t j = 0; j < lang.size(); ++j) {
            int c = lang[j];
            REPORTER_ASSERT(reporter, isALPHA(c) || isDIGIT(c) || '-' == c);
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

void DumpLoadedFonts(SkTDArray<FontFamily*> fontFamilies, const char* label) {
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
        if (!fontFamilies[i]->fLanguage.getTag().isEmpty()) {
            SkDebugf("  language %s\n", fontFamilies[i]->fLanguage.getTag().c_str());
        }
        for (int j = 0; j < fontFamilies[i]->fNames.count(); ++j) {
            SkDebugf("  name %s\n", fontFamilies[i]->fNames[j].c_str());
        }
        for (int j = 0; j < fontFamilies[i]->fFonts.count(); ++j) {
            const FontFileInfo& ffi = fontFamilies[i]->fFonts[j];
            SkDebugf("  file (%d) %s#%d\n", ffi.fWeight, ffi.fFileName.c_str(), ffi.fIndex);
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
            maxError = SkTMax(maxError, error);
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
        NULL);

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

