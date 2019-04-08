/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <unicode/brkiter.h>
#include "uchar.h"
#include "SkSpan.h"
#include "SkShaper.h"
#include "SkFontMetrics.h"

class SkRun;

class SkFontSizes {
  SkScalar fAscent;
  SkScalar fDescent;
  SkScalar fLeading;

 public:
  SkFontSizes() {
    clean();
  }
  SkFontSizes(SkScalar a, SkScalar d, SkScalar l) {
    fAscent = a;
    fDescent = a;
    fLeading = l;
  }
  void add(SkScalar a, SkScalar d, SkScalar l) {
    fAscent = SkTMin(fAscent, a);
    fDescent = SkTMax(fDescent, d);
    fLeading = SkTMax(fLeading, l);
  }
  void add (SkFontSizes other) {
    add(other.fAscent, other.fDescent, other.fLeading);
  }
  void clean() {
    fAscent = 0;
    fDescent = 0;
    fLeading = 0;
  }
  SkScalar diff(SkFontSizes maxSizes) const {
    return ascent() - maxSizes.ascent() - leading() / 2;
  }
  SkScalar height() const {
    return fDescent - fAscent + fLeading;
  }
  SkScalar leading() const { return fLeading; }
  SkScalar ascent() const { return fAscent; }
  SkScalar descent() const { return fDescent; }
};

struct SkCluster {

  enum BreakType {
    None,
    WordBoundary,             // calculated for all clusters (UBRK_WORD)
    WordBreakWithoutHyphen,   // calculated only for hyphenated words
    WordBreakWithHyphen,
    SoftLineBreak,            // calculated for all clusters (UBRK_LINE)
    HardLineBreak,            // calculated for all clusters (UBRK_LINE)
  };

  SkCluster() : fRun(nullptr), fWhiteSpaces(false), fIgnore(false), fBreakType(None) { }

  SkScalar sizeToChar(const char* ch) const {

    if (ch < fText.begin() || ch >= fText.end()) {
      return 0;
    }
    auto shift = ch - fText.begin();
    auto ratio = shift * 1.0 / fText.size();

    return fWidth * ratio;
  }
  SkScalar sizeFromChar(const char* ch) const {

    if (ch < fText.begin() || ch >= fText.end()) {
      return 0;
    }
    auto shift = fText.end() - ch - 1;
    auto ratio = shift * 1.0 / fText.size();

    return fWidth * (1 - ratio);
  }
  inline void setBreakType(BreakType type) { fBreakType = type; }
  inline BreakType getBreakType() const { return fBreakType; }
  inline void setIsWhiteSpaces(bool ws) { fWhiteSpaces = ws; }
  inline bool isWhitespaces() const { return fWhiteSpaces; }
  inline bool isIgnored() const { return fIgnore; }
  void ignore() { fIgnore = true; }
  bool canBreakLineAfter() const { return fBreakType == SoftLineBreak ||
                                          fBreakType == HardLineBreak; }
  bool isHardBreak() const { return fBreakType == HardLineBreak; }

  void setIsWhiteSpaces() {
    auto pos = fText.end();
    while (--pos >= fText.begin()) {
      auto ch = *pos;
      if (!u_isspace(ch) &&
          u_charType(ch) != U_CONTROL_CHAR &&
          u_charType(ch) != U_NON_SPACING_MARK) {
        return;
      }
    }
    fWhiteSpaces = true;
  }

  SkSpan<const char> fText;

  SkRun* fRun;
  size_t fStart;
  size_t fEnd;

  SkScalar fWidth;
  SkScalar fHeight;
  SkScalar fShift;

  bool fWhiteSpaces;
  bool fIgnore;

  BreakType fBreakType;
};

class SkRun {
 public:

  SkRun() : fFont() { }
  SkRun(const SkShaper::RunHandler::RunInfo& info, SkScalar shiftX);

  SkShaper::RunHandler::Buffer newRunBuffer();

  inline size_t size() const { return fGlyphs.size(); }
  void setWidth(SkScalar width) { fAdvance.fX = width; }
  void setHeight(SkScalar height) { fAdvance.fY = height; }
  void shift(SkScalar shiftX, SkScalar shiftY) {
    fOffset.fX += shiftX;
    fOffset.fY += shiftY;
  }
  SkVector advance() const {
    return SkVector::Make(fAdvance.fX,
                          fFontMetrics.fDescent + fFontMetrics.fLeading - fFontMetrics.fAscent);
  }
  inline SkVector offset() const { return fOffset; }
  inline SkScalar ascent() const { return fFontMetrics.fAscent; }
  inline SkScalar descent() const { return fFontMetrics.fDescent; }
  inline SkScalar leading() const { return fFontMetrics.fLeading; }
  inline const SkFont& font() const { return fFont ; };

  inline SkShaper::RunHandler::Range range() const { return fUtf8Range; }
  inline size_t cluster(size_t pos) const { return fClusters[pos]; }
  inline SkPoint position(size_t pos) const {
    if (pos < size()) {
      return fPositions[pos];
    }
    return SkVector::Make(
        fAdvance.fX - (fPositions[size() - 1].fX - fPositions[0].fX),
        fAdvance.fY);
  }
  SkRect clip() {
    return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fAdvance.fX, fAdvance.fY);
  }

  SkScalar calculateHeight();
  SkScalar calculateWidth(size_t start, size_t end);

  SkFontSizes sizes() const { return SkFontSizes(fFontMetrics.fAscent, fFontMetrics.fDescent, fFontMetrics.fLeading); }

  void copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size, SkVector offset) const;

 private:

  friend class SkParagraphImpl;

  SkFont fFont;
  SkFontMetrics fFontMetrics;
  uint8_t fLtr;
  SkVector fAdvance;
  size_t glyphCount;
  SkShaper::RunHandler::Range fUtf8Range;
  SkVector fOffset;
  SkSTArray<128, SkGlyphID, true> fGlyphs;
  SkSTArray<128, SkPoint, true> fPositions;
  SkSTArray<128, uint32_t, true> fClusters;
};
