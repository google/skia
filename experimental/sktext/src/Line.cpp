// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Processor.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/TextRun.h"
#include "modules/skshaper/src/SkUnicode.h"

namespace skia {
namespace text {
Line::Line(Processor* processor, const Stretch& stretch, const Stretch& spaces)
    : fTextStart(stretch.fGlyphStart)
    , fTextEnd(stretch.fGlyphEnd)
    , fWhitespacesEnd (spaces.fGlyphEnd)
    , fText(stretch.fTextRange)
    , fWhitespaces(spaces.fTextRange)
    , fTextWidth(stretch.fWidth)
    , fSpacesWidth(spaces.fWidth) {

    SkASSERT(stretch.isEmpty() ||
                    spaces.isEmpty() ||
        (stretch.fGlyphEnd == spaces.fGlyphStart));

    if (!stretch.isEmpty()) {
        this->fTextMetrics.merge(stretch.fTextMetrics);
    }
    if (!spaces.isEmpty()) {
        this->fTextMetrics.merge(spaces.fTextMetrics);
    }

    // This is just chosen to catch the common/fast cases. Feel free to tweak.
    constexpr int kPreallocCount = 4;
    auto start = stretch.fGlyphStart.fRunIndex;
    auto end = spaces.fGlyphEnd.fRunIndex;
    auto numRuns = end - start + 1;
    SkAutoSTArray<kPreallocCount, SkUnicode::BidiLevel> runLevels(numRuns);
    size_t runLevelsIndex = 0;
    for (auto runIndex = start; runIndex <= end; ++runIndex) {
        auto& run = processor->run(runIndex);
        runLevels[runLevelsIndex++] = run.bidiLevel();
    }
    SkASSERT(runLevelsIndex == numRuns);
    SkAutoSTArray<kPreallocCount, int32_t> logicalOrder(numRuns);
    processor->getUnicode()->reorderVisual(runLevels.data(), numRuns, logicalOrder.data());
    auto firstRunIndex = start;
    for (auto index : logicalOrder) {
        fRunsInVisualOrder.push_back(firstRunIndex + index);
    }
}
} // namespace text
} // namespace skia
