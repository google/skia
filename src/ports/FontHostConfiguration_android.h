/* libs/graphics/ports/FontHostConfiguration_android.h
**
** Copyright 2011, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
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
