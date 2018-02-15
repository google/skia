/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCommonFlags.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkMutex.h"
#include "SkOSFile.h"
#include "SkTestTypeface.h"
#include "SkUtils.h"
#include "sk_tool_utils.h"

namespace sk_tool_utils {

#include "test_font_monospace.inc"
#include "test_font_sans_serif.inc"
#include "test_font_serif.inc"
#include "test_font_index.inc"

void release_portable_typefaces() {
    for (int index = 0; index < gTestFontsCount; ++index) {
        SkTestFontData& fontData = gTestFonts[index];
        fontData.fCachedFont.reset();
    }
}

SK_DECLARE_STATIC_MUTEX(gTestFontMutex);

sk_sp<SkTypeface> create_font(const char* name, SkFontStyle style) {
    SkTestFontData* fontData = nullptr;
    const SubFont* sub;
    if (name) {
        for (int index = 0; index < gSubFontsCount; ++index) {
            sub = &gSubFonts[index];
            if (!strcmp(name, sub->fName) && sub->fStyle == style) {
                fontData = &sub->fFont;
                break;
            }
        }
        if (!fontData) {
            // Once all legacy callers to portable fonts are converted, replace this with
            // SK_ABORT();
            SkDebugf("missing %s weight %d, width %d, slant %d\n",
                     name, style.weight(), style.width(), style.slant());
            // If we called SkTypeface::CreateFromName() here we'd recurse infinitely,
            // so we reimplement its core logic here inline without the recursive aspect.
            sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
            return fm->legacyMakeTypeface(name, style);
        }
    } else {
        sub = &gSubFonts[gDefaultFontIndex];
        fontData = &sub->fFont;
    }
    sk_sp<SkTestFont> font;
    {
        SkAutoMutexAcquire ac(gTestFontMutex);
        if (fontData->fCachedFont) {
            font = fontData->fCachedFont;
        } else {
            font = sk_make_sp<SkTestFont>(*fontData);
            fontData->fCachedFont = font;
        }
    }
    return sk_make_sp<SkTestTypeface>(std::move(font), style);
}

sk_sp<SkTypeface> emoji_typeface() {
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
}

const char* emoji_sample_text() {
    return "\xF0\x9F\x98\x80"; // 😀
#if 0
    return "\xF0\x9F\x92\xB0" "\xF0\x9F\x8F\xA1" "\xF0\x9F\x8E\x85"  // 💰🏡🎅
           "\xF0\x9F\x8D\xAA" "\xF0\x9F\x8D\x95" "\xF0\x9F\x9A\x80"  // 🍪🍕🚀
           "\xF0\x9F\x9A\xBB" "\xF0\x9F\x92\xA9" "\xF0\x9F\x93\xB7"  // 🚻💩📷
           "\xF0\x9F\x93\xA6"                                        // 📦
           "\xF0\x9F\x87\xBA" "\xF0\x9F\x87\xB8" "\xF0\x9F\x87\xA6"; // 🇺🇸🇦
#endif
}

static const char* platform_os_name() {
    for (int index = 0; index < FLAGS_key.count(); index += 2) {
        if (!strcmp("os", FLAGS_key[index])) {
            return FLAGS_key[index + 1];
        }
    }
    return "";
}

static bool extra_config_contains(const char* substring) {
    for (int index = 0; index < FLAGS_key.count(); index += 2) {
        if (0 == strcmp("extra_config", FLAGS_key[index])
                && strstr(FLAGS_key[index + 1], substring)) {
            return true;
        }
    }
    return false;
}

const char* platform_font_manager() {
    if (extra_config_contains("GDI")) {
        return "GDI";
    }
    if (extra_config_contains("NativeFonts")){
        return platform_os_name();
    }
    return "";
}

sk_sp<SkTypeface> create_portable_typeface(const char* name, SkFontStyle style) {
    return create_font(name, style);
}

void set_portable_typeface(SkPaint* paint, const char* name, SkFontStyle style) {
    paint->setTypeface(create_font(name, style));
}

}
