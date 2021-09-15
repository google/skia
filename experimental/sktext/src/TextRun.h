// Copyright 2021 Google LLC.
#ifndef TextRun_DEFINED
#define TextRun_DEFINED

#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "modules/skshaper/include/SkShaper.h"

namespace skia {
namespace text {

struct Position;
class TextRun {
    public:
    TextRun(const SkShaper::RunHandler::RunInfo& info, TextIndex textStart, SkScalar glyphOffset);
    SkShaper::RunHandler::Buffer newRunBuffer();
    void commit();

    SkScalar calculateWidth(GlyphRange glyphRange) const;
    SkScalar calculateWidth(GlyphIndex start, GlyphIndex end) const {
      return calculateWidth(GlyphRange(start, end));
    }
    SkScalar width() const { return fAdvance.fX; }
    SkScalar firstGlyphPosition() const { return fPositions[0].fX; }

    bool leftToRight() const { return fBidiLevel % 2 == 0; }
    uint8_t bidiLevel() const { return fBidiLevel; }
    GlyphIndex findGlyph(TextIndex textIndex) const;
    size_t size() const { return fGlyphs.size(); }
    TextRange getTextRange(GlyphRange glyphRange) const;
    GlyphRange getGlyphRange(TextRange textRange) const;
    GlyphIndex getGlyphIndex(TextIndex textIndex) const;

    GlyphIndex findGlyphIndexLeftOf(GlyphRange runGlyphRange, TextIndex textIndex) const {
        GlyphIndex found = runGlyphRange.fStart;
        for (auto i = runGlyphRange.fStart; i <= runGlyphRange.fEnd; ++i) {
            auto clusterIndex = fClusters[i];
            if ((this->leftToRight() && clusterIndex > textIndex) ||
                (!this->leftToRight() && clusterIndex < textIndex)) {
                break;
            }
            found = i;
        }
        return found;
    }

    template <typename Callback>
    void forEachTextChunkInGlyphRange(SkSpan<TextIndex> textChunks, GlyphRange runGlyphRange, Callback&& callback) const {
        TextRange runTextRange = this->getTextRange(runGlyphRange);
        if (this->leftToRight()) {
            TextIndex currentIndex = 0;
            TextRange textRange(runTextRange.fStart, runTextRange.fStart);
            for (auto currentTextChunk : textChunks) {
                if (currentIndex >= runTextRange.fEnd) {
                    break;
                }
                currentIndex += currentTextChunk;
                if (currentIndex < runTextRange.fStart) {
                    continue;
                }
                textRange.fStart = textRange.fEnd;
                textRange.fEnd += currentTextChunk;
                textRange.fEnd = std::min(runTextRange.fEnd, textRange.fEnd);

                callback(textRange, getGlyphRange(textRange));
            }
        } else {
            // TODO: Implement RTL
            SkASSERT(false);
        }
    }

    template <typename Callback>
    void forEachCluster(Callback&& callback) {
        for(auto& clusterIndex : fClusters) {
            callback(fRunStart + clusterIndex);
        }
    }

    // Convert indexes into utf16 and also shift them to be on the entire text scale
    template <typename Callback>
    void convertClusterIndexes(Callback&& callback) {
        fGlyphByText.push_back_n(fUtf16Range.width() + 1);
        TextIndex start = fClusters[0];
        for (size_t glyph = 0; glyph < fClusters.size(); ++glyph) {
            auto& clusterIndex = fClusters[glyph];
            clusterIndex = callback(clusterIndex);
            // Convert fGlyphByText, too
            if (this->leftToRight()) {
                for (auto i = start; i <= clusterIndex; ++i) {
                    fGlyphByText[i] = glyph;
                }
            } else {
                for (auto i = clusterIndex; i >= start; --i) {
                    fGlyphByText[i] = glyph;
                }
            }
            start = clusterIndex + 1;
        }
    }

    // Convert indexes into utf16 and also shift them to be on the entire text scale
    template <typename Callback>
    void convertUtf16Range(Callback&& callback) {
        this->fUtf16Range.fStart = callback(this->fUtf8Range.begin());
        this->fUtf16Range.fEnd = callback(this->fUtf8Range.end());
    }

    private:
    friend class ShapedText;
    friend class FormattedText;
    SkFont fFont;

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

    SkSTArray<128, GlyphIndex> fGlyphByText; // Sort of the opposite of fClusters

    TextMetrics fTextMetrics;
    uint8_t fBidiLevel;

    size_t sizeWithoutTrailingSpaces() const;
};

// This is a input/output range within one run
class Line;
struct Position {
    Position(PositionType positionType, size_t lineIndex, const TextRun* run, GlyphRange glyphRange, TextRange textRange, SkRect rect)
        : fPositionType(positionType)
        , fLineIndex(lineIndex)
        , fRun(run)
        , fGlyphRange(glyphRange)
        , fTextRange(textRange)
        , fBoundaries(rect) { }

    Position(PositionType positionType)
        : Position(positionType, EMPTY_INDEX, nullptr, EMPTY_RANGE, EMPTY_RANGE, SkRect::MakeEmpty()) { }

    PositionType fPositionType;
    size_t fLineIndex;
    const TextRun* fRun;
    GlyphRange fGlyphRange;
    TextRange fTextRange;
    SkRect fBoundaries;
};
} // namespace text
} // namespace skia
#endif
