/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FONTHOSTCONFIGURATION_ANDROID_H_
#define FONTHOSTCONFIGURATION_ANDROID_H_

#include "SkTDArray.h"

/**
 * The FontFamily data structure is created during parsing and handed back to
 * Skia to fold into its representation of font families. fNames is the list of
 * font names that alias to a font family. fFileNames is the list of font
 * filenames for the family. Order is the priority order for the font. This is
 * used internally to determine the order in which to place fallback fonts as
 * they are read from the configuration files.
 */
struct FontFamily {
    SkTDArray<const char*>  fNames;
    SkTDArray<const char*>  fFileNames;
    int order;
};

/**
 * Parses all system font configuration files and returns the results in an
 * array of FontFamily structures.
 */
void getFontFamilies(SkTDArray<FontFamily*> &fontFamilies);

/**
 * Parse the fallback and vendor system font configuration files and return the
 * results in an array of FontFamily structures.
 */
void getFallbackFontFamilies(SkTDArray<FontFamily*> &fallbackFonts);

/**
 * Parses all test font configuration files and returns the results in an
 * array of FontFamily structures.
 */
void getTestFontFamilies(SkTDArray<FontFamily*> &fontFamilies,
                         const char* testMainConfigFile,
                         const char* testFallbackConfigFile);

struct AndroidLocale {
    char language[3];
    char region[3];
};

void getLocale(AndroidLocale &locale);

#endif /* FONTHOSTCONFIGURATION_ANDROID_H_ */
