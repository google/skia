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

/**
 *  For test only.
 *  Load font config from given xml files, instead of those from Android system.
 */
SK_API void SkUseTestFontConfigFile(const char* mainconf, const char* fallbackconf,
                                    const char* fontsdir);

/**
 *  For test only.
 *  Returns the information set by SkUseTestFontConfigFile.
 *  TODO: this should be removed once SkFontConfigInterface_android is removed,
 *  and then Chromium should be given a better way to set up it's test environment
 *  than SkUseTestFontConfigFile.
 */
void SkGetTestFontConfiguration(const char** mainconf, const char** fallbackconf,
                                const char** fontsdir);

#endif // #ifdef SK_BUILD_FOR_ANDROID
#endif // #ifndef SkTypeface_android_DEFINED
