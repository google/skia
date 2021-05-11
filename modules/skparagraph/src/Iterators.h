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
            , fCurrentChar(utf8.begin())
            , fCurrentStyle(fTextStyles.begin())
            , fCurrentLocale(defaultStyle.getLocale()) {}

    void consume() override {
        SkASSERT(fCurrentChar < fText.end());

        if (fCurrentStyle == fTextStyles.end()) {
            fCurrentChar = fText.end();
            return;
        }

        fCurrentChar = fText.begin() + fCurrentStyle->fRange.end;
        fCurrentLocale = fCurrentStyle->fStyle.getLocale();
        while (++fCurrentStyle != fTextStyles.end() && !fCurrentStyle->fStyle.isPlaceholder()) {
            if (fCurrentStyle->fStyle.getLocale() != fCurrentLocale) {
                break;
            }
            fCurrentChar = fText.begin() + fCurrentStyle->fRange.end;
        }
    }

    size_t endOfCurrentRun() const override { return fCurrentChar - fText.begin(); }
    bool atEnd() const override { return fCurrentChar >= fText.end(); }
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
