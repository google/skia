// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkTypes.h"

#ifdef SK_USING_SKSHAPER

#include "Test.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkFontMgr.h"
#include "SkShaper.h"

DEF_TEST(ShaperSimple, reporter) {
    static constexpr char kText[] = "ꕢꕎꕌ ꔖꔜꕯꕊ (1) ꕉꕜꕮ ꔔꘋ ꖸ ꔰ ꗋꘋ ꕮꕨ ꔔꘋ ꖸ ꕎ ꕉꖸꕊ ꕴꖃi"
        " ꕃꔤꘂ ꗱ, ꕉꖷ ꗪꗡ ꔻꔤ ꗏꗒꗡ ꕎ ꗪ ꕉꖸꕊ ꖏꕎ. ꕉꕡ ꖏ ꗳꕮꕊ ꗏ ꕪ ꗓ ꕉꖷ ꕉꖸ ꕘꕞ ꗪ. ꖏꖷ ꕉꖸꔧ ꖏ ꖸ ꕚ"
        "ꕌꘂ ꗷꔤ ꕞ ꘃꖷ ꘉꔧ ꗠꖻ ꕞ ꖴꘋ ꔳꕩ ꕉꖸ ꗳ.";
    sk_sp<SkFontMgr> fontManager = SkFontMgr::RefDefault();
    if (!fontManager) {
        ERRORF(reporter, "%s: no default font mugger?");
        return;
    }
    SkFont font(fontManager->legacyMakeTypeface("Noto Sans", SkFontStyle()), 18);
    sk_sp<SkTypeface> typeface = font.refTypefaceOrDefault();
    REPORTER_ASSERT(reporter, typeface);
    SkShaper shaper(std::move(typeface));
    REPORTER_ASSERT(reporter, shaper.good());
    SkTextBlobBuilderRunHandler handler(kText);
    shaper.shape(&handler, font, kText, strlen(kText), true, {0, 0}, 504);
    auto blob = handler.makeBlob();
    REPORTER_ASSERT(reporter, blob);
}
#endif
