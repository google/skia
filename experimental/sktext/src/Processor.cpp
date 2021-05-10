// Copyright 2021 Google LLC.

#include <stack>
#include "experimental/sktext/include/Processor.h"
#include "experimental/sktext/src/Formatter.h"
#include "experimental/sktext/src/Shaper.h"
#include "experimental/sktext/src/Wrapper.h"

namespace skia {
namespace text {

// The result of shaping is a set of Runs placed on one endless line
// It has all the glyph information
bool Processor::shape(TextFontStyle fontStyle, SkTArray<FontBlock, true> fontBlocks) {

    if (fUnicode == nullptr) {
        return false;
    }

    fFontBlocks = std::move(fontBlocks);

    Shaper shaper(this, fontStyle);
    if (!shaper.process()) {
        return false;
    }

    this->markGlyphs();

    return true;
}

// TODO: we can wrap to any shape, not just a rectangle
// The result of wrapping is a set of Lines that fit the required sizes and
// contain all the glyph information
bool Processor::wrap(SkScalar width, SkScalar height) {

    Wrapper wrapper(this, width, height);
    if (!wrapper.process()) {
        return false;
    }
    return true;
}

// The result of formatting is a possible shift of glyphs as the format type requires
bool Processor::format(TextFormatStyle formatStyle) {

    Formatter formatter(this, formatStyle);
    if (!formatter.process()) {
        return false;
    }
    return true;
}

// Once the text is decorated you can iterate it by segments (intersect of run, decor block and line)
bool Processor::decorate(SkTArray<DecorBlock, true> decorBlocks) {

    this->iterateByVisualOrder(decorBlocks,
       [&](SkSize offset, SkScalar baseline, const TextRun* run, TextRange textRange, GlyphRange glyphRange, const DecorBlock& block) {
        SkTextBlobBuilder builder;
        const auto& blobBuffer = builder.allocRunPos(run->fFont, SkToInt(glyphRange.width()));
        sk_careful_memcpy(blobBuffer.glyphs, run->fGlyphs.data() + glyphRange.fStart, glyphRange.width() * sizeof(SkGlyphID));
        sk_careful_memcpy(blobBuffer.points(), run->fPositions.data() + glyphRange.fStart, glyphRange.width() * sizeof(SkPoint));

        offset.fHeight += baseline;
        fTextOutputs.emplace_back(builder.make(), *block.fForegroundColor, offset);
    });

    return true;
}

// All at once
bool Processor::drawText(std::u16string text, SkCanvas* canvas, SkScalar x, SkScalar y) {

    return drawText(std::move(text), canvas, TextFormatStyle(TextAlign::kLeft, TextDirection::kLtr), SK_ColorBLACK, SK_ColorWHITE, SkString("Roboto"), 14, SkFontStyle::Normal(), x, y);
}

bool Processor::drawText(std::u16string text, SkCanvas* canvas, SkScalar width) {
    return drawText(std::move(text), canvas,
                    TextFormatStyle(TextAlign::kLeft, TextDirection::kLtr), SK_ColorBLACK, SK_ColorWHITE, SkString("Roboto"), 14, SkFontStyle::Normal(),
                    SkSize::Make(width, SK_ScalarInfinity), 0, 0);
}

bool Processor::drawText(std::u16string text, SkCanvas* canvas, TextFormatStyle textFormat, SkColor foreground, SkColor background, const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle, SkScalar x, SkScalar y) {
    return drawText(std::move(text), canvas, textFormat, foreground, background, fontFamily, fontSize, fontStyle, SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity), x, y);
}

bool Processor::drawText(std::u16string text, SkCanvas* canvas,
                         TextFormatStyle textFormat, SkColor foreground, SkColor background, const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle,
                         SkSize reqSize, SkScalar x, SkScalar y) {

    TextRange textRange(0, text.size());
    Processor processor(std::move(text));

    if (!processor.computeCodeUnitProperties()) {
        return false;
    }
    if (!processor.shape({ textFormat.fDefaultTextDirection, SkFontMgr::RefDefault()}, {{{ fontFamily, fontSize, fontStyle, textRange }}})) {
        return false;
    }
    if (!processor.wrap(reqSize.fWidth, reqSize.fHeight)) {
        return false;
    }
    if (!processor.format(textFormat)) {
        return false;
    }
    SkTArray<DecorBlock, true> decor;
    SkPaint backgroundPaint; backgroundPaint.setColor(background);
    SkPaint foregroundPaint; foregroundPaint.setColor(foreground);
    if (!processor.decorate({{{&foregroundPaint, &backgroundPaint, textRange}}})) {
        return false;
    }

    for (auto& output : processor.fTextOutputs) {
        canvas->drawTextBlob(output.fTextBlob, x + output.fOffset.fWidth, y + output.fOffset.fHeight, output.fPaint);
    }

    return true;
}

// Also adjust the decoration block edges to cluster edges while we at it
// to avoid an enormous amount of complications
void Processor::sortDecorBlocks(SkTArray<DecorBlock, true>& decorBlocks) {
    // Soft the blocks
    std::sort(decorBlocks.begin(), decorBlocks.end(),
              [](const DecorBlock& a, const DecorBlock& b) {
                return a.fRange.fStart < b.fRange.fStart;
              });
    // Walk through the blocks using the default when missing
    SkPaint* foreground = new SkPaint();
    foreground->setColor(SK_ColorBLACK);
    std::stack<DecorBlock> defaultBlocks;
    defaultBlocks.emplace(foreground, nullptr, TextRange(0, fText.size()));
    size_t start = 0;
    for (auto& block : decorBlocks) {
        this->adjustLeft(&block.fRange.fStart);
        this->adjustLeft(&block.fRange.fEnd);
        SkASSERT(start <= block.fRange.fStart);
        if (start < block.fRange.fStart) {
            auto defaultBlock = defaultBlocks.top();
            decorBlocks.emplace_back(defaultBlock.fForegroundColor, defaultBlock.fBackgroundColor, Range(start, block.fRange.fStart));
        }
        start = block.fRange.fEnd;
        while (!defaultBlocks.empty()) {
            auto defaultBlock = defaultBlocks.top();
            if (defaultBlock.fRange.fEnd <= block.fRange.fEnd) {
                defaultBlocks.pop();
            }
        }
        defaultBlocks.push(block);
    }
    if (start < fText.size()) {
        auto defaultBlock = defaultBlocks.top();
        decorBlocks.emplace_back(defaultBlock.fForegroundColor, defaultBlock.fBackgroundColor, Range(start, fText.size()));
    }
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
void Processor::iterateByVisualOrder(SkTArray<DecorBlock, true>& decorBlocks, Visitor visitor) {

    this->sortDecorBlocks(decorBlocks);
    // Decor blocks have to be sorted by text cannot intersect but can skip some parts of the text
    // (in which case we use default text style from paragraph style)
    // The edges of the decor blocks don't have to match glyph, grapheme or even unicode code point edges
    // It's out responsibility to adjust them to some reasonable values
    // [a:b) -> [c:d) where
    // c is closest GG cluster edge to a from the left and d is closest GG cluster edge to b from the left

    DecorBlock* currentBlock = &decorBlocks[0];
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

            SkASSERT(currentBlock->fRange.fStart <= textRange.fStart);
            for (auto glyphIndex = startGlyph; glyphIndex <= endGlyph; ++glyphIndex) {
                auto textIndex = run.fClusters[glyphIndex];
                if (run.leftToRight() && textIndex < currentBlock->fRange.fEnd) {
                    continue;
                } else if (!run.leftToRight() && textIndex > currentBlock->fRange.fStart) {
                    continue;
                }

                if (run.leftToRight()) {
                    textRange.fEnd = textIndex;
                } else {
                    textRange.fStart = textIndex;
                }
                glyphRange.fEnd = glyphIndex;
                SkSize shift = SkSize::Make(offset.fWidth - run.fPositions[startGlyph].fX, offset.fHeight);
                visitor(shift, line.fTextMetrics.baseline(), &run, textRange, glyphRange, *currentBlock);
                if (run.leftToRight()) {
                    textRange.fStart = textIndex;
                } else {
                    textRange.fEnd = textIndex;
                }
                glyphRange.fStart = glyphIndex;
                offset.fWidth += run.calculateWidth(glyphRange);
            }

            // The last line
            if (run.leftToRight()) {
                textRange.fEnd = run.fClusters[endGlyph];
            } else {
                textRange.fStart = run.fClusters[endGlyph];
            }
            glyphRange.fEnd = endGlyph;
            SkSize shift = SkSize::Make(offset.fWidth - run.fPositions[startGlyph].fX, offset.fHeight);
            visitor(shift, line.fTextMetrics.baseline(), &run, textRange, glyphRange, *currentBlock);
        }
        offset.fHeight += line.fTextMetrics.height();
    }
}

} // namespace text
} // namespace skia
