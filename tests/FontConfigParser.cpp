/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCommandLineFlags.h"
#include "SkFontConfigParser_android.h"
#include "Test.h"

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

DEF_TEST(FontConfigParserAndroid, reporter) {

    bool resourcesMissing = false;

    SkTDArray<FontFamily*> preV17FontFamilies;
    SkFontConfigParser::GetCustomFontFamilies(preV17FontFamilies,
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


    SkTDArray<FontFamily*> v17FontFamilies;
    SkFontConfigParser::GetCustomFontFamilies(v17FontFamilies,
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


    SkTDArray<FontFamily*> v22FontFamilies;
    SkFontConfigParser::GetCustomFontFamilies(v22FontFamilies,
        SkString("/custom/font/path/"),
        GetResourcePath("android_fonts/v22/fonts.xml").c_str(),
        NULL);

    if (v22FontFamilies.count() > 0) {
        REPORTER_ASSERT(reporter, v22FontFamilies.count() == 53);
        REPORTER_ASSERT(reporter, CountFallbacks(v22FontFamilies) == 42);

        DumpLoadedFonts(v22FontFamilies, "version 22");
        ValidateLoadedFonts(v22FontFamilies, "Roboto-Thin.ttf", reporter);
    } else {
        resourcesMissing = true;
    }

    if (resourcesMissing) {
        SkDebugf("---- Resource files missing for FontConfigParser test\n");
    }
}

