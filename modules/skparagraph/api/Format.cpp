// Copyright 2020 Google LLC.
#include "modules/skparagraph/api/Format.h"

namespace skia {
namespace textlayout {

FormatStyle& FormatStyle::font(const char* family) {
    auto families = this->getFontFamilies();
    families.emplace_back(family);
    this->setFontFamilies(families);
    return *this;
}

FormatStyle& FormatStyle::fonts(std::vector<const char*> families) {
    auto strFamilies = this->getFontFamilies();
    for (auto& family : families) {
        strFamilies.emplace_back(family);
    }
    this->setFontFamilies(strFamilies);
    return *this;
}

FormatStyle& FormatStyle::fontSize(SkScalar size) {
    this->setFontSize(size);
    return *this;
}
FormatStyle& FormatStyle::foregroundPaint(SkPaint paint) {
    this->setForegroundColor(std::move(paint));
    return *this;
}

FormatStyle& FormatStyle::backgroundPaint(SkPaint paint) {
    this->setBackgroundColor(std::move(paint));
    return *this;
}

FormatStyle& Format::italic() {
    fLastStyle = fParagraphBuilder.peekStyle();
    fLastStyle.setFontStyle(SkFontStyle::Italic());
    return fLastStyle;
}

Format::Format(sk_sp<FontCollection> fontCollection)
    : fLastStyle()
    , fParagraphBuilder(ParagraphStyle(), fontCollection) { }

Format& Format::addText(const char* utf8) {
    fParagraphBuilder.addText(utf8);
    return *this;
}

Format& Format::addText(const char* utf8, const FormatStyle& style) {
    fParagraphBuilder.pushStyle(style.textStyle());
    fParagraphBuilder.addText(utf8);
    fParagraphBuilder.pop();
    return *this;
}

Format& Format::pushStyle(const FormatStyle& style) {
    fParagraphBuilder.pushStyle(style.textStyle());
    return *this;
}

Format& Format::pop() {
    fParagraphBuilder.pop();
    return *this;
}

void Format::measure(SkSize desiredSize) {
    fParagraph = fParagraphBuilder.Build();
    //auto impl = static_cast<ParagraphImpl*>(fParagraph.get());
    // TODO: Implement desizedSize.height()
    fParagraph->layout(desiredSize.width());
}

void Format::format(SkSize desiredSize) {
    fParagraph = fParagraphBuilder.Build();
    //auto impl = static_cast<ParagraphImpl*>(fParagraph.get());
    // TODO: Implement desizedSize.height()
    fParagraph->layout(desiredSize.width());
}

void Format::paint(SkCanvas* canvas, SkScalar x, SkScalar y) {
    if (fParagraph == nullptr) {
        format(SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity));
    }
    fParagraph->paint(canvas, x, y);
}

SkRect Format::hitTest(SkScalar x, SkScalar y, HitType hitType) {
    if (fParagraph == nullptr) {
        format(SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity));
    }
    TextRange range;
    auto pos = fParagraph->getGlyphPositionAtCoordinate(x, y);
    if (pos.affinity == Affinity::kDownstream) {
        range.start = pos.position;
        range.end = pos.position + 1;
    } else {
        range.start = pos.position - 1;
        range.end = pos.position;
    }
    auto rects = fParagraph->getRectsForRange
        (range.start, range.end, RectHeightStyle::kTight, RectWidthStyle::kTight);
    if (rects.empty()) {
        return SkRect::MakeEmpty();
    } else {
        return rects[0].rect;
    }

}

std::vector<Format::HitMapping> Format::hitTestMapping(HitType hitType) {
    return { };
}
}
}