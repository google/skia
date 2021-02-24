// Copyright 2021 Google LLC.
#ifndef TextLine_DEFINED
#define TextLine_DEFINED

#include "experimental/sktext/include/Types.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"

namespace skia {
namespace text {

class Processor;

class TextMetrics {

public:
  TextMetrics() {
      clean();
  }

  TextMetrics(const SkFont& font) {

      SkFontMetrics metrics;
      font.getMetrics(&metrics);
      fAscent = metrics.fAscent;
      fDescent = metrics.fDescent;
      fLeading = metrics.fLeading;
  }

  TextMetrics(const TextMetrics&) = default;

  void merge(TextMetrics tail) {
      this->fAscent = std::min(this->fAscent, tail.fAscent);
      this->fDescent = std::max(this->fDescent, tail.fDescent);
      this->fLeading = std::max(this->fLeading, tail.fLeading);
  }

  void clean() {
      this->fAscent = 0;
      this->fDescent = 0;
      this->fLeading = 0;
  }

  SkScalar height() const {
      return this->fDescent - this->fAscent + this->fLeading;
  }

SkScalar baseline() const {
      return - this->fAscent + this->fLeading / 2;
  }

private:
    SkScalar fAscent;
    SkScalar fDescent;
    SkScalar fLeading;
};

class GlyphPos {
public:

    GlyphPos() { }
    GlyphPos(size_t runIndex, size_t glyphIndex) : fRunIndex(runIndex), fGlyphIndex(glyphIndex) { }

    bool operator==(const GlyphPos& other) const {
        return this->fRunIndex == other.fRunIndex && this->fGlyphIndex == other.fGlyphIndex;
    }

    size_t fRunIndex;
    size_t fGlyphIndex;
};

class Stretch {
public:

    Stretch() : fGlyphStart(), fGlyphEnd(), fWidth(0), fTextRange(0, 0), fTextMetrics(), fEmpty(true) { }

    Stretch(GlyphPos glyphStart, size_t textIndex, const TextMetrics& metrics)
        : fGlyphStart(glyphStart)
        , fGlyphEnd(glyphStart)
        , fWidth(0.0)
        , fTextRange(textIndex, textIndex)
        , fTextMetrics(metrics)
        , fEmpty(false) { }

    Stretch(Stretch&&) = default;
    Stretch& operator=(Stretch&&) = default;

    bool isEmpty() const {
      return  this->fEmpty/* ||
             (this->fGlyphStart.fRunIndex == this->fGlyphEnd.fRunIndex &&
              this->fGlyphStart.fGlyphIndex == this->fGlyphEnd.fGlyphIndex)*/;
    }

    void clean() {
        this->fEmpty = true;
    }

    void moveTo(Stretch& tail) {

        if (tail.fEmpty) {
            return;
        }

        if (this->fEmpty) {
            if (!tail.fEmpty) {
                this->fGlyphStart = tail.fGlyphStart;
                this->fGlyphEnd = tail.fGlyphEnd;
                this->fWidth = tail.fWidth;
                this->fTextRange = tail.fTextRange;
                this->fTextMetrics = tail.fTextMetrics;
                this->fEmpty = tail.fEmpty;
            }
            tail.clean();
            return;
        }

        SkASSERT(this->fGlyphEnd.fRunIndex != tail.fGlyphStart.fRunIndex ||
                       this->fGlyphEnd.fGlyphIndex == tail.fGlyphStart.fGlyphIndex);
        this->fGlyphEnd = tail.fGlyphEnd;
        this->fWidth += tail.fWidth;
        this->fTextRange.merge(tail.fTextRange);
        this->fTextMetrics.merge(tail.fTextMetrics);
        tail.clean();
    }

    void finish(size_t glyphIndex, size_t textIndex, SkScalar width) {
        this->fTextRange.fEnd = textIndex;
        this->fGlyphEnd.fGlyphIndex = glyphIndex;
        this->fWidth = width;
    }

    GlyphPos fGlyphStart;
    GlyphPos fGlyphEnd;
    SkScalar fWidth;
    Range fTextRange;
    TextMetrics fTextMetrics;
    bool fEmpty;
};

class Line {
public:
    Line(Processor* processor, const Stretch& stretch, const Stretch& spaces);
    ~Line() = default;

private:
    friend class Processor;

    GlyphPos fTextStart;
    GlyphPos fTextEnd ;
    GlyphPos fWhitespacesEnd;
    Range fText;
    Range fWhitespaces;
    SkScalar fTextWidth;
    SkScalar fSpacesWidth;
    TextMetrics fTextMetrics;
    SkSTArray<1, size_t, true> fRunsInVisualOrder;
    Processor* fProcessor;
};

} // namespace text
} // namespace skia
#endif
