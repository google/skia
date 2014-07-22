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
    FontFileInfo() : fIndex(0) { }

    SkString              fFileName;
    int                   fIndex;
    SkPaintOptionsAndroid fPaintOptions;
};

/**
 * The FontFamily data structure is created during parsing and handed back to
 * Skia to fold into its representation of font families. fNames is the list of
 * font names that alias to a font family. fontFileArray is the list of information
 * about each file.  Order is the priority order for the font. This is
 * used internally to determine the order in which to place fallback fonts as
 * they are read from the configuration files.
 */
struct FontFamily {
    FontFamily() : fIsFallbackFont(false), order(-1) {}

    SkTArray<SkString> fNames;
    SkTArray<FontFileInfo> fFontFiles;
    bool fIsFallbackFont;
    int order; // only used internally by SkFontConfigParser
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

SkString GetLocale();

} // SkFontConfigParser namespace

#endif /* SKFONTCONFIGPARSER_ANDROID_H_ */
