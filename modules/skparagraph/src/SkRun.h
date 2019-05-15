/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <unicode/brkiter.h>
#include "include/core/SkPoint.h"
#include "include/core/SkTextBlob.h"
#include "uchar.h"
#include "src/core/SkSpan.h"
#include "modules/skshaper/include/SkShaper.h"
#include "include/core/SkFontMetrics.h"

class SkCluster;
class SkRun {
 public:

  SkRun() = default;
  SkRun(SkSpan<const char> text,
        const SkShaper::RunHandler::RunInfo& info,
        SkScalar lineHeight,
        size_t index,
        SkScalar shiftX);
  ~SkRun() { }

  SkShaper::RunHandler::Buffer newRunBuffer();

  inline size_t size() const { return fGlyphs.size(); }
  void setWidth(SkScalar width) { fAdvance.fX = width; }
  void setHeight(SkScalar height) { fAdvance.fY = height; }
  void shift(SkScalar shiftX, SkScalar shiftY) {
    fOffset.fX += shiftX;
    fOffset.fY += shiftY;
  }
  SkVector advance() const {
    return SkVector::Make(fAdvance.fX, fFontMetrics.fDescent - fFontMetrics.fAscent);
  }
  inline SkVector offset() const { return fOffset; }
  inline SkScalar ascent() const { return fFontMetrics.fAscent; }
  inline SkScalar descent() const { return fFontMetrics.fDescent; }
  inline SkScalar leading() const { return fFontMetrics.fLeading; }
  inline const SkFont& font() const { return fFont ; };
  inline bool leftToRight() const { return fBidiLevel % 2 == 0; }
  inline size_t index() const { return fIndex; }
  inline SkScalar lineHeight() const { return fHeightMultiplier; }
  inline SkSpan<const char> text() const { return fText; }
  inline size_t clusterIndex(size_t pos) const { return fClusterIndexes[pos]; }
  SkScalar positionX(size_t pos) const {
    return fPositions[pos].fX + fOffsets[pos];
  }
  SkScalar offset(size_t index) const { return fOffsets[index]; }
  inline SkSpan<SkCluster> clusters() const { return fClusters; }
  inline void setClusters(SkSpan<SkCluster> clusters) { fClusters = clusters; }
  SkRect clip() const {
    return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fAdvance.fX, fAdvance.fY);
  }

  SkScalar addSpacesAtTheEnd(SkScalar space, SkCluster* cluster);
  SkScalar addSpacesEvenly(SkScalar space, SkCluster* cluster);
  void shift(const SkCluster* cluster, SkScalar offset);

  SkScalar calculateHeight() const {
    return fFontMetrics.fDescent - fFontMetrics.fAscent;
  }
  SkScalar calculateWidth(size_t start, size_t end, bool clip) const;

  void copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size, SkVector offset) const;

  void iterateThroughClustersInTextOrder(std::function<void(
      size_t glyphStart,
      size_t glyphEnd,
      size_t charStart,
      size_t charEnd,
      SkScalar width,
      SkScalar height)> visitor);

  std::tuple<bool, SkCluster*, SkCluster*> findLimitingClusters(SkSpan<const char> text);

 private:

  friend class SkParagraphImpl;
  friend class SkLine;
  friend class SkLineMetrics;

  SkFont fFont;
  SkFontMetrics fFontMetrics;
  SkScalar fHeightMultiplier;
  size_t fIndex;
  uint8_t fBidiLevel;
  SkVector fAdvance;
  SkSpan<const char> fText;
  SkSpan<SkCluster> fClusters;
  SkVector fOffset;
  SkShaper::RunHandler::Range fUtf8Range;
  SkSTArray<128, SkGlyphID, false> fGlyphs;
  SkSTArray<128, SkPoint, true> fPositions;
  SkSTArray<128, uint32_t, true> fClusterIndexes;
  SkSTArray<128, SkScalar, true> fOffsets;        // For formatting (letter/word spacing, justification)
  bool fSpaced;
};

class SkCluster {

 public:
  enum BreakType {
    None,
    CharacterBoundary,        // not yet in use (UBRK_CHARACTER)
    WordBoundary,             // calculated for all clusters (UBRK_WORD)
    WordBreakWithoutHyphen,   // calculated only for hyphenated words
    WordBreakWithHyphen,
    SoftLineBreak,            // calculated for all clusters (UBRK_LINE)
    HardLineBreak,            // calculated for all clusters (UBRK_LINE)
  };

  SkCluster()
      : fText(nullptr, 0), fRun(nullptr), fStart(0), fEnd(), fWidth(),
        fSpacing(0), fHeight(), fWhiteSpaces(false), fBreakType(None) {}

  SkCluster(SkRun* run, size_t start, size_t end, SkSpan<const char> text, SkScalar width, SkScalar height)
      : fText(text), fRun(run)
      , fStart(start), fEnd(end)
      , fWidth(width), fSpacing(0), fHeight(height)
      , fWhiteSpaces(false), fBreakType(None) { }

  ~SkCluster() = default;

  SkScalar sizeToChar(const char* ch) const;

  SkScalar sizeFromChar(const char* ch) const;

  void space(SkScalar shift, SkScalar space) {
    fSpacing += space;
    fWidth += shift;
  }

  inline void setBreakType(BreakType type) { fBreakType = type; }
  inline void setIsWhiteSpaces(bool ws) { fWhiteSpaces = ws; }
  inline bool isWhitespaces() const { return fWhiteSpaces; }
  inline bool canBreakLineAfter() const { return fBreakType == SoftLineBreak || fBreakType == HardLineBreak; }
  inline bool isHardBreak() const { return fBreakType == HardLineBreak; }
  inline bool isSoftBreak() const { return fBreakType == SoftLineBreak; }
  inline SkRun* run() const { return fRun; }
  inline size_t startPos() const { return fStart; }
  inline size_t endPos() const { return fEnd; }
  inline SkScalar width() const { return fWidth; }
  inline SkScalar trimmedWidth() const { return fWidth - fSpacing; }
  inline SkScalar lastSpacing() const { return fSpacing; }
  inline SkScalar height() const { return fHeight; }
  inline SkSpan<const char> text() const { return fText; }

  void shift(SkScalar offset) const { this->run()->shift(this, offset); }

  void setIsWhiteSpaces();

  bool contains(const char* ch) const {
    return ch >= fText.begin() && ch < fText.end();
  }

  bool belongs(SkSpan<const char> text) const {
    return fText.begin() >= text.begin() && fText.end() <= text.end();
  }

  bool startsIn(SkSpan<const char> text) const {
    return fText.begin() >= text.begin() && fText.begin() < text.end();
  }

 private:
  SkSpan<const char> fText;

  SkRun* fRun;
  size_t fStart;
  size_t fEnd;
  SkScalar fWidth;
  SkScalar fSpacing;
  SkScalar fHeight;
  bool fWhiteSpaces;
  BreakType fBreakType;
};

class SkLineMetrics {
 public:
  SkLineMetrics() { clean(); }

  SkLineMetrics(SkScalar a, SkScalar d, SkScalar l) {
    fAscent = a;
    fDescent = d;
    fLeading = l;
  }

  void add (SkRun* run) {
    fAscent = SkTMin(fAscent, run->ascent() * run->lineHeight());
    fDescent = SkTMax(fDescent, run->descent() * run->lineHeight());
    fLeading = SkTMax(fLeading, run->leading() * run->lineHeight());
  }

  void add (SkLineMetrics other) {
    fAscent = SkTMin(fAscent, other.fAscent);
    fDescent = SkTMax(fDescent, other.fDescent);
    fLeading = SkTMax(fLeading, other.fLeading);
  }
  void clean() {
    fAscent = 0;
    fDescent = 0;
    fLeading = 0;
  }

  SkScalar delta() const { return height() - ideographicBaseline(); }

  void updateLineMetrics(SkLineMetrics& metrics, bool forceHeight) {
    if (forceHeight) {
      metrics.fAscent = fAscent;
      metrics.fDescent = fDescent;
      metrics.fLeading = fLeading;
    } else {
      metrics.fAscent = SkTMin(metrics.fAscent, fAscent);
      metrics.fDescent = SkTMax(metrics.fDescent, fDescent);
      metrics.fLeading = SkTMax(metrics.fLeading, fLeading);
    }
  }

  SkScalar runTop(SkRun* run) const {
    return fLeading / 2 - fAscent + run->ascent() + delta();
  }
  inline SkScalar height() const { return SkScalarRoundToInt(fDescent - fAscent + fLeading); }
  inline SkScalar alphabeticBaseline() const { return fLeading / 2 - fAscent;  }
  inline SkScalar ideographicBaseline() const { return fDescent - fAscent + fLeading; }
  inline SkScalar baseline() const { return fLeading / 2 - fAscent; }
  inline SkScalar ascent() const { return fAscent; }
  inline SkScalar descent() const { return fDescent; }
  inline SkScalar leading() const { return fLeading; }

 private:

  SkScalar fAscent;
  SkScalar fDescent;
  SkScalar fLeading;
};
