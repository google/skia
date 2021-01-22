// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkFontMgr_example2, 1536, 512, false, 0) {
const char* tostr(SkFontStyle::Slant s) {
    switch (s) {
        case SkFontStyle::kUpright_Slant: return "SkFontStyle::kUpright_Slant";
        case SkFontStyle::kItalic_Slant:  return "SkFontStyle::kItalic_Slant";
        case SkFontStyle::kOblique_Slant: return "SkFontStyle::kOblique_Slant";
        default: return "";
    }
}

void draw(SkCanvas* canvas) {
    float x = 10, y = 10;
    float textScale = 24;

    sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());
    for (int i = 0; i < mgr->countFamilies(); ++i) {
        SkString familyName;
        mgr->getFamilyName(i, &familyName);
        sk_sp<SkFontStyleSet> styleSet(mgr->createStyleSet(i));
        for (int j = 0; j < styleSet->count(); ++j) {
            SkFontStyle fontStyle;
            SkString style;
            styleSet->getStyle(j, &fontStyle, &style);
            auto s = SkStringPrintf(
                    "SkFont font(mgr->legacyMakeTypeface(\"%s\", SkFontStyle(%3d, %1d, %-27s), "
                    "%g);",
                    familyName.c_str(), fontStyle.weight(), fontStyle.width(),
                    tostr(fontStyle.slant()), textScale);
            SkFont font(mgr->legacyMakeTypeface(familyName.c_str(), fontStyle), textScale);
            y += font.getSpacing() * 1.5;
            canvas->drawString(s, x, y, font, SkPaint());
        }
    }
}
}  // END FIDDLE
