// Copyright 2019 Google LLC.
#ifndef Paragraph_DEFINED
#define Paragraph_DEFINED

#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"

class SkCanvas;

namespace skia {
namespace textlayout {

struct Block {
    Block(size_t start, size_t end, const TextStyle& style)
            : fStart(start), fEnd(end), fStyle(style) {}
    size_t fStart;
    size_t fEnd;
    TextStyle fStyle;
};

class Paragraph {

public:
    Paragraph(ParagraphStyle style, sk_sp<FontCollection> fonts)
            : fFontCollection(std::move(fonts)), fParagraphStyle(std::move(style)) {}

    virtual ~Paragraph() = default;

    SkScalar getMaxWidth() { return fWidth; }

    SkScalar getHeight() { return fHeight; }

    SkScalar getMinIntrinsicWidth() { return fMinIntrinsicWidth; }

    SkScalar getMaxIntrinsicWidth() { return fMaxIntrinsicWidth; }

    SkScalar getAlphabeticBaseline() { return fAlphabeticBaseline; }

    SkScalar getIdeographicBaseline() { return fIdeographicBaseline; }

    virtual bool didExceedMaxLines() = 0;

    virtual void layout(SkScalar width) = 0;

    virtual void paint(SkCanvas* canvas, SkScalar x, SkScalar y) = 0;

    // Returns a vector of bounding boxes that enclose all text between
    // start and end glyph indexes, including start and excluding end
    virtual std::vector<TextBox> getRectsForRange(unsigned start,
                                                  unsigned end,
                                                  RectHeightStyle rectHeightStyle,
                                                  RectWidthStyle rectWidthStyle) = 0;

    // Returns the index of the glyph that corresponds to the provided coordinate,
    // with the top left corner as the origin, and +y direction as down
    virtual PositionWithAffinity getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) = 0;

    // Finds the first and last glyphs that define a word containing
    // the glyph at index offset
    virtual SkRange<size_t> getWordBoundary(unsigned offset) = 0;

    virtual size_t lineNumber() = 0;

protected:
    sk_sp<FontCollection> fFontCollection;
    ParagraphStyle fParagraphStyle;

    // Things for Flutter
    SkScalar fAlphabeticBaseline;
    SkScalar fIdeographicBaseline;
    SkScalar fHeight;
    SkScalar fWidth;
    SkScalar fMaxIntrinsicWidth;
    SkScalar fMinIntrinsicWidth;
    SkScalar fMaxLineWidth;
};
}  // namespace textlayout
}  // namespace skia

#endif  // Paragraph_DEFINED
