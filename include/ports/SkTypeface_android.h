/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTypeface_android_DEFINED
#define SkTypeface_android_DEFINED

#include "SkTypeface.h"

#ifdef SK_BUILD_FOR_ANDROID

class SkPaintOptionsAndroid;

/**
 *  Get the family name of the font in the fallback font list containing
 *  the specified character taking into account the provided language. This
 *  function also assumes the only families with the elegant or default variants
 *  will be returned.
 *
 *  @param uni  The unicode character to use for the lookup.
 *  @param lang The null terminated string representing the BCP 47 language
 *              identifier for the preferred language. If there is no unique
 *              fallback chain for that language the system's default language
 *              will be used.
 *  @param name The family name of the font file containing the unicode character
 *              in the preferred language
 *  @return     true if a font is found and false otherwise
 */
SK_API bool SkGetFallbackFamilyNameForChar(SkUnichar uni, const char* lang, SkString* name);

/**
 *  For test only.
 *  Load font config from given xml files, instead of those from Android system.
 */
SK_API void SkUseTestFontConfigFile(const char* mainconf, const char* fallbackconf,
                                    const char* fontsdir);

#endif // #ifdef SK_BUILD_FOR_ANDROID
#endif // #ifndef SkTypeface_android_DEFINED
