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

  template <typename Callback>
  void forEachCluster(Callback&& callback) {
      for(auto& clusterIndex : fClusters) {
          callback(fRunStart + clusterIndex);
      }
  }

    // Convert indexes into utf16 and also shift them to be on the entire text scale
    template <typename Callback>
    void convertClusterIndexes(Callback&& callback) {
        for(auto& clusterIndex : fClusters) {
            clusterIndex = callback(clusterIndex);
        }
    }

    // Convert indexes into utf16 and also shift them to be on the entire text scale
    template <typename Callback>
    void convertUtf16Range(Callback&& callback) {
        this->fUtf16Range.fStart = callback(this->fUtf8Range.begin());
        this->fUtf16Range.fEnd = callback(this->fUtf8Range.end());
    }

 private:
  //friend class UnicodeText;
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

  TextMetrics fTextMetrics;
  uint8_t fBidiLevel;
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
