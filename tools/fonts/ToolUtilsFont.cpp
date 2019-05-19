/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/ToolUtils.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkMutex.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"
#include "tools/fonts/TestFontMgr.h"

namespace ToolUtils {

sk_sp<SkTypeface> planet_typeface() {
    static const sk_sp<SkTypeface> planetTypeface = []() {
        const char* filename;
#if defined(SK_BUILD_FOR_WIN)
        filename = "fonts/planetcolr.ttf";
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        filename = "fonts/planetsbix.ttf";
#else
        filename = "fonts/planetcbdt.ttf";
#endif
        sk_sp<SkTypeface> typeface = MakeResourceAsTypeface(filename);
        if (typeface) {
            return typeface;
        }
        return SkTypeface::MakeFromName("Planet", SkFontStyle());
    }();
    return planetTypeface;
}

sk_sp<SkTypeface> emoji_typeface() {
    static const sk_sp<SkTypeface> emojiTypeface = []() {
        const char* filename;
#if defined(SK_BUILD_FOR_WIN)
        filename = "fonts/colr.ttf";
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        filename = "fonts/sbix.ttf";
#else
        filename = "fonts/cbdt.ttf";
#endif
        sk_sp<SkTypeface> typeface = MakeResourceAsTypeface(filename);
        if (typeface) {
            return typeface;
        }
        return SkTypeface::MakeFromName("Emoji", SkFontStyle());
    }();
    return emojiTypeface;
}

const char* emoji_sample_text() {
    return "\xF0\x9F\x98\x80"
           " "
           "\xE2\x99\xA2";  // 😀 ♢
}
static sk_sp<SkTypeface> create_font(const char* name, SkFontStyle style) {
    static sk_sp<SkFontMgr> portableFontMgr = MakePortableFontMgr();
    return portableFontMgr->legacyMakeTypeface(name, style);
}

sk_sp<SkTypeface> create_portable_typeface(const char* name, SkFontStyle style) {
    return create_font(name, style);
}
}  // namespace ToolUtils
