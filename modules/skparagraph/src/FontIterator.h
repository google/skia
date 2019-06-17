// Copyright 2019 Google LLC.
#ifndef FontIterator_DEFINED
#define FontIterator_DEFINED

#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPictureRecorder.h"
#include "modules/skparagraph/src/FontResolver.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkUTF.h"

namespace skia {
namespace textlayout {

class FontIterator final : public SkShaper::FontRunIterator {
public:
    FontIterator(SkSpan<const char> utf8,
                   SkSpan<TextBlock> styles,
                   sk_sp<FontCollection> fonts);

    void consume() override;

    size_t endOfCurrentRun() const override { return fCurrentChar - fText.begin(); }
    bool atEnd() const override { return fCurrentChar == fText.end(); }
    const SkFont& currentFont() const override { return fFont; }
    SkScalar lineHeight() const { return fLineHeight; }

private:
    struct Hash {
        uint32_t operator()(const std::pair<SkFont, SkScalar>& key) const {
            return SkTypeface::UniqueID(key.first.getTypeface()) +
                   SkScalarCeilToInt(key.first.getSize()) + SkScalarCeilToInt(key.second);
        }
    };

    void findAllFontsForAllStyledBlocks();

    SkSpan<const char> fText;
    SkSpan<TextBlock> fStyles;
    const char* fCurrentChar;
    SkFont fFont;
    SkScalar fLineHeight;
    FontResolver fFontResolver;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontIterator_DEFINED
