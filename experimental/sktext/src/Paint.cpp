// Copyright 2021 Google LLC.
#include "experimental/sktext/src/Paint.h"

using namespace skia_private;

namespace skia {
namespace text {

    bool Paint::drawText(std::u16string text, SkCanvas* canvas, SkScalar x, SkScalar y) {
        SkPaint foregroundPaint(SkColors::kBlack);
        SkPaint backgroundPaint(SkColors::kWhite);
        return drawText(std::move(text), canvas, TextDirection::kLtr, TextAlign::kLeft,
                        std::move(foregroundPaint), std::move(backgroundPaint), SkString("Roboto"),
                        14, SkFontStyle::Normal(), x, y);
    }

    bool Paint::drawText(std::u16string text, SkCanvas* canvas, SkScalar width) {
        SkPaint foregroundPaint(SkColors::kBlack);
        SkPaint backgroundPaint(SkColors::kWhite);
        return drawText(std::move(text), canvas,
                        TextDirection::kLtr, TextAlign::kLeft, std::move(foregroundPaint),
                        std::move(backgroundPaint), SkString("Roboto"), 14, SkFontStyle::Normal(),
                        SkSize::Make(width, SK_ScalarInfinity), 0, 0);
    }

    bool Paint::drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkPaint foreground, SkPaint background,
                         const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle, SkScalar x, SkScalar y) {
        return drawText(std::move(text), canvas, textDirection, textAlign, std::move(foreground),
                        std::move(background), fontFamily, fontSize, fontStyle,
                        SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity), x, y);
    }

    bool Paint::drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkPaint foreground, SkPaint background,
                         const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle,
                         SkSize reqSize, SkScalar x, SkScalar y) {

        size_t textSize = text.size();
        sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>(fontFamily.c_str(), fontSize, fontStyle);
        FontBlock fontBlock(text.size(), fontChain);
        if (fontChain->getTypeface() == nullptr || fontChain->getTypeface().get() == nullptr) {
            return false;
        }

        auto formattedText = Paint::layout(std::move(text), textDirection, textAlign, reqSize, SkSpan<FontBlock>(&fontBlock, 1));
        if (formattedText == nullptr) {
            return false;
        }

        DecoratedBlock decoratedBlock(textSize, std::move(foreground), std::move(background));
        Paint paint;
        paint.paint(canvas, SkPoint::Make(x, y), nullptr, formattedText.get(), SkSpan<DecoratedBlock>(&decoratedBlock, 1));

        return true;
    }

    void Paint::onGlyphRun(const SkFont& font,
                           DirTextRange dirTextRange,
                           SkRect bounds,
                           TextIndex trailingSpaces,
                           size_t glyphCount,
                           const uint16_t glyphs[],
                           const SkPoint positions[],
                           const TextIndex clusters[]) {

        DecoratedBlock decoratedBlock = findDecoratedBlock(dirTextRange);

        SkTextBlobBuilder builder;
        const auto& blobBuffer = builder.allocRunPos(font , SkToInt(glyphCount));
        sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
        sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
        auto blob = builder.make();
        if (!decoratedBlock.backgroundPaint.nothingToDraw()) {
            bounds.offset(fXY.fX, fXY.fY);
            fCanvas->drawRect(bounds, decoratedBlock.backgroundPaint);
        }
        fCanvas->drawTextBlob(blob, fXY.fX, fXY.fY, decoratedBlock.foregroundPaint);
    }

    std::unique_ptr<WrappedText> Paint::layout(std::u16string text,
                                       TextDirection textDirection, TextAlign textAlign,
                                       SkSize reqSize,
                                       SkSpan<FontBlock> fontBlocks) {
        auto unicode = SkUnicode::Make();
        auto unicodeText = std::make_unique<UnicodeText>(std::move(unicode), SkSpan<uint16_t>((uint16_t*)text.data(), text.size()));
        auto fontResolvedText = unicodeText->resolveFonts(fontBlocks);
        auto shapedText = fontResolvedText->shape(unicodeText.get(), TextDirection::kLtr);
        auto wrappedText = shapedText->wrap(unicodeText.get(), reqSize.width(), reqSize.height());
        wrappedText->format(textAlign, textDirection);
        return wrappedText;
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

    void Paint::paint(SkCanvas* canvas, SkPoint xy, UnicodeText* unicodeText, WrappedText* wrappedText, SkSpan<DecoratedBlock> decoratedBlocks) {
        fCanvas = canvas;
        fXY = xy;
        fDecoratedBlocks = decoratedBlocks;

        TArray<size_t> chunks;
        chunks.resize(decoratedBlocks.size());
        size_t index = 0;
        for (size_t i = 0; i < decoratedBlocks.size(); ++i) {
            index += decoratedBlocks[i].charCount;
            chunks[i] = index;
        }

        if (chunks.size() == 1) {
            wrappedText->visit( this);
        } else {
            wrappedText->visit(unicodeText, this, PositionType::kGraphemeCluster, SkSpan<size_t>(chunks.data(), chunks.size()));
        }
    }

} // namespace text
} // namespace skia
