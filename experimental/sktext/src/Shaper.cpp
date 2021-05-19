// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Layout.h"
#include "experimental/sktext/src/Shaper.h"

namespace skia {
namespace text {

// TODO: SkShaper operates in UTF8 indexes
// TODO: calculate intrinsic sizes
// Shape the text in one line
bool Shaper::process() {

    SkString text8 = fProcessor->fUnicode->convertUtf16ToUtf8(fProcessor->fText);
    for (auto& block : this->fProcessor->fFontBlocks) {

        SkFont font(this->createFont(block));

        SkShaper::TrivialFontRunIterator fontIter(font, text8.size());
        SkShaper::TrivialLanguageRunIterator langIter(text8.c_str(), text8.size());
        std::unique_ptr<SkShaper::BiDiRunIterator> bidiIter(
            SkShaper::MakeSkUnicodeBidiRunIterator(
                fProcessor->fUnicode.get(), text8.c_str(), text8.size(), fProcessor->fDefaultTextDirection == TextDirection::kLtr ? 0 : 1));
        std::unique_ptr<SkShaper::ScriptRunIterator> scriptIter(
            SkShaper::MakeSkUnicodeHbScriptRunIterator(fProcessor->fUnicode.get(), text8.c_str(), text8.size()));
        auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
        if (shaper == nullptr) {
            // For instance, loadICU does not work. We have to stop the process
            return false;
        }

        shaper->shape(text8.c_str(), text8.size(),
                fontIter, *bidiIter, *scriptIter, langIter,
                std::numeric_limits<SkScalar>::max(), this);
    }

    fProcessor->markGlyphs();

    return true;
}

void Shaper::commitRunBuffer(const RunInfo&) {
    fCurrentRun->commit();

    // Convert all utf8 into utf16
    for (size_t i = 0; i < fCurrentRun->fClusters.size(); ++i) {
        auto& element = fCurrentRun->fClusters[i];
        element = fProcessor->fUTF16FromUTF8[element];
    }
    fProcessor->fRuns.emplace_back(std::move(*fCurrentRun));
}

SkFont Shaper::createFont(const FontBlock& block) {

    SkFont font(block.fTypeface, block.fFontSize);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setHinting(SkFontHinting::kSlight);
    font.setSubpixel(true);

    return font;
}

} // namespace text
} // namespace skia
