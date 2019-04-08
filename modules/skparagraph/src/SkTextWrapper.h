/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "SkRun.h"
#include "SkSpan.h"
#include "SkTHash.h"
#include "SkLine.h"

class SkTextWrapper {

  class Position {
    std::string toString(SkSpan<const char> text) {
      icu::UnicodeString
          utf16 = icu::UnicodeString(text.begin(), SkToS32(text.size()));
      std::string str;
      utf16.toUTF8String(str);
      return str;
    }
   public:
    Position(const SkCluster* start) {
      clean(start);
    }
    inline SkScalar width() const { return fWidth + fWhitespaces.fX; }
    inline SkScalar trimmedWidth() const { return fWidth; }
    inline SkScalar height() const {
      return fWidth == 0 ? fWhitespaces.fY : fSizes.height();
    }
    inline const SkCluster* trimmed() { return fTrimmedEnd; }
    inline const SkCluster* end() { return fEnd; }
    inline SkFontSizes sizes() { return fSizes; }

    void clean(const SkCluster* start) {
      fEnd = start;
      fTrimmedEnd = start;
      fWidth = 0;
      fWhitespaces = SkVector::Make(0, 0);
      fSizes.clean();
    }

    SkScalar add(Position& other) {

      auto result = other.fWidth;
      this->fWidth += this->fWhitespaces.fX + other.fWidth;
      this->fTrimmedEnd = other.fTrimmedEnd;
      this->fEnd = other.fEnd;
      this->fWhitespaces = other.fWhitespaces;
      this->fSizes.add(other.fSizes);
      other.clean(other.fEnd);

      return result;
    }

    void add(const SkCluster& cluster) {
      if (cluster.isWhitespaces()) {
        fWhitespaces.fX += cluster.fWidth;
        fWhitespaces.fY = SkTMax(fWhitespaces.fY, cluster.fRun->calculateHeight());
      } else {
        fTrimmedEnd = &cluster;
        fWidth += cluster.fWidth + fWhitespaces.fX;
        fWhitespaces = SkVector::Make(0, 0);
        fSizes.add(cluster.fRun->ascent(), cluster.fRun->descent(), cluster.fRun->leading());
      }
      fEnd = &cluster;
    }

    void extend(SkScalar w) { fWidth += w; }

    SkSpan<const char> trimmedText(const SkCluster* start) {
      return SkSpan<const char>(start->fText.begin(), fTrimmedEnd->fText.end() - start->fText.begin());
    }

   private:
    SkScalar fWidth;
    SkFontSizes fSizes;
    SkVector fWhitespaces;
    const SkCluster* fEnd;
    const SkCluster* fTrimmedEnd;
  };

 public:

  SkTextWrapper() : fClosestBreak(nullptr), fAfterBreak(nullptr), fMinIntrinsicWidth(0) { }
  void formatText(SkSpan<SkCluster> clusters,
                  SkScalar maxWidth,
                  size_t maxLines,
                  const std::string& ellipsis);
  SkSpan<SkLine> getLines() { return SkSpan<SkLine>(fLines.begin(), fLines.size()); }
  SkSpan<const SkLine> getConstLines() const { return SkSpan<const SkLine>(fLines.begin(), fLines.size()); }
  SkLine* getLastLine() { return &fLines.back(); }

  inline SkScalar height() const { return fHeight; }
  inline SkScalar width() const { return fWidth; }
  inline SkScalar intrinsicWidth() const { return fMinIntrinsicWidth; }

  void reset() { fLines.reset(); }

 private:

  bool endOfText() const { return fLineStart == fClusters.end(); }
  bool reachedLinesLimit(int32_t delta) const {
    return fMaxLines != std::numeric_limits<size_t>::max() && fLines.size() >= fMaxLines + delta;
  }
  SkRun* createEllipsis(Position& pos);
  bool addLine(Position& pos);
  SkRun* shapeEllipsis(SkRun* run);
  SkRun* getEllipsis(SkRun* run);

  SkSpan<SkCluster> fClusters;
  SkTArray<SkLine, true> fLines;
  SkScalar fMaxWidth;
  size_t fMaxLines;
  std::string fEllipsis;
  const SkCluster* fLineStart;
  Position fClosestBreak;
  Position fAfterBreak;
  SkVector fCurrentLineOffset;
  SkScalar fWidth;
  SkScalar fHeight;
  SkScalar fMinIntrinsicWidth;

  // TODO: make a static cache?
  SkTHashMap<SkFont, SkRun> fEllipsisCache; // All found so far shapes of ellipsis
};
