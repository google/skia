// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkFontMgr_example2, 1536, 512, false, 0) {

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
                "SkFont font(mgr->legacyMakeTypeface(\"%s\", SkFontStyle(%f, %f, %f, %f, %s), %g);",
                familyName.c_str(), fontStyle.weight(), fontStyle.stretch(), fontStyle.angle(),
                fontStyle.italic(), fontStyle.isFixedPitch() ? "true" : "false", textScale);
            SkFont font(mgr->legacyMakeTypeface(familyName.c_str(), fontStyle), textScale);
            y += font.getSpacing() * 1.5;
            canvas->drawString(s, x, y, font, SkPaint());
        }
    }
}
}  // END FIDDLE
