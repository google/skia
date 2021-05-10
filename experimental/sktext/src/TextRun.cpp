// Copyright 2021 Google LLC.

#include "experimental/sktext/src/TextRun.h"

namespace skia {
namespace text {

class Processor;

TextRun::TextRun(const SkShaper::RunHandler::RunInfo& info)
    : fFont(info.fFont)
    , fBidiLevel(info.fBidiLevel)
    , fAdvance(info.fAdvance)
    , fUtf8Range(info.utf8Range) {
    fGlyphs.push_back_n(info.glyphCount);
    fBounds.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fClusters.push_back_n(info.glyphCount + 1);
}

void TextRun::commit() {
    fFont.getBounds(fGlyphs.data(), fGlyphs.size(), fBounds.data(), nullptr);
    // To make edge cases easier
    fPositions[fGlyphs.size()] = fAdvance;
    fClusters[fGlyphs.size()] = this->leftToRight() ? fUtf8Range.end() : fUtf8Range.begin();
}

SkShaper::RunHandler::Buffer TextRun::newRunBuffer() {
    return {fGlyphs.data(), fPositions.data(), nullptr, fClusters.data(), {0.0f, 0.0f} };
}

SkScalar TextRun::calculateWidth(GlyphRange glyphRange) const {
    SkASSERT(glyphRange.fStart <= glyphRange.fEnd && glyphRange.fEnd < fPositions.size());
    return fPositions[glyphRange.fEnd].fX - fPositions[glyphRange.fStart].fX;
}

} // namespace text
} // namespace skia
