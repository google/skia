// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkFontMgr_example, 1280, 512, true, 0) {

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
            SkDebugf("SkFont font(mgr->legacyMakeTypeface(\"%s\", SkFontStyle(%f, %f, %f, %f, %s));\n",
                     familyName.c_str(),fontStyle.weight(), fontStyle.stretch(), fontStyle.angle(),
                     fontStyle.italic(), fontStyle.isFixedPitch() ? "true" : "false");
        }
        SkDebugf("\n");
    }
}
}  // END FIDDLE
