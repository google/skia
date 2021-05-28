// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Layout.h"
#include "experimental/sktext/src/Visitor.h"

namespace skia {
namespace text {

void Visitor::visit() {

    SkPoint offset = SkPoint::Make(0 , 0);
    for (auto& line : fProcessor->fLines) {
        offset.fX = 0;
        LineInfo lineInfo(line.fText, line.fTextMetrics.baseline(), - line.fTextMetrics.above(), line.fTextMetrics.below());
        this->onBeginLine(lineInfo);
        for (auto& runIndex : line.fRunsInVisualOrder) {
            auto& run = fProcessor->fRuns[runIndex];
            auto startGlyph = runIndex == line.fTextStart.runIndex() ? line.fTextStart.glyphIndex() : 0;
            auto endGlyph = runIndex == line.fTextEnd.runIndex() ? line.fTextEnd.glyphIndex() : run.fGlyphs.size();
            TextRange textRange(run.fClusters[startGlyph], run.fClusters[endGlyph]);
            auto count = endGlyph - startGlyph;

            // Update positions
            SkAutoSTMalloc<256, SkPoint> positions(count + 1);
            offset.fX -= run.fPositions[startGlyph].fX;
            for (size_t i = startGlyph; i <= endGlyph; ++i) {
                positions[i - startGlyph] = run.fPositions[i] + offset + SkPoint::Make(0, line.fTextMetrics.baseline());
            }
            offset.fX += run.fPositions[endGlyph].fX;

            RunInfo runInfo(sk_sp<SkTypeface>(run.fFont.getTypeface()),
                            run.fFont.getSize(),
                            SkSpan<uint16_t>(run.fGlyphs.data() + startGlyph, count),
                            SkSpan<SkPoint>(positions.data(), count + 1));
            this->onGlyphRun(runInfo, DecorBlock(textRange.width()));
        }
        this->onEndLine(lineInfo);
        offset.fY += line.fTextMetrics.height();
    }
}

void Visitor::visit(SkSpan<DecorBlock> decorBlocks) {

    // Decor blocks have to be sorted by text cannot intersect but can skip some parts of the text
    // (in which case we use default text style from paragraph style)
    // The edges of the decor blocks don't have to match glyph, grapheme or even unicode code point edges
    // It's out responsibility to adjust them to some reasonable values
    // [a:b) -> [c:d) where
    // c is closest GG cluster edge to a from the left and d is closest GG cluster edge to b from the left

    DecorBlock* currentBlock = &decorBlocks[0];
    size_t currentStart = 0;
    SkPoint offset = SkPoint::Make(0 , 0);
    for (auto& line : fProcessor->fLines) {
        offset.fX = 0;
        LineInfo lineInfo(line.fText, line.fTextMetrics.baseline(), - line.fTextMetrics.above(), line.fTextMetrics.below());
        this->onBeginLine(lineInfo);
        for (auto& runIndex : line.fRunsInVisualOrder) {
            auto& run = fProcessor->fRuns[runIndex];
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

            size_t currentEnd = currentStart + currentBlock->fLength;
            for (auto glyphIndex = startGlyph; glyphIndex <= endGlyph; ++glyphIndex) {

                SkASSERT(currentBlock < decorBlocks.end());
                auto textIndex = run.fClusters[glyphIndex];
                if (glyphIndex == endGlyph) {
                    // last piece of the text
                } else if (run.leftToRight() && textIndex < currentEnd) {
                    continue;
                } else if (!run.leftToRight() && textIndex >= currentStart) {
                    continue;
                }
                textRange.fEnd = textIndex;
                glyphRange.fEnd = glyphIndex;

                // Update positions
                SkAutoSTMalloc<256, SkPoint> positions(glyphRange.width() + 1);
                offset.fX -= run.fPositions[startGlyph].fX;
                for (size_t i = glyphRange.fStart; i <= glyphRange.fEnd; ++i) {
                    positions[i - glyphRange.fStart] = run.fPositions[i] + offset + SkPoint::Make(0, line.fTextMetrics.baseline());
                }
                offset.fX += run.fPositions[endGlyph].fX;

                RunInfo runInfo(sk_sp<SkTypeface>(run.fFont.getTypeface()),
                                run.fFont.getSize(),
                                SkSpan<uint16_t>(run.fGlyphs.data() + startGlyph, glyphRange.width()),
                                SkSpan<SkPoint>(positions.data(), glyphRange.width() + 1));
                this->onGlyphRun(runInfo, *currentBlock);

                textRange.fStart = textIndex;
                glyphRange.fStart = glyphIndex;

                if (glyphIndex != endGlyph) {
                    // We are here because we reached the end of the block
                    ++currentBlock;
                }
            }
            /*
            // The last line
            if (endGlyph > startGlyph) {
                if (run.leftToRight()) {
                    textRange.fEnd = run.fClusters[endGlyph];
                } else {
                    textRange.fStart = run.fClusters[endGlyph];
                }
                glyphRange.fEnd = endGlyph;

            }
            */
        }
        this->onEndLine(lineInfo);
        offset.fY += line.fTextMetrics.height();
    }
}

} // namespace text
} // namespace skia
