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

    fTextFontStyle = std::move(fontStyle);
    fFontBlocks = std::move(fontBlocks);

    Shaper shaper(this, fTextFontStyle.fFontManager);
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

  fTextFormatStyle = std::move(formatStyle);
    Formatter formatter(this);
    if (!formatter.process()) {
        return false;
    }
    return true;
}

// Once the text is decorated you can iterate it by segments (intersect of run, decor block and line)
bool Processor::decorate(SkTArray<DecorBlock, true>& decorBlocks) {

    this->sortDecorBlocks(decorBlocks);

    SkPaint dummy;
    dummy.setColor(SK_ColorBLACK);

    this->iterateByVisualOrder(CodeUnitFlags::kNonExistingFlag,
       [&](SkSize offset, SkScalar baseline, const TextRun* run, Range textRange, Range glyphRange, CodeUnitFlags flags) {
        SkTextBlobBuilder builder;
        const auto& blobBuffer = builder.allocRunPos(run->fFont, SkToInt(glyphRange.width()));
        sk_careful_memcpy(blobBuffer.glyphs, run->fGlyphs.data() + glyphRange.fStart, glyphRange.width() * sizeof(SkGlyphID));
        sk_careful_memcpy(blobBuffer.points(), run->fPositions.data() + glyphRange.fStart, glyphRange.width() * sizeof(SkPoint));

        fTextOutputs.emplace_back(builder.make(), dummy, offset);
    });

    return true;
}

// All at once
bool Processor::drawText(const char* text, SkCanvas* canvas, SkScalar x, SkScalar y) {

    SkString str(text);
    Range textRange(0, str.size());
    Processor processor(str);
    if (!processor.computeCodeUnitProperties()) {
        return false;
    }
    if (!processor.shape({ TextDirection::kLtr, SkFontMgr::RefDefault()}, {{{ SkString("Roboto"), 12.0f, SkFontStyle::Normal(), textRange }}})) {
        return false;
    }
    if (!processor.wrap(SK_ScalarInfinity, SK_ScalarInfinity)) {
        return false;
    }
    if (!processor.format(TextAlign::kLeft)) {
        return false;
    }
    SkTArray<DecorBlock, true> decor;
    if (!processor.decorate(decor)) {
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
    defaultBlocks.emplace(foreground, nullptr, Range(0, fText.size()));
    size_t start = 0;
    for (auto& block : fDecorBlocks) {
        this->adjustLeft(&block.fRange.fStart);
        this->adjustLeft(&block.fRange.fEnd);
        SkASSERT(start <= block.fRange.fStart);
        if (start < block.fRange.fStart) {
            auto defaultBlock = defaultBlocks.top();
            fDecorBlocks.emplace_back(defaultBlock.fForegroundColor, defaultBlock.fBackgroundColor, Range(start, block.fRange.fStart));
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
        fDecorBlocks.emplace_back(defaultBlock.fForegroundColor, defaultBlock.fBackgroundColor, Range(start, fText.size()));
    }
}

bool Processor::computeCodeUnitProperties() {

    fCodeUnitProperties.push_back_n(fText.size() + 1, CodeUnitFlags::kNoCodeUnitFlag);

    fUnicode = std::move(SkUnicode::Make());
    if (nullptr == fUnicode) {
        return false;
    }

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
    std::vector<SkUnicode::LineBreakBefore> lineBreaks;
    if (!fUnicode->getLineBreaks(fText.c_str(), fText.size(), &lineBreaks)) {
        return false;
    }
    for (auto& lineBreak : lineBreaks) {
        fCodeUnitProperties[lineBreak.pos] |= lineBreak.breakType == SkUnicode::LineBreakType::kHardLineBreak
                                           ? CodeUnitFlags::kHardLineBreakBefore
                                           : CodeUnitFlags::kSoftLineBreakBefore;
    }

    // Get graphemes
    std::vector<SkUnicode::Position> graphemes;
    if (!fUnicode->getGraphemes(fText.c_str(), fText.size(), &graphemes)) {
        return false;
    }
    for (auto pos : graphemes) {
        fCodeUnitProperties[pos] |= CodeUnitFlags::kGraphemeStart;
    }

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
void Processor::iterateByLogicalOrder(CodeUnitFlags units, Visitor visitor) {
    Range range(0, 0);
    for (size_t index = 0; index < fCodeUnitProperties.size(); ++index) {
        if (this->hasProperty(index, units)) {
            range.fEnd = index;
            visitor(range, this->fCodeUnitProperties[index]);
            range.fStart = index;
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

            auto startGlyph = runIndex == line.fTextStart.fRunIndex == runIndex ? line.fTextStart.fGlyphIndex : 0;
            auto endGlyph = runIndex == line.fTextEnd.fRunIndex ? line.fTextEnd.fGlyphIndex : run.fGlyphs.size();

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
    if (decorBlocks.empty()) {
        return;
    }

    SkSize offset = SkSize::MakeEmpty();
    for (auto& line : fLines) {
        offset.fWidth = 0;
        for (auto& runIndex : line.fRunsInVisualOrder) {
            auto& run = fRuns[runIndex];

            auto startGlyph = runIndex == line.fTextStart.fRunIndex == runIndex ? line.fTextStart.fGlyphIndex : 0;
            auto endGlyph = runIndex == line.fTextEnd.fRunIndex ? line.fTextEnd.fGlyphIndex : run.fGlyphs.size();

            Range textRange(run.fUtf8Range.begin(), run.fUtf8Range.end());
            Range glyphRange(startGlyph, endGlyph);
            for (auto glyphIndex = startGlyph; glyphIndex <= endGlyph; ++glyphIndex) {
                auto textIndex = run.fClusters[glyphIndex];
                // TODO: We can round decor block by glyphs, glyph clusters or graphemes or any combinations of them
                if (glyphIndex < endGlyph && !this->hasProperty(textIndex, CodeUnitFlags::kGlyphClusterStart)) {
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
/*
void iterateStylesInTextRange(const TextRun* run, Range textRange, SkScalar* runOffset, SkScalar textOffset, DecorBlockVisitor visitor) {
    // TODO: Take in account text direction so we always move in glyph order
    for (size_t blockIndex = 0; blockIndex < fDecorBlocks.size(); ++blockIndex) {
        auto& block = fDecorBlocks[blockIndex];
        if (block.fRange.fEnd <= textRange.fStart) {
            continue;
        } else if (block.fRange.fStart >= textRange.fEnd) {
            break;
        }
        Range textIntersect(std::max(block.fRange.fStart, textRange.fStart), std::min(block.fRange.fEnd, textRange.fEnd));
        Range glyphIntersect = Wrapper::glyphRange(run, textIntersect);
        // TODO: Somewhere we should adjust all ranges to grapheme edges
        visitor(run, glyphIntersect, {*runOffset, textOffset}, block.fForegroundColor);
        *runOffset = run->calculateWidth(glyphIntersect);
    }
}
*/
}}
