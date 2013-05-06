/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTypeface_android_DEFINED
#define SkTypeface_android_DEFINED

#include "SkTypeface.h"

/**
 *  Get the family name of the font in the fallback font list containing
 *  the specified chararacter. if no font is found, returns false.
 */
SK_API bool SkGetFallbackFamilyNameForChar(SkUnichar uni, SkString* name);

/**
 *  For test only.
 *  Load font config from given xml files, instead of those from Android system.
 */
SK_API void SkUseTestFontConfigFile(const char* mainconf, const char* fallbackconf,
                                    const char* fontsdir);

/**
 *  Given a "current" fontID, return a ref to the next logical typeface
 *  when searching fonts for a given unicode value. Typically the caller
 *  will query a given font, and if a unicode value is not supported, they
 *  will call this, and if 0 is not returned, will search that font, and so
 *  on. This process must be finite, and when the fonthost sees a
 *  font with no logical successor, it must return NULL.
 *
 *  The original fontID is also provided. This is the initial font that was
 *  stored in the typeface of the caller. It is provided as an aid to choose
 *  the best next logical font. e.g. If the original font was bold or serif,
 *  but the 2nd in the logical chain was plain, then a subsequent call to
 *  get the 3rd can still inspect the original, and try to match its
 *  stylistic attributes.
 */
SkTypeface* SkAndroidNextLogicalTypeface(SkFontID currFontID, SkFontID origFontID);

#endif
