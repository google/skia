/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkFontMgr.h"
#include "SkMutex.h"
#include "SkOSFile.h"
#include "SkTestScalerContext.h"
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
            return sk_sp<SkTypeface>(fm->legacyCreateTypeface(name, style));
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
            SkDEBUGCODE(font->fDebugName = sub->fName);
            SkDEBUGCODE(font->fDebugStyle = sub->fStyle);
            fontData->fCachedFont = font;
        }
    }
    return sk_make_sp<SkTestTypeface>(std::move(font), style);
}

}
