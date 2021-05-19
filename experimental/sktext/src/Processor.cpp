// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Processor.h"
#include <stack>
#include "experimental/sktext/src/Decorator.h"
#include "experimental/sktext/src/Formatter.h"
#include "experimental/sktext/src/Shaper.h"
#include "experimental/sktext/src/Wrapper.h"
#include "experimental/sktext/src/Visitor.h"

namespace skia {
namespace text {

// All at once
bool Processor::drawText(std::u16string text, SkCanvas* canvas, SkScalar x, SkScalar y) {

    return drawText(std::move(text), canvas, TextDirection::kLtr, TextAlign::kLeft, SK_ColorBLACK, SK_ColorWHITE, SkString("Roboto"), 14, SkFontStyle::Normal(), x, y);
}

bool Processor::drawText(std::u16string text, SkCanvas* canvas, SkScalar width) {
    return drawText(std::move(text), canvas,
                    TextDirection::kLtr, TextAlign::kLeft, SK_ColorBLACK, SK_ColorWHITE, SkString("Roboto"), 14, SkFontStyle::Normal(),
                    SkSize::Make(width, SK_ScalarInfinity), 0, 0);
}

bool Processor::drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkColor foreground, SkColor background,
                         const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle, SkScalar x, SkScalar y) {
    return drawText(std::move(text), canvas, textDirection, textAlign, foreground, background, fontFamily, fontSize, fontStyle, SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity), x, y);
}

bool Processor::drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkColor foreground, SkColor background,
                         const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle,SkSize reqSize, SkScalar x, SkScalar y) {

    size_t textSize = text.size();
    Processor processor(std::move(text));

    if (!processor.computeCodeUnitProperties()) {
        return false;
    }

    FontBlock fontBlock(fontFamily, fontSize, fontStyle, textSize);
    Shaper shaper(&processor, textDirection, SkSpan<FontBlock>(&fontBlock, 1));
    if (!shaper.process()) {
        return false;
    }

    Wrapper wrapper(&processor, reqSize.fWidth, reqSize.fHeight);
    if (!wrapper.process()) {
        return false;
    }

    Formatter formatter(&processor, textDirection, textAlign);
    if (!formatter.process()) {
        return false;
    }

    SkPaint backgroundPaint; backgroundPaint.setColor(background);
    SkPaint foregroundPaint; foregroundPaint.setColor(foreground);
    DecorBlock decorBlock(&foregroundPaint, &backgroundPaint, textSize);
    Decorator decorator(&processor, SkSpan<DecorBlock>(&decorBlock, 1));
    if (!decorator.process()) {
        return false;
    }

    SkiaVisitor visitor(canvas, SkPoint::Make(x, y));
    visitor.visit(&processor, SkSpan<DecorBlock>(&decorBlock, 1));
    /*
    processor.iterateByVisualOrder(SkSpan<DecorBlock>(&decorBlock, 1),
       [&](SkSize offset, SkScalar baseline, const TextRun* run, TextRange textRange, GlyphRange glyphRange, const DecorBlock& block) {
        SkTextBlobBuilder builder;
        const auto& blobBuffer = builder.allocRunPos(run->fFont, SkToInt(glyphRange.width()));
        sk_careful_memcpy(blobBuffer.glyphs, run->fGlyphs.data() + glyphRange.fStart, glyphRange.width() * sizeof(SkGlyphID));
        sk_careful_memcpy(blobBuffer.points(), run->fPositions.data() + glyphRange.fStart, glyphRange.width() * sizeof(SkPoint));

        offset.fHeight += baseline;
        canvas->drawTextBlob(builder.make(), x + offset.fWidth, y + offset.fHeight, *block.fForegroundColor);
    });
    */
    return true;
}

bool Processor::computeCodeUnitProperties() {

    fCodeUnitProperties.push_back_n(fText.size() + 1, CodeUnitFlags::kNoCodeUnitFlag);

    fUnicode = std::move(SkUnicode::Make());
    if (nullptr == fUnicode) {
        return false;
    }

    // Create utf8 -> utf16 conversion table
    auto text8 = fUnicode->convertUtf16ToUtf8(fText);
    size_t utf16Index = 0;
    fUTF16FromUTF8.push_back_n(text8.size() + 1, utf16Index);
    fUnicode->forEachCodepoint(text8.c_str(), text8.size(),
        [this, &utf16Index](SkUnichar unichar, int32_t start, int32_t end) {
            for (auto i = start; i < end; ++i) {
                fUTF16FromUTF8[i] = utf16Index;
            }
            ++utf16Index;
       });
    fUTF16FromUTF8[text8.size()] = utf16Index;

    // Get white spaces
    fUnicode->forEachCodepoint(fText.c_str(), fText.size(),
       [this](SkUnichar unichar, int32_t start, int32_t end) {
            if (fUnicode->isWhitespace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfWhiteSpace;
                }
            }
       });

    // Get line breaks
    fUnicode->forEachBreak(fText.c_str(), fText.size(), SkUnicode::BreakType::kLines,
                           [&](SkBreakIterator::Position pos, SkBreakIterator::Status status){
                                fCodeUnitProperties[pos] |= (status == (SkBreakIterator::Status)SkUnicode::LineBreakType::kHardLineBreak
                                                               ? CodeUnitFlags::kHardLineBreakBefore
                                                               : CodeUnitFlags::kSoftLineBreakBefore);
                            });

    // Get graphemes
    fUnicode->forEachBreak(fText.c_str(), fText.size(), SkUnicode::BreakType::kGraphemes,
                           [&](SkBreakIterator::Position pos, SkBreakIterator::Status){
                                fCodeUnitProperties[pos]|= CodeUnitFlags::kGraphemeStart;
                            });

    return true;
}

void Processor::markGlyphs() {
    for (auto& run : fRuns) {
        for (auto index : run.fClusters) {
            fCodeUnitProperties[index] |= CodeUnitFlags::kGlyphStart;
        }
    }
}

template<typename Visitor>
void Processor::iterateByVisualOrder(CodeUnitFlags units, Visitor visitor) {
    SkSize offset = SkSize::MakeEmpty();
    for (auto& line : fLines) {
        offset.fWidth = 0;
        for (auto& runIndex : line.fRunsInVisualOrder) {
            auto& run = fRuns[runIndex];

            auto startGlyph = runIndex == line.fTextStart.runIndex() ? line.fTextStart.glyphIndex() : 0;
            auto endGlyph = runIndex == line.fTextEnd.runIndex() ? line.fTextEnd.glyphIndex() : run.fGlyphs.size();

            Range textRange(run.fUtf8Range.begin(), run.fUtf8Range.end());
            Range glyphRange(startGlyph, endGlyph);
            for (auto glyphIndex = startGlyph; glyphIndex <= endGlyph; ++glyphIndex) {
                auto textIndex = run.fClusters[glyphIndex];
                if (glyphIndex < endGlyph && !this->hasProperty(textIndex, units)) {
                    continue;
                }
                textRange.fEnd = textIndex;
                glyphRange.fEnd = glyphIndex;
                visitor(offset, line.fTextMetrics.baseline(), &run, textRange, glyphRange, this->fCodeUnitProperties[textIndex]);
                textRange.fStart = textIndex;
                glyphRange.fStart = glyphIndex;
                offset.fWidth += run.calculateWidth(glyphRange);
            }
        }
        offset.fHeight += line.fTextMetrics.height();
    }
}

template<typename Visitor>
void Processor::iterateByVisualOrder(SkSpan<DecorBlock> decorBlocks, Visitor visitor) {

    // Decor blocks have to be sorted by text cannot intersect but can skip some parts of the text
    // (in which case we use default text style from paragraph style)
    // The edges of the decor blocks don't have to match glyph, grapheme or even unicode code point edges
    // It's out responsibility to adjust them to some reasonable values
    // [a:b) -> [c:d) where
    // c is closest GG cluster edge to a from the left and d is closest GG cluster edge to b from the left

    DecorBlock* currentBlock = &decorBlocks[0];
    size_t currentStart = 0;
    SkSize offset = SkSize::MakeEmpty();
    for (auto& line : fLines) {
        offset.fWidth = 0;
        for (auto& runIndex : line.fRunsInVisualOrder) {
            auto& run = fRuns[runIndex];
            // The run edges are good (aligned to GGC)
            // "ABCdef" -> "defCBA"
            // "AB": red
            // "Cd": green
            // "ef": blue
            // green[d] blue[ef] green [C] red [BA]
            auto startGlyph = runIndex == line.fTextStart.runIndex() ? line.fTextStart.glyphIndex() : 0;
            auto endGlyph = runIndex == line.fTextEnd.runIndex() ? line.fTextEnd.glyphIndex() : run.fGlyphs.size();

            TextRange textRange(run.fClusters[startGlyph], run.fClusters[endGlyph]);
            GlyphRange glyphRange(startGlyph, endGlyph);

            //SkASSERT(currentStart <= textRange.fStart);
            size_t currentEnd = currentStart + currentBlock->fLength;
            for (auto glyphIndex = startGlyph; glyphIndex < endGlyph; ++glyphIndex) {
                auto textIndex = run.fClusters[glyphIndex];
                if (run.leftToRight() && textIndex < currentEnd) {
                    continue;
                } else if (!run.leftToRight() && textIndex > currentStart) {
                    continue;
                }

                textRange.fEnd = textIndex;
                glyphRange.fEnd = glyphIndex;
                SkSize shift = SkSize::Make(offset.fWidth - run.fPositions[startGlyph].fX, offset.fHeight);
                visitor(shift, line.fTextMetrics.baseline(), &run, textRange, glyphRange, *currentBlock);
                textRange.fStart = textIndex;
                glyphRange.fStart = glyphIndex;
                offset.fWidth += run.calculateWidth(glyphRange);
                ++currentBlock;
                SkASSERT(currentBlock <= decorBlocks.end());
            }

            // The last line
            if (endGlyph > startGlyph) {
                if (run.leftToRight()) {
                    textRange.fEnd = run.fClusters[endGlyph];
                } else {
                    textRange.fStart = run.fClusters[endGlyph];
                }
                glyphRange.fEnd = endGlyph;
                SkSize shift = SkSize::Make(offset.fWidth - run.fPositions[startGlyph].fX, offset.fHeight);
                visitor(shift, line.fTextMetrics.baseline(), &run, textRange, glyphRange, *currentBlock);
            }
        }
        offset.fHeight += line.fTextMetrics.height();
    }
}

} // namespace text
} // namespace skia
