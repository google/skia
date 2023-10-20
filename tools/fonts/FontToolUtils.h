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

}  // namespace ToolUtils

#endif
