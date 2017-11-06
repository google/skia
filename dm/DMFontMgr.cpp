/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMFontMgr.h"
#include "sk_tool_utils.h"

namespace DM {

    FontStyleSet::FontStyleSet(int familyIndex) : fFamilyIndex(familyIndex) {}

    // Each font family has 4 styles: Normal, Bold, Italic, BoldItalic.
    int FontStyleSet::count() { return 4; }
    void FontStyleSet::getStyle(int index, SkFontStyle* style, SkString* name) {
        switch (index) {
            default:
            case  0: *style = SkFontStyle::Normal(); *name = "Normal"    ; break;
            case  1: *style = SkFontStyle::Bold  (); *name = "Bold"      ; break;
            case  2: *style = SkFontStyle::Italic(); *name = "Italic"    ; break;
            case  3: *style = SkFontStyle::Italic(); *name = "BoldItalic"; break;
        }
    }

    SkTypeface* FontStyleSet::createTypeface(int index) {
        const char* familyName;
        switch (fFamilyIndex) {
            default:
            case  0: familyName = "Toy Liberation Sans" ; break;
            case  1: familyName = "Toy Liberation Serif"; break;
            case  2: familyName = "Toy Liberation Mono" ; break;
        }

        SkFontStyle style;
        switch (index) {
            default:
            case  0: style = SkFontStyle::Normal(); break;
            case  1: style = SkFontStyle::Bold  (); break;
            case  2: style = SkFontStyle::Italic(); break;
            case  3: style = SkFontStyle::Italic(); break;
        }

        return sk_tool_utils::create_portable_typeface(familyName, style).release();
    }


    // We have 3 font families: Sans, Serif, Mono.
    int FontMgr::onCountFamilies() const { return 3; }
    void FontMgr::onGetFamilyName(int index, SkString* familyName) const {
        switch (index) {
            default:
            case  0: *familyName = "Toy Liberation Sans" ; break;
            case  1: *familyName = "Toy Liberation Serif"; break;
            case  2: *familyName = "Toy Liberation Mono" ; break;
        }
    }

    SkFontStyleSet* FontMgr::onCreateStyleSet(int index) const {
        return new FontStyleSet(index);
    }

} // namespace DM
