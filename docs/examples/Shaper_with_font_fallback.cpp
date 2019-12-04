// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Shaper_with_font_fallback, 512, 128, false, 0) {
void draw(SkCanvas* canvas) {
    static constexpr char kText[] =
        "😁 All human beings are born free and equal in dignity and rights. "
        "They are endowed with reason and conscience and should act towards "
        "one another in a spirit of brotherhood. 😁";

    static constexpr int kWidth = 512;
    static constexpr float kMargin = 6;
    static constexpr float kFontSize = 20;

    const size_t textBytes = strlen(kText);

    std::unique_ptr<SkShaper::BiDiRunIterator> bidi(
            SkShaper::MakeBiDiRunIterator(kText, textBytes, 0xfe));
    if (!bidi) {
        return;
    }

    std::unique_ptr<SkShaper::LanguageRunIterator> language(
            SkShaper::MakeStdLanguageRunIterator(kText, textBytes));
    if (!language) {
        return;
    }

    SkFourByteTag undeterminedScript = SkSetFourByteTag('Z','y','y','y');
    std::unique_ptr<SkShaper::ScriptRunIterator> script(
            SkShaper::MakeScriptRunIterator(kText, textBytes, undeterminedScript));
    if (!script) {
        return;
    }

    sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefDefault();
    SkFont srcFont(nullptr, kFontSize);
    srcFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    std::unique_ptr<SkShaper::FontRunIterator> fontItr(
            SkShaper::MakeFontMgrRunIterator(
                kText, textBytes, srcFont, fontMgr,
                "Arial", SkFontStyle::Bold(), &*language));
    if (!fontItr) {
        return;
    }
    SkTextBlobBuilderRunHandler builder(kText, {0, 0});
    std::unique_ptr<SkShaper> shaper = SkShaper::MakeShaperDrivenWrapper();
    shaper->shape(kText, textBytes, *fontItr, *bidi, *script, *language,
                  kWidth - 2 * kMargin, &builder);
    SkPaint paint(SkColors::kBlack);
    canvas->drawTextBlob(builder.makeBlob(), kMargin, kMargin, paint);
}
}  // END FIDDLE
