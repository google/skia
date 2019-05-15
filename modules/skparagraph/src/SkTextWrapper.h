/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <string>
#include "src/core/SkSpan.h"
#include "SkLine.h"

class SkParagraphImpl;
class SkTextWrapper {
 public:

  class Position {
   public:
    explicit Position(const SkCluster* start) { clean(start); }
    inline SkScalar width() const { return fWidth + fWhitespaces.fX; }
    SkScalar trimmedWidth() const { return fWidth - fTrimmedEnd->lastSpacing(); }
    inline SkScalar height() const { return fLineMetrics.height(); }
    inline const SkCluster* trimmed() const { return fTrimmedEnd; }
    inline const SkCluster* end() const { return fEnd; }
    inline SkLineMetrics& sizes() { return fLineMetrics; }

    void clean(const SkCluster* start) {
      if (fEnd != start) {
        fLineMetrics.clean();
      }
      fEnd = start;
      fTrimmedEnd = start;
      fWidth = 0;
      fWhitespaces = SkVector::Make(0, 0);
    }
    SkScalar moveTo(Position& other) {
      auto result = other.fWidth;
      if (other.fWidth > 0) {
        this->fWidth += this->fWhitespaces.fX + other.fWidth;
        this->fTrimmedEnd = other.fTrimmedEnd;
        this->fEnd = other.fEnd;
        this->fWhitespaces = other.fWhitespaces;
      } else {
        this->fWhitespaces.fX += other.fWhitespaces.fX;
        this->fWhitespaces.fY = SkTMax(this->fWhitespaces.fY, other.fWhitespaces.fY);
      }
      this->fLineMetrics.add(other.fLineMetrics);
      other.clean(other.fEnd);
      return result;
    }
    void moveTo(const SkCluster& cluster) {
      if (cluster.isWhitespaces()) {
        this->fWhitespaces.fX += cluster.width();
        this->fWhitespaces.fY = SkTMax(this->fWhitespaces.fY, cluster.run()->calculateHeight());
      } else {
        this->fTrimmedEnd = &cluster;
        this->fWidth += this->fWhitespaces.fX + cluster.width();
        this->fWhitespaces = SkVector::Make(0, 0);
      }
      this->fLineMetrics.add(cluster.run());
      this->fEnd = &cluster;
    }

    SkSpan<const char> trimmedText(const SkCluster* start) {
      size_t size = fTrimmedEnd->text().end() > start->text().begin()
                    ? fTrimmedEnd->text().end() - start->text().begin() : 0;
      return SkSpan<const char>(start->text().begin(), size);
    }

   private:
    SkScalar fWidth;
    SkLineMetrics fLineMetrics;
    SkVector fWhitespaces;
    const SkCluster* fEnd;
    const SkCluster* fTrimmedEnd;
  };

  explicit SkTextWrapper(SkParagraphImpl* parent)
  : fParent(parent), fLastSoftLineBreak(nullptr), fLastClusterBreak(nullptr) { reset(); }

  void formatText(SkSpan<SkCluster> clusters,
                  SkScalar maxWidth,
                  size_t maxLines,
                  const std::string& ellipsis);

  inline SkScalar height() const { return fHeight; }
  inline SkScalar width() const { return fWidth; }
  inline SkScalar intrinsicWidth() const { return fMinIntrinsicWidth; }

  void reset() {
    fLineStart = nullptr;
    fLastSoftLineBreak.clean(nullptr);
    fLastClusterBreak.clean(nullptr);
    fMinIntrinsicWidth = 0;
    fOffsetY = 0;
    fWidth = 0;
    fHeight = 0;
    fLineNumber = 0;
    fMaxLines = std::numeric_limits<size_t>::max();
  }

 private:

  bool endOfText() const { return fLineStart == fClusters.end(); }
  bool addLineUpToTheLastBreak();
  bool reachedLinesLimit() const {
    return fMaxLines != std::numeric_limits<size_t>::max() && fLineNumber >= fMaxLines;
  }
  SkScalar lengthUntilSoftLineBreak(SkCluster* cluster);

  SkParagraphImpl* fParent;
  SkSpan<SkCluster> fClusters;
  std::string fEllipsisText;
  const SkCluster* fLineStart;
  Position fLastSoftLineBreak;
  Position fLastClusterBreak;
  SkScalar fOffsetY;
  size_t fLineNumber;
  size_t fMaxLines;
  SkScalar fMaxWidth;
  SkScalar fWidth;
  SkScalar fHeight;
  SkScalar fMinIntrinsicWidth;
};
