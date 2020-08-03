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

Format::Format(sk_sp<FontCollection> fontCollection)
    : fParagraphBuilder(ParagraphStyle(), fontCollection) {
    fStyles.push(FormatStyle());
}

Format& Format::addText(const char* utf8) {
    ensureText();
    fStyles.push(fStyles.top());
    fLastText = utf8;
    return *this;
}

Format& Format::addText(const char* utf8, const FormatStyle& style) {
    ensureText();
    fParagraphBuilder.pushStyle(style.textStyle());
    fParagraphBuilder.addText(utf8);
    fParagraphBuilder.pop();
    return *this;
}

bool Format::ensureText() {
    if (fLastText != nullptr) {
        fParagraphBuilder.pushStyle(fStyles.top());
        fParagraphBuilder.addText(fLastText);
        fParagraphBuilder.pop();
        fLastText = nullptr;
        fStyles.pop();
    }

    return true;
}

Format& Format::pushStyle(const FormatStyle& style) {
    fStyles.push(style);
    fParagraphBuilder.pushStyle(style);
    return *this;
}

Format& Format::pop() {
    fParagraphBuilder.pop();
    fStyles.pop();
    return *this;
}

Format& Format::font(const char* family) {
    fStyles.top().setFontFamilies({ SkString(family) });
    return *this;
}

Format& Format::fonts(std::vector<const char*> families) {
    std::vector<SkString> strFamilies;
    for (auto& ff : families) {
        strFamilies.emplace_back(ff);
    }
     fStyles.top().setFontFamilies(strFamilies);
    return *this;
}

Format& Format::fontSize(SkScalar size) {
    fStyles.top().setFontSize(size);
    return *this;
}

Format& Format::foregroundPaint(SkPaint paint) {
    fStyles.top().setForegroundColor(paint);
    return *this;
}

Format& Format::backgroundPaint(SkPaint paint) {
    fStyles.top().setBackgroundColor(paint);
    return *this;
}

Format& Format::italic() {
    fStyles.top().setFontStyle(SkFontStyle::Italic());
    return *this;
}

Format& Format::bold() {
    fStyles.top().setFontStyle(SkFontStyle::Bold());
    return *this;
}

Format& Format::normal() {
    fStyles.top().setFontStyle(SkFontStyle::Normal());
    return *this;
}

SkSize Format::measure(SkSize desiredSize) {
    this->ensureText();
    fParagraphBuilder.fParagraphStyle.setDesiredHeight(desiredSize.height());
    fParagraph = fParagraphBuilder.Build();
    //auto impl = static_cast<ParagraphImpl*>(fParagraph.get());
    fParagraph->layout(desiredSize.width());
    return { fParagraph->getLongestLine(), fParagraph->getHeight() };
}

void Format::format(SkSize desiredSize) {
    this->ensureText();
    fParagraphBuilder.fParagraphStyle.setDesiredHeight(desiredSize.height());
    fParagraph = fParagraphBuilder.Build();
    //auto impl = static_cast<ParagraphImpl*>(fParagraph.get());
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