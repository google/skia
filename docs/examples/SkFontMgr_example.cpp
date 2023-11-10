// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkFontMgr_example, 1280, 512, true, 0) {
const char* tostr(SkFontStyle::Slant s) {
    switch (s) {
        case SkFontStyle::kUpright_Slant: return "SkFontStyle::kUpright_Slant";
        case SkFontStyle::kItalic_Slant:  return "SkFontStyle::kItalic_Slant";
        case SkFontStyle::kOblique_Slant: return "SkFontStyle::kOblique_Slant";
        default: return "";
    }
}

void draw(SkCanvas* canvas) {
    SkDebugf("sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());\n\n");

    sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());
    for (int i = 0; i < mgr->countFamilies(); ++i) {
        SkString familyName;
        mgr->getFamilyName(i, &familyName);
        sk_sp<SkFontStyleSet> styleSet(mgr->createStyleSet(i));
        int N = styleSet->count();
        for (int j = 0; j < N; ++j) {
            SkFontStyle fontStyle;
            SkString style;
            styleSet->getStyle(j, &fontStyle, &style);
            SkDebugf("SkFont font(mgr->legacyMakeTypeface(\"%s\",\n"
                     "                                    SkFontStyle(%3d, %1d, %-27s));\n",
                     familyName.c_str(), fontStyle.weight(), fontStyle.width(),
                     tostr(fontStyle.slant()));
        }
        SkDebugf("\n");
    }
}
}  // END FIDDLE
