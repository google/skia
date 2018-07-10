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
#include "SkPaint.h"
#include "SkTestFontMgr.h"
#include "SkUtils.h"
#include "sk_tool_utils.h"

namespace sk_tool_utils {

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
    return "\xF0\x9F\x98\x80" " " "\xE2\x99\xA2"; // 😀 ♢
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

static sk_sp<SkTypeface> create_font(const char* name, SkFontStyle style) {
    static sk_sp<SkFontMgr> portableFontMgr = MakePortableFontMgr();
    return portableFontMgr->legacyMakeTypeface(name, style);
}

sk_sp<SkTypeface> create_portable_typeface(const char* name, SkFontStyle style) {
    return create_font(name, style);
}

void set_portable_typeface(SkPaint* paint, const char* name, SkFontStyle style) {
    paint->setTypeface(create_font(name, style));
}

}
