// Copyright 2021 Google LLC.

#include "experimental/sktext/src/LogicalRun.h"

namespace skia {
namespace text {

LogicalRun::LogicalRun(const SkShaper::RunHandler::RunInfo& info, TextIndex textStart, SkScalar glyphOffset)
    : fFont(info.fFont)
    , fTextMetrics(info.fFont)
    , fRunType(LogicalRunType::kText)
    , fAdvance(info.fAdvance)
    , fUtf8Range(info.utf8Range)
    , fRunStart(textStart)
    , fRunOffset(glyphOffset)
    , fBidiLevel(info.fBidiLevel)
{
    fGlyphs.push_back_n(info.glyphCount);
    fBounds.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fOffsets.push_back_n(info.glyphCount);
    fClusters.push_back_n(info.glyphCount + 1);
}

} // namespace text
} // namespace skia
