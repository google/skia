// Copyright 2021 Google LLC.

#include "experimental/sktext/src/TextRun.h"

namespace skia {
namespace text {

class Processor;

TextRun::TextRun(const SkShaper::RunHandler::RunInfo& info, TextIndex textStart, SkScalar glyphOffset)
    : fFont(info.fFont)
    , fBidiLevel(info.fBidiLevel)
    , fAdvance(info.fAdvance)
    , fUtf8Range(info.utf8Range)
    , fTextMetrics(info.fFont)
    , fRunStart(textStart)
    , fRunOffset(glyphOffset) {
    fGlyphs.push_back_n(info.glyphCount);
    fBounds.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fOffsets.push_back_n(info.glyphCount);
    fClusters.push_back_n(info.glyphCount + 1);
}

void TextRun::commit() {
    fFont.getBounds(fGlyphs.data(), fGlyphs.size(), fBounds.data(), nullptr);
    // To make edge cases easier
    fPositions[fGlyphs.size()] = fAdvance;
    fClusters[fGlyphs.size()] = this->leftToRight() ? fUtf8Range.end() : fUtf8Range.begin();
}

SkShaper::RunHandler::Buffer TextRun::newRunBuffer() {
    return {fGlyphs.data(), fPositions.data(), fOffsets.data(), fClusters.data(), {0.0f, 0.0f} };
}

SkScalar TextRun::calculateWidth(GlyphRange glyphRange) const {
    SkASSERT(glyphRange.fStart <= glyphRange.fEnd && glyphRange.fEnd < fPositions.size());
    return fPositions[glyphRange.fEnd].fX - fPositions[glyphRange.fStart].fX;
}

GlyphIndex TextRun::findGlyph(TextIndex textIndex) const {
    GlyphIndex glyphIndex = EMPTY_INDEX;
    for (size_t i = 0; i < fClusters.size(); ++i) {
        auto cluster = fClusters[i];
        if (this->leftToRight()) {
            if (cluster > textIndex) {
                break;
            }
        } else if (cluster < textIndex) {
            break;
        }
        glyphIndex = i;
    }
    return glyphIndex;
}

} // namespace text
} // namespace skia
