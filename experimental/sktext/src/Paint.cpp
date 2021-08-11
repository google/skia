// Copyright 2021 Google LLC.
#include "experimental/sktext/src/Paint.h"

namespace skia {
namespace text {

bool Paint::drawText(std::u16string text, SkCanvas* canvas, SkScalar x, SkScalar y) {

    return drawText(std::move(text), canvas, TextDirection::kLtr, TextAlign::kLeft, SK_ColorBLACK, SK_ColorWHITE, SkString("Roboto"), 14, SkFontStyle::Normal(), x, y);
}

bool Paint::drawText(std::u16string text, SkCanvas* canvas, SkScalar width) {
    return drawText(std::move(text), canvas,
                    TextDirection::kLtr, TextAlign::kLeft, SK_ColorBLACK, SK_ColorWHITE, SkString("Roboto"), 14, SkFontStyle::Normal(),
                    SkSize::Make(width, SK_ScalarInfinity), 0, 0);
}

bool Paint::drawText(std::u16string text, SkCanvas* canvas,
                     TextDirection textDirection, TextAlign textAlign,
                     SkColor foreground, SkColor background,
                     const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle, SkScalar x, SkScalar y) {
    return drawText(std::move(text), canvas,
                    textDirection, textAlign, foreground, background,
                    fontFamily, fontSize, fontStyle, SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity), x, y);
}

bool Paint::drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkColor foreground, SkColor background,
                         const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle, SkSize reqSize, SkScalar x, SkScalar y) {

    size_t textSize = text.size();
    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>(fontFamily.c_str(), fontSize);
    FontBlock fontBlock(text.size(), fontChain);

    auto formattedText = Paint::layout(std::move(text), textDirection, textAlign, reqSize, SkSpan<FontBlock>(&fontBlock, 1));
    if (formattedText == nullptr) {
        return false;
    }

    SkPaint backgroundPaint; backgroundPaint.setColor(background);
    SkPaint foregroundPaint; foregroundPaint.setColor(foreground);
    DecoratedBlock decoratedBlock(textSize, foregroundPaint, backgroundPaint);
    Paint paint;
    paint.paint(canvas, SkPoint::Make(x, y), formattedText.get(), SkSpan<DecoratedBlock>(&decoratedBlock, 1));

    return true;
}

void Paint::onBeginLine(TextRange lineText, float baselineY, float horizontalOffset) {
    fHorizontalOffset = horizontalOffset;
    fBaselineY = baselineY;
}
void Paint::onEndLine(TextRange, float baselineY) { }
void Paint::onPlaceholder(TextRange, const SkRect& bounds) { }
void Paint::onGlyphRun(SkFont font,
                       TextRange textRange,
                       SkRect boundingRect,
                       int glyphCount,
                       const uint16_t glyphs[],
                       const SkPoint  positions[],
                       const SkPoint offsets[]) {

    DecoratedBlock decoratedBlock = findDecoratedBlock(textRange);

    SkTextBlobBuilder builder;
    const auto& blobBuffer = builder.allocRunPos(font , SkToInt(glyphCount));
    sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
    sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
    auto blob = builder.make();
    boundingRect.offset(fXY.fX + fHorizontalOffset, fXY.fY);
    fCanvas->drawRect(boundingRect, decoratedBlock.backgroundPaint);
    fCanvas->drawTextBlob(blob, fXY.fX + fHorizontalOffset, fXY.fY, decoratedBlock.foregroundPaint);
}

sk_sp<FormattedText> Paint::layout(std::u16string text,
                            TextDirection textDirection, TextAlign textAlign,
                            SkSize reqSize,
                            SkSpan<FontBlock> fontBlocks) {
    auto unicodeText = Text::parse(SkSpan<uint16_t>((uint16_t*)text.data(), text.size()));
    auto shapedText = unicodeText->shape(fontBlocks, textDirection);
    auto wrappedText = shapedText->wrap(reqSize.width(), reqSize.height(), unicodeText->getUnicode());
    auto formattedText = wrappedText->format(textAlign, textDirection);

    return formattedText;
}

DecoratedBlock Paint::findDecoratedBlock(TextRange textRange) {
    TextIndex start = 0;
    for (auto& block : fDecoratedBlocks) {
        if (start + block.charCount <= textRange.fStart) {
            start += block.charCount;
            continue;
        } else if (start >= textRange.fEnd) {
            break;
        }
        return block;
    }
    return DecoratedBlock(0, SkPaint(), SkPaint());
}

void Paint::paint(SkCanvas* canvas, SkPoint xy, FormattedText* formattedText, SkSpan<DecoratedBlock> decoratedBlocks) {
    fCanvas = canvas;
    fXY = xy;
    fDecoratedBlocks = decoratedBlocks;

    SkTArray<size_t> chunks;
    chunks.resize(decoratedBlocks.size());
    size_t index = 0;
    for (size_t i = 0; i < decoratedBlocks.size(); ++i) {
        index += decoratedBlocks[i].charCount;
        chunks[i] = index;
    }

    formattedText->visit(this, SkSpan<size_t>(chunks.data(), chunks.size()));
}

} // namespace text
} // namespace skia
