// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Processor.h"
#include "experimental/sktext/src/Shaper.h"

namespace skia {
namespace text {

// TODO: calculate intrinsic sizes
// Shape the text in one line
bool Shaper::process() {

    auto text(fProcessor->fText);
    for (auto& block : fProcessor->fFontBlocks) {

        SkFont font(this->createFont(block));

        SkShaper::TrivialFontRunIterator fontIter(font, text.size());
        SkShaper::TrivialLanguageRunIterator langIter(text.c_str(), text.size());
        std::unique_ptr<SkShaper::BiDiRunIterator> bidiIter(
            SkShaper::MakeSkUnicodeBidiRunIterator(
                fProcessor->fUnicode.get(), text.c_str(), text.size(), fDefaultTextDirection == TextDirection::kLtr ? 0 : 1));
        std::unique_ptr<SkShaper::ScriptRunIterator> scriptIter(
            SkShaper::MakeSkUnicodeHbScriptRunIterator(fProcessor->fUnicode.get(), text.c_str(), text.size()));
        auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
        if (shaper == nullptr) {
            // For instance, loadICU does not work. We have to stop the process
            return false;
        }

        shaper->shape(text.c_str(), text.size(),
                fontIter, *bidiIter, *scriptIter, langIter,
                std::numeric_limits<SkScalar>::max(), this);
    }

    return true;
}

void Shaper::commitRunBuffer(const RunInfo&) {
    fCurrentRun->commit();
    fProcessor->fRuns.emplace_back(std::move(*fCurrentRun));
}

SkFont Shaper::createFont(const FontBlock& block) {

    sk_sp<SkTypeface> typeface = matchTypeface(block.fFontFamily, block.fFontStyle);

    SkFont font(std::move(typeface), block.fFontSize);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setHinting(SkFontHinting::kSlight);
    font.setSubpixel(true);

    return font;
}

sk_sp<SkTypeface> Shaper::matchTypeface(const SkString& fontFamily, SkFontStyle fontStyle) {
    sk_sp<SkFontStyleSet> set(fFontManager->matchFamily(fontFamily.c_str()));
    if (!set || set->count() == 0) {
        return nullptr;
    }

    sk_sp<SkTypeface> match(set->matchStyle(fontStyle));
    if (match) {
        return match;
    }

    return nullptr;
}

} // namespace text
} // namespace skia
