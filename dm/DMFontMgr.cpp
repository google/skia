/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMFontMgr.h"
#include "sk_tool_utils.h"

namespace DM {

    static constexpr const char* kFamilyNames[] = {
        "Toy Liberation Sans",
        "Toy Liberation Serif",
        "Toy Liberation Mono",
    };

    FontStyleSet::FontStyleSet(int familyIndex) : fFamilyName(kFamilyNames[familyIndex]) {}

    // Each font family has 4 styles: Normal, Bold, Italic, BoldItalic.
    int FontStyleSet::count() { return 4; }
    void FontStyleSet::getStyle(int index, SkFontStyle* style, SkString* name) {
        switch (index) {
            default:
            case  0: *style = SkFontStyle::Normal    (); *name = "Normal"    ; break;
            case  1: *style = SkFontStyle::Bold      (); *name = "Bold"      ; break;
            case  2: *style = SkFontStyle::Italic    (); *name = "Italic"    ; break;
            case  3: *style = SkFontStyle::BoldItalic(); *name = "BoldItalic"; break;
        }
    }

    SkTypeface* FontStyleSet::createTypeface(int index) {
        SkFontStyle style;
        SkString styleName;
        this->getStyle(index, &style, &styleName);

        return this->matchStyle(style);
    }

    SkTypeface* FontStyleSet::matchStyle(const SkFontStyle& style) {
        return sk_tool_utils::create_portable_typeface(fFamilyName, style).release();
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    // We have 3 font families: Sans, Serif, Mono.
    int FontMgr::onCountFamilies() const { return SK_ARRAY_COUNT(kFamilyNames); }
    void FontMgr::onGetFamilyName(int index, SkString* familyName) const {
        *familyName = kFamilyNames[index];
    }

    SkFontStyleSet* FontMgr::onCreateStyleSet(int index) const {
        return new FontStyleSet(index);
    }

} // namespace DM
