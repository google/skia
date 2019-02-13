// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkTypes.h"

#ifdef SK_USING_SKSHAPER

#include "Test.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkFontMgr.h"
#include "SkShaper.h"

DEF_TEST(ShaperSimple, r) {
    static constexpr char kText[] = "HПривет, мир!";
    sk_sp<SkFontMgr> fontManager = SkFontMgr::RefDefault();
    if (!fontManager) {
        ERRORF(r, "assert(SkFontMgr::RefDefault())");
        return;
    }
    SkFont font(fontManager->legacyMakeTypeface("Noto Sans", SkFontStyle()), 18);
    SkShaper shaper(font.refTypefaceOrDefault());
    if (!shaper.good()) {
        ERRORF(r, "assert(shaper.good())");
        return;
    }
    SkTextBlobBuilderRunHandler handler(kText);
    shaper.shape(&handler, font, kText, strlen(kText), true, {0, 0}, 504);
    auto blob = handler.makeBlob();
    REPORTER_ASSERT(r, blob);
}
#endif
