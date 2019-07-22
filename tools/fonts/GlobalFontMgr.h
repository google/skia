/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GlobalFontMgr_DEFINED
#define GlobalFontMgr_DEFINED

#include "include/core/SkFontMgr.h"

// Provides access to a global SkFontMgr. Utility functions using the global SkFontMgr.

namespace ToolUtils {

/** Calls SetGlobalFontMgr with the result of SkNativeFontMgrFactory(). Call only once near the
    beginning of main(). Not thread-safe.
*/
void SetGlobalNativeFontMgr();

/** Set the value to be returned by GlobalFontMgr(). If null, uses an empty SkFontMgr. Call near the
    beginning of main() to avoid unexpected behavior. Not thead-safe.
*/
void SetGlobalFontMgr(sk_sp<SkFontMgr>);

/** Return the singleton global fontmgr. */
sk_sp<SkFontMgr> GlobalFontMgr();

/** Returns the default normal typeface, which is never nullptr.
    This function caches its return value, so results may be unexpected after calling
    SetGlobalFontMgr.
*/
sk_sp<SkTypeface> DefaultTypeface();

/** Creates a new reference to the typeface that most closely matches the
    requested familyName and fontStyle. This method allows extended font
    face specifiers as in the SkFontStyle type. Will never return null.

    @param familyName  May be NULL. The name of the font family.
    @param fontStyle   The style of the typeface.
    @return reference to the closest-matching typeface.
*/
sk_sp<SkTypeface> TypefaceFromName(const char familyName[], SkFontStyle fontStyle);

}  // namespace ToolUtils

#endif  // GlobalFontMgr_DEFINED
