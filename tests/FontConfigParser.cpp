/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkFontConfigParser_android.h"
#include "Test.h"

void ValidateLoadedFonts(SkTDArray<FontFamily*> fontFamilies,
                         skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, fontFamilies[0]->fNames.count() == 5);
    REPORTER_ASSERT(reporter, !strcmp(fontFamilies[0]->fNames[0].c_str(), "sans-serif"));
    REPORTER_ASSERT(reporter,
                    !strcmp(fontFamilies[0]->fFontFiles[0].fFileName.c_str(),
                            "Roboto-Regular.ttf"));
    REPORTER_ASSERT(reporter, !fontFamilies[0]->fIsFallbackFont);

}

void DumpLoadedFonts(SkTDArray<FontFamily*> fontFamilies) {
#if SK_DEBUG_FONTS
    for (int i = 0; i < fontFamilies.count(); ++i) {
        SkDebugf("Family %d:\n", i);
        switch(fontFamilies[i]->fVariant) {
            case SkPaintOptionsAndroid::kElegant_Variant: SkDebugf("  elegant"); break;
            case SkPaintOptionsAndroid::kCompact_Variant: SkDebugf("  compact"); break;
            default: break;
        }
        if (!fontFamilies[i]->fLanguage.getTag().isEmpty()) {
            SkDebugf("  language: %s", fontFamilies[i]->fLanguage.getTag().c_str());
        }
        for (int j = 0; j < fontFamilies[i]->fNames.count(); ++j) {
            SkDebugf("  name %s\n", fontFamilies[i]->fNames[j].c_str());
        }
        for (int j = 0; j < fontFamilies[i]->fFontFiles.count(); ++j) {
            const FontFileInfo& ffi = fontFamilies[i]->fFontFiles[j];
            SkDebugf("  file (%d %s %d) %s\n",
                     ffi.fWeight,
                     ffi.fPaintOptions.getLanguage().getTag().isEmpty() ? "" :
                         ffi.fPaintOptions.getLanguage().getTag().c_str(),
                     ffi.fPaintOptions.getFontVariant(),
                     ffi.fFileName.c_str());
        }
    }
#endif // SK_DEBUG_FONTS
}

DEF_TEST(FontConfigParserAndroid, reporter) {

    SkTDArray<FontFamily*> preV17FontFamilies;
    SkFontConfigParser::GetTestFontFamilies(preV17FontFamilies,
        GetResourcePath("android_fonts/pre_v17/system_fonts.xml").c_str(),
        GetResourcePath("android_fonts/pre_v17/fallback_fonts.xml").c_str());

    REPORTER_ASSERT(reporter, preV17FontFamilies.count() == 14);

    DumpLoadedFonts(preV17FontFamilies);
    ValidateLoadedFonts(preV17FontFamilies, reporter);


    SkTDArray<FontFamily*> v17FontFamilies;
    SkFontConfigParser::GetTestFontFamilies(v17FontFamilies,
        GetResourcePath("android_fonts/v17/system_fonts.xml").c_str(),
        GetResourcePath("android_fonts/v17/fallback_fonts.xml").c_str());

    REPORTER_ASSERT(reporter, v17FontFamilies.count() == 41);

    DumpLoadedFonts(v17FontFamilies);
    ValidateLoadedFonts(v17FontFamilies, reporter);


    SkTDArray<FontFamily*> v22FontFamilies;
    SkFontConfigParser::GetTestFontFamilies(v22FontFamilies,
        GetResourcePath("android_fonts/v22/fonts.xml").c_str(),
        NULL);

    REPORTER_ASSERT(reporter, v22FontFamilies.count() == 53);

    DumpLoadedFonts(v22FontFamilies);
    ValidateLoadedFonts(v22FontFamilies, reporter);
}

