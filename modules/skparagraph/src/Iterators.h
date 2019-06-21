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
    FontIterator(SkSpan<const char> utf8, FontResolver* fontResolver)
        : fText(utf8), fCurrentChar(utf8.begin()), fFontResolver(fontResolver) { }

    void consume() override {
      SkASSERT(fCurrentChar < fText.end());
      SkString locale;
      auto found = fFontResolver->findFirst(fCurrentChar, &fFont, &fLineHeight);
      SkASSERT(found);

      // Move until we find the first character that cannot be resolved with the current font
      while (++fCurrentChar != fText.end()) {
          SkFont font;
          SkScalar height;
          SkString locale;
          found = fFontResolver->findNext(fCurrentChar, &font, &height);
          if (found) {
              if (fFont == font && fLineHeight == height) {
                  continue;
              }
              break;
          }
      }
  }

    size_t endOfCurrentRun() const override { return fCurrentChar - fText.begin(); }
    bool atEnd() const override { return fCurrentChar == fText.end(); }
    const SkFont& currentFont() const override { return fFont; }
    SkScalar currentLineHeight() const { return fLineHeight; }

private:

    SkSpan<const char> fText;
    const char* fCurrentChar;
    SkFont fFont;
    SkScalar fLineHeight;
    FontResolver* fFontResolver;
};

class LangIterator final : public SkShaper::LanguageRunIterator {
public:
    LangIterator(SkSpan<const char> utf8, SkSpan<TextBlock> styles, TextStyle defaultStyle)
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

        fCurrentChar = fCurrentStyle->text().end();
        fCurrentLocale = fCurrentStyle->style().getLocale();
        while (++fCurrentStyle != fTextStyles.end()) {
            if (fCurrentStyle->style().getLocale() != fCurrentLocale) {
                break;
            }
            fCurrentChar = fCurrentStyle->text().end();
        }
    }

    size_t endOfCurrentRun() const override { return fCurrentChar - fText.begin(); }
    bool atEnd() const override { return fCurrentChar == fText.end(); }
    const char* currentLanguage() const override { return fCurrentLocale.c_str(); }

private:
    SkSpan<const char> fText;
    SkSpan<TextBlock> fTextStyles;
    const char* fCurrentChar;
    TextBlock* fCurrentStyle;
    SkString fCurrentLocale;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontIterator_DEFINED
