// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Processor.h"
#include "experimental/sktext/src/Painter.h"

namespace skia {
namespace text {

void Painter::onBeginLine(const LineInfo&) { }
void Painter::onGlyphRun(const RunInfo& runInfo, const DecorBlock& decorBlock) {
    auto size = runInfo.glyphs.size();
    SkTextBlobBuilder builder;
    const auto& blobBuffer = builder.allocRunPos(SkFont(runInfo.typeface, runInfo.size) , SkToInt(size));
    sk_careful_memcpy(blobBuffer.glyphs, runInfo.glyphs.data(), size * sizeof(uint16_t));
    sk_careful_memcpy(blobBuffer.points(), runInfo.positions.data(),size * sizeof(SkPoint));
    fCanvas->drawTextBlob(builder.make(), fXY.fX, fXY.fY, *decorBlock.fForegroundColor);
}

void Painter::onPlaceholderRun(const PlaceholderInfo&) { }
void Painter::onEndLine(const LineInfo&) { }

void Painter::paint(SkCanvas* canvas, SkPoint xy, SkSpan<DecorBlock> decorBlocks) {
    fCanvas = canvas;
    fXY = xy;
    visit(decorBlocks);
}

} // namespace text
} // namespace skia
