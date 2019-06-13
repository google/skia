// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/FontIterator.h"
#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPictureRecorder.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

FontIterator::FontIterator(
        SkSpan<const char> utf8, SkSpan<TextBlock> styles, sk_sp<FontCollection> fonts)
        : fText(utf8)
        , fStyles(styles)
        , fCurrentChar(utf8.begin())
        , fFontCollection(std::move(fonts)) {
    findAllFontsForAllStyledBlocks();
}

void FontIterator::consume() {
    SkASSERT(fCurrentChar < fText.end());
    auto found = fFontCollection->findFontForCodepoint(fCurrentChar, true);
    SkASSERT(found != nullptr);
    fFont = found->first;
    fLineHeight = found->second;

    // Move until we find the first character that cannot be resolved with the current font
    while (++fCurrentChar != fText.end()) {
        found = fFontCollection->findFontForCodepoint(fCurrentChar, false);
        if (found != nullptr) {
            if (fFont == found->first && fLineHeight == found->second) {
                continue;
            }
            break;
        }
    }
}

void FontIterator::findAllFontsForAllStyledBlocks() {
    TextBlock combined;
    for (auto& block : fStyles) {
        SkASSERT(combined.text().begin() == nullptr ||
                 combined.text().end() == block.text().begin());

        if (combined.text().begin() != nullptr &&
            block.style().matchOneAttribute(StyleType::kFont, combined.style())) {
            combined.add(block.text());
            continue;
        }

        if (!combined.text().empty()) {
            fFontCollection->resolveFonts(combined.style(), combined.text());
        }

        combined = block;
    }
    fFontCollection->resolveFonts(combined.style(), combined.text());
}

}  // namespace textlayout
}  // namespace skia
