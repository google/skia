/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKFONTCONFIGPARSER_ANDROID_H_
#define SKFONTCONFIGPARSER_ANDROID_H_

#include "SkTypes.h"

#include "SkPaintOptionsAndroid.h"
#include "SkString.h"
#include "SkTDArray.h"

struct FontFileInfo {
    FontFileInfo() : fIndex(0), fWeight(0) { }

    SkString              fFileName;
    int                   fIndex;
    SkPaintOptionsAndroid fPaintOptions;
    int                   fWeight;
};

/**
 * A font family provides one or more names for a collection of fonts, each of
 * which has a different style (normal, italic) or weight (thin, light, bold,
 * etc).
 * Some fonts may occur in compact variants for use in the user interface.
 * Android distinguishes "fallback" fonts to support non-ASCII character sets.
 */
struct FontFamily {
    FontFamily()
        : fVariant(SkPaintOptionsAndroid::kDefault_Variant)
        , fOrder(-1)
        , fIsFallbackFont(false) { }

    SkTArray<SkString>                 fNames;
    SkTArray<FontFileInfo>             fFonts;
    SkLanguage                         fLanguage;
    SkPaintOptionsAndroid::FontVariant fVariant;
    int                                fOrder; // internal to SkFontConfigParser
    bool                               fIsFallbackFont;
};

namespace SkFontConfigParser {

/**
 * Parses all system font configuration files and returns the results in an
 * array of FontFamily structures.
 */
void GetFontFamilies(SkTDArray<FontFamily*> &fontFamilies);

/**
 * Parses all test font configuration files and returns the results in an
 * array of FontFamily structures.
 */
void GetTestFontFamilies(SkTDArray<FontFamily*> &fontFamilies,
                         const char* testMainConfigFile,
                         const char* testFallbackConfigFile);

} // SkFontConfigParser namespace

#endif /* SKFONTCONFIGPARSER_ANDROID_H_ */
