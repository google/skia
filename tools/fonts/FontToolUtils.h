/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FontToolUtils_DEFINED
#define FontToolUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"

class SkBitmap;
class SkImage;
class SkFont;
class SkFontMgr;
class SkTypeface;

namespace ToolUtils {
/**
 * Returns a font that has a non-empty typeface. This could change, so don't depend on things like
 * how it looks, font metrics, etc.
 */
SkFont DefaultPortableFont();

sk_sp<SkTypeface> DefaultPortableTypeface();

/**
 * Returns a platform-independent text renderer.
 */
sk_sp<SkTypeface> CreatePortableTypeface(const char* name, SkFontStyle style);

/* Return a color emoji typeface with planets to scale if available. */
sk_sp<SkTypeface> PlanetTypeface();

/** Return a color emoji typeface if available. */
sk_sp<SkTypeface> EmojiTypeface();

/** Sample text for the emoji_typeface font. */
constexpr const char* EmojiSampleText() {
    return "\xF0\x9F\x98\x80"
           " "
           "\xE2\x99\xA2";  // 😀 ♢
}

/** A simple SkUserTypeface for testing. */
sk_sp<SkTypeface> SampleUserTypeface();

SkBitmap CreateStringBitmap(int w, int h, SkColor c, int x, int y, int textSize, const char* str);
sk_sp<SkImage> CreateStringImage(int w, int h, SkColor c, int x, int y, int textSize, const char* str);

// This returns the SkFontMgr that has been compiled in and configured (e.g. via CLI flag)
sk_sp<SkFontMgr> TestFontMgr();

// Must be called before the first call to TestFontMgr to have any effect.
void UsePortableFontMgr();

// Returns true if this platform is Windows and this binary is being configured to run
// with the GDI font manager.
bool FontMgrIsGDI();

// This returns the default SkTypeface returned by the TestFontMgr(). If there was no default
// Typeface, DefaultPortableTypeface() is returned instead.
sk_sp<SkTypeface> DefaultTypeface();

// Returns a Typeface matching the given criteria as returned by TestFontMgr(). This may be different
// on different platforms.
sk_sp<SkTypeface> CreateTestTypeface(const char* name, SkFontStyle style);

// Load the resource with the provided name as a Typeface using TestFontMgr().
sk_sp<SkTypeface> CreateTypefaceFromResource(const char* resource, int ttcIndex = 0);

// This returns a font using DefaultTypeface()
SkFont DefaultFont();

#if !defined(SK_DISABLE_LEGACY_FONTMGR_FACTORY)
// Temporary hack to set the global for non-migrated uses of SkFontMgr::RefDefault()
void SetDefaultFontMgr();
#endif

}  // namespace ToolUtils

#endif
