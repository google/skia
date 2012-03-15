/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTypeface_android_DEFINED
#define SkTypeface_android_DEFINED

#include "SkTypeface.h"

enum FallbackScripts {
    kArabic_FallbackScript,
    kEthiopic_FallbackScript,
    kHebrewRegular_FallbackScript,
    kHebrewBold_FallbackScript,
    kThai_FallbackScript,
    kArmenian_FallbackScript,
    kGeorgian_FallbackScript,
    kDevanagari_FallbackScript,
    kBengali_FallbackScript,
    kTamil_FallbackScript,
    kFallbackScriptNumber
};

#define SkTypeface_ValidScript(s) (s >= 0 && s < kFallbackScriptNumber)

/**
 *  Return a new typeface for a fallback script. If the script is
 *  not valid, or can not map to a font, returns null.
 *  @param  script  The script id.
 *  @return reference to the matching typeface. Caller must call
 *          unref() when they are done.
 */
SK_API SkTypeface* SkCreateTypefaceForScript(FallbackScripts script);

/**
 *  Return the string representation for the fallback script on Android.
 *  If the script is not valid, returns null.
 */
SK_API const char* SkGetFallbackScriptID(FallbackScripts script);

/**
 *  Return the fallback script enum for the ID on Android.
 *  If the ID is not valid, or can not map to a fallback
 *  script, returns kFallbackScriptNumber.
 */
SK_API FallbackScripts SkGetFallbackScriptFromID(const char* id);

#endif
