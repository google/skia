// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Layout.h"
#include "experimental/sktext/include/Paint.h"

namespace skia {
namespace text {


bool Paint::drawText(std::u16string text, SkCanvas* canvas, SkScalar x, SkScalar y) {

    sk_sp<SkTypeface> defaultTypeface(Paint::getDefaultTypeface());
    return drawText(std::move(text), canvas, TextDirection::kLtr, TextAlign::kLeft, SK_ColorBLACK, SK_ColorWHITE, std::move(defaultTypeface), 14, SkFontStyle::Normal(), x, y);
}

bool Paint::drawText(std::u16string text, SkCanvas* canvas, SkScalar width) {
    sk_sp<SkTypeface> defaultTypeface(Paint::getDefaultTypeface());
    return drawText(std::move(text), canvas,
                    TextDirection::kLtr, TextAlign::kLeft, SK_ColorBLACK, SK_ColorWHITE,  std::move(defaultTypeface), 14, SkFontStyle::Normal(),
                    SkSize::Make(width, SK_ScalarInfinity), 0, 0);
}

bool Paint::drawText(std::u16string text, SkCanvas* canvas,
                     TextDirection textDirection, TextAlign textAlign,
                     SkColor foreground, SkColor background,
                     sk_sp<SkTypeface> typeface, SkScalar fontSize, SkFontStyle fontStyle, SkScalar x, SkScalar y) {
    return drawText(std::move(text), canvas,
                    textDirection, textAlign, foreground, background,
                    std::move(typeface), fontSize, fontStyle, SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity), x, y);
}

bool Paint::drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkColor foreground, SkColor background,
                         sk_sp<SkTypeface> typeface, SkScalar fontSize, SkFontStyle fontStyle, SkSize reqSize, SkScalar x, SkScalar y) {

    size_t textSize = text.size();
    std::vector<FontBlock> fontBlocks = {{ std::move(typeface), fontSize, fontStyle, textSize }};
    auto processor = Layout::layout(std::move(text), std::move(fontBlocks), textDirection, textAlign, reqSize);
    if (processor == nullptr) {
        return false;
    }

    SkPaint backgroundPaint; backgroundPaint.setColor(background);
    SkPaint foregroundPaint; foregroundPaint.setColor(foreground);
    DecorBlock decorBlock(&foregroundPaint, &backgroundPaint, textSize);
    Paint paint(processor.get());
    paint.paint(canvas, SkPoint::Make(x, y), SkSpan<DecorBlock>(&decorBlock, 1));

    return true;
}

void Paint::onBeginLine(const LineInfo&) { }
void Paint::onGlyphRun(const RunInfo& runInfo, const DecorBlock& decorBlock) {
    auto size = runInfo.glyphs.size();
    SkTextBlobBuilder builder;
    const auto& blobBuffer = builder.allocRunPos(SkFont(runInfo.typeface, runInfo.size) , SkToInt(size));
    sk_careful_memcpy(blobBuffer.glyphs, runInfo.glyphs.data(), size * sizeof(uint16_t));
    sk_careful_memcpy(blobBuffer.points(), runInfo.positions.data(),size * sizeof(SkPoint));
    fCanvas->drawTextBlob(builder.make(), fXY.fX, fXY.fY, *decorBlock.fForegroundColor);
}
void Paint::onPlaceholderRun(const PlaceholderInfo&) { }
void Paint::onEndLine(const LineInfo&) { }

void Paint::paint(SkCanvas* canvas, SkPoint xy, SkSpan<DecorBlock> decorBlocks) {
    fCanvas = canvas;
    fXY = xy;
    visit(decorBlocks);
}

} // namespace text
} // namespace skia
