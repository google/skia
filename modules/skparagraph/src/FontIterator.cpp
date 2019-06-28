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
#include "src/utils/SkUTF.h"

// TODO: FontCollection and FontIterator have common functionality
namespace skia {
namespace textlayout {

FontIterator::FontIterator(SkSpan<const char> utf8,
                           SkSpan<TextBlock> styles,
                           sk_sp<FontCollection> fonts)
        : fText(utf8)
        , fStyles(styles)
        , fCurrentChar(utf8.begin())
        , fFontResolver(std::move(fonts), utf8) {
    findAllFontsForAllStyledBlocks();

    if (!fFontResolver.findFirst(fCurrentChar, &fFont, &fLineHeight) || fFont.getTypeface() == nullptr) {
        consume();
    }
}

void FontIterator::consume() {
    SkASSERT(fCurrentChar < fText.end());

    // Move until we find the first character that cannot be resolved with the current font
    while (++fCurrentChar != fText.end()) {
        SkFont font;
        SkScalar height;
        if (fFontResolver.findNext(fCurrentChar, &font, &height)) {
            if (font.getTypeface() != nullptr && fFont != font) {
                break;
            }
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
            fFontResolver.findAllFontsForStyledBlock(combined.style(), combined.text());
        }

        combined = block;
    }
    fFontResolver.findAllFontsForStyledBlock(combined.style(), combined.text());
}

void FontIterator::scanFontsMap(std::function<void(const char* ch, size_t size, SkFont font, SkScalar height)> visitor) {

    SkFont font1;
    SkScalar height1;
    const char* first = fText.begin();
    auto found = fFontResolver.findFirst(first, &font1, &height1);
    SkASSERT(found);

    // Move until we find the first character that cannot be resolved with the current font
    const char* ch = first;
    while (++ch != fText.end()) {
        SkFont font;
        SkScalar height;
        found = fFontResolver.findNext(ch, &font, &height);
        if (found) {
            if (font1 == font && height1 == height) {
                continue;
            }
            visitor(first, ch - first, font1, height1);
            first = ch;
            font1 = font;
            height1 = height;
        }
    }
    visitor(first, ch - first, font1, height1);
}

}  // namespace textlayout
}  // namespace skia
