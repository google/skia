// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Processor.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/TextRun.h"
#include "modules/skshaper/src/SkUnicode.h"

namespace skia {
namespace text {
Line::Line(Processor* processor, const Stretch& stretch, const Stretch& spaces)
    : fTextStart(stretch.glyphStart())
    , fTextEnd(stretch.glyphEnd())
    , fWhitespacesEnd (spaces.glyphEnd())
    , fText(stretch.textRange())
    , fWhitespaces(spaces.textRange())
    , fTextWidth(stretch.width())
    , fSpacesWidth(spaces.width()) {

    SkASSERT(stretch.isEmpty() ||
                    spaces.isEmpty() ||
        (stretch.glyphEnd() == spaces.glyphStart()));

    if (!stretch.isEmpty()) {
        this->fTextMetrics.merge(stretch.textMetrics());
    }
    if (!spaces.isEmpty()) {
        this->fTextMetrics.merge(spaces.textMetrics());
    }

    // This is just chosen to catch the common/fast cases. Feel free to tweak.
    constexpr int kPreallocCount = 4;
    auto start = stretch.glyphStart().runIndex();
    auto end = spaces.glyphEnd().runIndex();
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
