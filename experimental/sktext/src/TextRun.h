// Copyright 2021 Google LLC.
#ifndef TextRun_DEFINED
#define TextRun_DEFINED

#include "experimental/sktext/include/Types.h"
#include "modules/skshaper/include/SkShaper.h"

namespace skia {
namespace text {

class Run {

};
class TextRun {
 public:
  TextRun(const SkShaper::RunHandler::RunInfo& info);
  SkShaper::RunHandler::Buffer newRunBuffer();
  void commit();

  SkScalar calculateWidth(GlyphRange glyphRange) const;

  bool leftToRight() const { return fBidiLevel % 2 == 0; }
  uint8_t bidiLevel() const { return fBidiLevel; }
  GlyphIndex findGlyph(TextIndex textIndex) const;

 private:
  friend class UnicodeText;
  friend class ShapedText;
  friend class FormattedText;
  SkFont fFont;

  SkVector fAdvance;
  SkShaper::RunHandler::Range fUtf8Range;
  TextRange fUtf16Range;
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
