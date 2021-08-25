// Copyright 2021 Google LLC.
#ifndef LogicalRun_DEFINED
#define LogicalRun_DEFINED

#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "modules/skshaper/include/SkShaper.h"

namespace skia {
namespace text {

class LogicalRun {
    public:
    LogicalRun(const SkShaper::RunHandler::RunInfo& info, TextIndex textStart, SkScalar glyphOffset);
    SkShaper::RunHandler::Buffer newRunBuffer() {
        return {fGlyphs.data(), fPositions.data(), fOffsets.data(), fClusters.data(), {0.0f, 0.0f} };
    }
    void commit() {
        fFont.getBounds(fGlyphs.data(), fGlyphs.size(), fBounds.data(), nullptr);
        fPositions[fGlyphs.size()] = fAdvance;
        fClusters[fGlyphs.size()] = this->leftToRight() ? fUtf8Range.end() : fUtf8Range.begin();
    }

    TextRange getTextRange() const { return fUtf16Range; }

    SkScalar calculateWidth(GlyphRange glyphRange) const {
        SkASSERT(glyphRange.fStart <= glyphRange.fEnd && glyphRange.fEnd < fPositions.size());
        return fPositions[glyphRange.fEnd].fX - fPositions[glyphRange.fStart].fX;
    }
    SkScalar calculateWidth(GlyphIndex start, GlyphIndex end) const {
      return calculateWidth(GlyphRange(start, end));
    }
    SkScalar width() const { return fAdvance.fX; }
    SkScalar firstGlyphPosition() const { return fPositions[0].fX; }

    bool leftToRight() const { return fBidiLevel % 2 == 0; }
    uint8_t bidiLevel() const { return fBidiLevel; }
    size_t size() const { return fGlyphs.size(); }

    LogicalRunType getRunType() const { return fRunType; }
    void setRunType(LogicalRunType runType) { fRunType = runType; }

    template <typename Callback>
    void forEachCluster(Callback&& callback) {
        GlyphIndex glyph = 0;
        for(; glyph < fClusters.size(); ++glyph) {
            callback(glyph, fRunStart + fClusters[glyph]);
        }
    }

    template <typename Callback>
    void convertUtf16Range(Callback&& callback) {
        this->fUtf16Range.fStart = callback(this->fUtf8Range.begin());
        this->fUtf16Range.fEnd = callback(this->fUtf8Range.end());
    }

    // Convert indexes into utf16 and also shift them to be on the entire text scale
    template <typename Callback>
    void convertClusterIndexes(Callback&& callback) {
        for (size_t glyph = 0; glyph < fClusters.size(); ++glyph) {
            fClusters[glyph] = callback(fClusters[glyph]);
        }
    }

    private:
    friend class ShapedText;
    friend class WrappedText;
    SkFont fFont;
    TextMetrics fTextMetrics;

    LogicalRunType fRunType;
    SkVector fAdvance;
    SkShaper::RunHandler::Range fUtf8Range;
    TextRange fUtf16Range;
    TextIndex fRunStart;
    SkScalar  fRunOffset;
    SkSTArray<128, SkGlyphID, true> fGlyphs;
    SkSTArray<128, SkPoint, true> fPositions;
    SkSTArray<128, SkPoint, true> fOffsets;
    SkSTArray<128, uint32_t, true> fClusters;
    SkSTArray<128, SkRect, true> fBounds;

    uint8_t fBidiLevel;
};

} // namespace text
} // namespace skia
#endif
