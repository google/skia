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