// Copyright 2019 Google LLC.
#ifndef FontIterator_DEFINED
#define FontIterator_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skshaper/include/SkShaper.h"

namespace skia {
namespace textlayout {

class LangIterator final : public SkShaper::LanguageRunIterator {
public:
    LangIterator(SkSpan<const char> utf8, SkSpan<Block> styles, const TextStyle& defaultStyle)
            : fText(utf8)
            , fTextStyles(styles)
            , fCurrentChar(utf8.data())
            , fCurrentStyle(fTextStyles.data())
            , fCurrentLocale(defaultStyle.getLocale()) {}

    void consume() override {
        const char* textEnd = fText.data() + fText.size();
        const Block* stylesEnd = fTextStyles.data() + fTextStyles.size();

        SkASSERT(fCurrentChar < textEnd);

        if (fCurrentStyle == stylesEnd) {
            fCurrentChar = textEnd;
            return;
        }

        fCurrentChar = fText.data() + fCurrentStyle->fRange.end;
        fCurrentLocale = fCurrentStyle->fStyle.getLocale();
        while (++fCurrentStyle != stylesEnd && !fCurrentStyle->fStyle.isPlaceholder()) {
            if (fCurrentStyle->fStyle.getLocale() != fCurrentLocale) {
                break;
            }
            fCurrentChar = fText.data() + fCurrentStyle->fRange.end;
        }
    }

    size_t endOfCurrentRun() const override { return fCurrentChar - fText.data(); }
    bool atEnd() const override { return fCurrentChar >= fText.data() + fText.size(); }
    const char* currentLanguage() const override { return fCurrentLocale.c_str(); }

private:
    SkSpan<const char> fText;
    SkSpan<Block> fTextStyles;
    const char* fCurrentChar;
    Block* fCurrentStyle;
    SkString fCurrentLocale;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontIterator_DEFINED
