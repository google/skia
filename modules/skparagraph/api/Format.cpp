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

SkRect Format::hitTest(SkScalar x, SkScalar y) {
    if (fParagraph == nullptr) {
        format(SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity));
    }
    auto impl = static_cast<ParagraphImpl*>(fParagraph.get());
    SkRect result = SkRect::MakeEmpty();
    for (auto& line : impl->fLines) {
        auto offsetY = line.offset().fY;
        if (y >= offsetY + line.height() && &line != &impl->fLines.back()) {
            // This line is not good enough
            continue;
        }
        // Found the line
        line.iterateThroughVisualRuns(true,
            [&]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                bool keepLooking = true;
                *runWidthInLine = line.iterateThroughSingleRunByStyles(
                    run, runOffsetInLine, textRange, StyleType::kNone,
                    [&](TextRange textRange,
                         const TextStyle& style,
                         const TextLine::ClipContext& context) {

                        SkScalar offsetX = line.offset().fX;
                        if (x < context.clip.fLeft + offsetX) {
                            return keepLooking = false;
                        } else if (x >= context.clip.fRight + offsetX) {
                            return keepLooking = true;
                        }

                        // TODO: binary search
                        size_t found = context.pos;
                        for (size_t index = context.pos; index < context.pos + context.size; ++index) {
                            auto end = context.run->positionX(index) + context.fTextShift + offsetX;
                            if (end > x) {
                                break;
                            }
                            found = index;
                        }
                        SkScalar glyphemePosLeft = context.run->positionX(found) + context.fTextShift + offsetX;
                        SkScalar glyphemePosWidth = context.run->positionX(found + 1) - context.run->positionX(found);
                        result = SkRect::MakeXYWH(glyphemePosLeft,
                                                  context.clip.fTop,
                                                  glyphemePosWidth,
                                                  context.clip.fBottom);
                        return keepLooking = false;
                    });
                return keepLooking;
            });
        break;
    }
    return result;
}

std::vector<Format::HitMapping> Format::hitTestMapping() {
    std::vector<HitMapping> graphemes;
    auto impl = static_cast<ParagraphImpl*>(fParagraph.get());
    impl->ensureUTF16Mapping();
    HitMapping current;
    current.fText.start = 0;
    for (size_t index = 1; index < impl->fText.size(); ++index) {
        if ((impl->fCodeUnitProperties[index] & CodeUnitFlags::kGraphemeStart) != 0) {
            current.fText.end = index;
            auto result = impl->getRectsForRange(
                impl->getUTF16Index(current.fText.start), impl->getUTF16Index(current.fText.end),
                RectHeightStyle::kTight, RectWidthStyle::kTight);
            if (result.size() == 1) {
                current.fRect = result[0].rect;
                graphemes.emplace_back(current);
            } else if (result.size() > 1) {
                SkASSERT(false);
                graphemes.emplace_back(current);
            } else {
                // TODO: stop if the text has vertical limit
            }
            current.fText.start = index;
            current.fText.end = index;
            current.fRect = SkRect::MakeEmpty();
        }
    }

    current.fText.end = impl->fText.size();
    auto result = impl->getRectsForRange(
        impl->getUTF16Index(current.fText.start), impl->getUTF16Index(current.fText.end),
        RectHeightStyle::kTight, RectWidthStyle::kTight);
    graphemes.emplace_back(current);

    return graphemes;
}
}
}