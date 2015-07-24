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

#include "test_font_monospace.cpp"
#include "test_font_sans_serif.cpp"
#include "test_font_serif.cpp"
#include "test_font_index.cpp"

void release_portable_typefaces() {
    for (int index = 0; index < gTestFontsCount; ++index) {
        SkTestFontData& fontData = gTestFonts[index];
        SkSafeUnref(fontData.fFontCache);
    }
}

SK_DECLARE_STATIC_MUTEX(gTestFontMutex);

SkTypeface* create_font(const char* name, SkTypeface::Style style) {
    SkTestFontData* fontData = NULL;
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
            // SK_CRASH();
            SkDebugf("missing %s %d\n", name, style);
            // If we called SkTypeface::CreateFromName() here we'd recurse infinitely,
            // so we reimplement its core logic here inline without the recursive aspect.
            SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
            return fm->legacyCreateTypeface(name, style);
        }
    } else {
        sub = &gSubFonts[gDefaultFontIndex];
        fontData = &sub->fFont;
    }
    SkTestFont* font;
    {
        SkAutoMutexAcquire ac(gTestFontMutex);
        if (fontData->fFontCache) {
            font = SkSafeRef(fontData->fFontCache);
        } else {
            font = SkNEW_ARGS(SkTestFont, (*fontData));
            SkDEBUGCODE(font->fDebugName = sub->fName);
            SkDEBUGCODE(font->fDebugStyle = sub->fStyle);
            fontData->fFontCache = SkSafeRef(font);
        }
    }
    return SkNEW_ARGS(SkTestTypeface, (font, SkFontStyle(style)));
}

}
