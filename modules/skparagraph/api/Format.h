// Copyright 2020 Google LLC.
#ifndef Format_DEFINED
#define Format_DEFINED

#include "include/core/SkFontStyle.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

#include <stddef.h>
#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace skia {
namespace textlayout {

class FormatStyle : public TextStyle {
public:
    FormatStyle() = default;
    FormatStyle(const TextStyle textStyle) : TextStyle(textStyle) { }
    FormatStyle& font(const char* family);
    FormatStyle& fonts(std::vector<const char*> families);
    FormatStyle& fontSize(SkScalar size);
    FormatStyle& foregroundPaint(SkPaint paint);
    FormatStyle& backgroundPaint(SkPaint paint);
    FormatStyle& italic();
    const TextStyle& textStyle() const { return *this; }
};

class Format {
public:
    Format(sk_sp<FontCollection> fontCollection);
    // Build text
    Format& addText(const char* utf8);
    Format& addText(const char* utf8, const FormatStyle& style);
    // Build style
    Format& pop();
    Format& pushStyle(const FormatStyle& style);
    Format& setFont(const char* family);
    Format& setFonts(std::vector<const char*> families);
    Format& setFontSize(SkScalar size);
    Format& setForegroundPaint(SkPaint paint);
    Format& setBackgroundPaint(SkPaint paint);
    Format& setItalic();
    FormatStyle& font(const char* family);
    FormatStyle& fonts(std::vector<const char*> families);
    FormatStyle& fontSize(SkScalar size);
    FormatStyle& foregroundPaint(SkPaint paint);
    FormatStyle& backgroundPaint(SkPaint paint);
    FormatStyle& italic();

    FormatStyle defaultStyle() { return fLastStyle; }
    // Measure text
    void measure(SkSize desiredSize);
    // Format text
    void format(SkSize desiredSize);
    // Paint text
    void paint(SkCanvas* canvas, SkScalar x, SkScalar y);
    // Hit test
    enum HitType {
        Glyph,
        GlyphCluster
    };
    SkRect hitTest(SkScalar x, SkScalar y, HitType hitType);
    struct HitMapping {
        TextRange fText;
        SkRect fRect;
    };
    std::vector<HitMapping> hitTestMapping(HitType hitType);
private:
    friend class ParagraphImpl;
    friend class ParagraphBuilderImpl;

    FormatStyle fLastStyle;
    ParagraphBuilderImpl fParagraphBuilder;
    std::unique_ptr<Paragraph> fParagraph;
};
}
}
#endif
