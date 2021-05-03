// Copyright 2021 Google LLC.
#ifndef TextRun_DEFINED
#define TextRun_DEFINED

#include "experimental/sktext/include/Types.h"
#include "modules/skshaper/include/SkShaper.h"

namespace skia {
namespace text {
class TextRun {
 public:
  TextRun(const SkShaper::RunHandler::RunInfo& info);
  TextRun& operator=(const TextRun&) = delete;
  TextRun(TextRun&&) = default;
  TextRun& operator=(TextRun&&) = delete;
  ~TextRun() = default;

  SkShaper::RunHandler::Buffer newRunBuffer();
  void commit();

  SkScalar calculateWidth(GlyphRange glyphRange) const;

  bool leftToRight() const { return fBidiLevel % 2 == 0; }
  uint8_t bidiLevel() const { return fBidiLevel; }

 private:
  friend class Wrapper;
  friend class Processor;

  SkFont fFont;

  SkVector fAdvance;
  SkShaper::RunHandler::Range fUtf8Range;
  SkSTArray<128, SkGlyphID, true> fGlyphs;
  SkSTArray<128, SkPoint, true> fPositions;
  SkSTArray<128, uint32_t, true> fClusters;
  SkSTArray<128, SkRect, true> fBounds;

  uint8_t fBidiLevel;
};
} // namespace text
} // namespace skia
#endif
