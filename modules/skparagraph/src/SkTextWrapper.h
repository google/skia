/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <string>
#include "SkLine.h"
#include "src/core/SkSpan.h"

class SkParagraphImpl;
<<<<<<< HEAD   (4d3028 format errors)
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
                this->fWhitespaces.fY =
                        SkTMax(this->fWhitespaces.fY, cluster.run()->calculateHeight());
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
                                  ? fTrimmedEnd->text().end() - start->text().begin()
                                  : 0;
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
            : fParent(parent), fLastSoftLineBreak(nullptr), fLastClusterBreak(nullptr) {
        reset();
    }

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
=======

class SkTextWrapper {
    class SkClusterPos {
    public:
        SkClusterPos() : fCluster(nullptr), fPos(0) {}
        SkClusterPos(SkCluster* cluster, size_t pos) : fCluster(cluster), fPos(pos) {}
        inline SkCluster* cluster() const { return fCluster; }
        inline size_t position() const { return fPos; }
        void move(bool up) {
            fCluster += up ? 1 : -1;
            fPos = up ? 0: fCluster->endPos();
        }
        void setPosition(size_t pos) { fPos = pos; }
        void clean() {
            fCluster = nullptr;
            fPos = 0;
        }

    private:
        SkCluster* fCluster;
        size_t fPos;
    };
    class SkTextStretch {
    public:
        SkTextStretch() : fStart(), fEnd(), fWidth(0) {}
        explicit SkTextStretch(SkCluster* s, SkCluster* e)
                : fStart(s, 0), fEnd(e, e->endPos()), fMetrics(), fWidth(0) {
            for (auto c = s; c <= e; ++c) {
                if (c->run() != nullptr) fMetrics.add(c->run());
            }
        }

        inline SkScalar width() const { return fWidth; }
        inline SkCluster* startCluster() const { return fStart.cluster(); }
        inline SkCluster* endCluster() const { return fEnd.cluster(); }
        inline SkCluster* breakCluster() const { return fBreak.cluster(); }
        inline SkLineMetrics& metrics() { return fMetrics; }
        inline size_t startPos() const { return fStart.position(); }
        inline size_t endPos() const { return fEnd.position(); }
        bool endOfCluster() { return fEnd.position() == fEnd.cluster()->endPos(); }
        bool endOfWord() {
            return endOfCluster() && (fEnd.cluster()->isHardBreak() || fEnd.cluster()->isSoftBreak());
        }

        void extend(SkTextStretch& stretch) {
            fMetrics.add(stretch.fMetrics);
            fEnd = stretch.fEnd;
            fWidth += stretch.fWidth;
            stretch.clean();
        }

        void extend(SkCluster* cluster) {
            fEnd = SkClusterPos(cluster, cluster->endPos());
            fMetrics.add(cluster->run());
            fWidth += cluster->width();
        }

        void extend(SkCluster* cluster, size_t pos) {
            fEnd = SkClusterPos(cluster, pos);
            if (cluster->run() != nullptr) {
                fMetrics.add(cluster->run());
            }
        }

        void startFrom(SkCluster* cluster, size_t pos) {
            fStart = SkClusterPos(cluster, pos);
            fEnd = SkClusterPos(cluster, pos);
            if (cluster->run() != nullptr) {
                fMetrics.add(cluster->run());
            }
            fWidth = 0;
        }

        void nextPos() {
            if (fEnd.position() == fEnd.cluster()->endPos()) {
                fEnd.move(true);
            } else {
                fEnd.setPosition(fEnd.cluster()->endPos());
            }
        }

        void saveBreak() {
            fBreak = fEnd;
        }

        void restoreBreak() {
            fEnd = fBreak;
        }

        void trim() {
            fWidth -= (fEnd.cluster()->width() - fEnd.cluster()->trimmedWidth(fEnd.position()));
        }

        void trim(SkCluster* cluster) {
            SkASSERT(fEnd.cluster() == cluster);
            fEnd.move(false);
            fWidth -= cluster->width();
        }

        void clean() {
            fStart.clean();
            fEnd.clean();
            fWidth = 0;
            fMetrics.clean();
        }

    private:
        SkClusterPos fStart;
        SkClusterPos fEnd;
        SkClusterPos fBreak;
        SkLineMetrics fMetrics;
        SkScalar fWidth;
    };

public:
    SkTextWrapper() { fLineNumber = 1; }

    using AddLineToParagraph = std::function<void(SkCluster* start,
                                                  SkCluster* end,
                                                  size_t startClip,
                                                  size_t endClip,
                                                  SkVector offset,
                                                  SkVector advance,
                                                  SkLineMetrics metrics,
                                                  bool addEllipsis)>;
    void breakTextIntoLines(SkParagraphImpl* parent,
                            SkSpan<SkCluster>
                                    span,
                            SkScalar maxWidth,
                            size_t maxLines,
                            const std::string& ellipsisStr,
                            const AddLineToParagraph& addLine);

    inline SkScalar height() const { return fHeight; }
    inline SkScalar intrinsicWidth() const { return fMinIntrinsicWidth; }

private:
    SkTextStretch fWords;
    SkTextStretch fClusters;
    SkTextStretch fClip;
    SkTextStretch fEndLine;
    size_t fLineNumber;
    bool fTooLongWord;
    bool fTooLongCluster;

    bool fHardLineBreak;

    SkScalar fHeight;
    SkScalar fMinIntrinsicWidth;

    void reset() {
        fWords.clean();
        fClusters.clean();
        fClip.clean();
        fTooLongCluster = false;
        fTooLongWord = false;
    }

    void lookAhead(SkScalar maxWidth, SkCluster* endOfClusters);
    void moveForward();
    void trimEndSpaces();
    void trimStartSpaces(SkCluster* endOfClusters);
};
>>>>>>> BRANCH (b84053 Addressed few comments from the code review)
