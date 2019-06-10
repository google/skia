// Copyright 2019 Google LLC.
#ifndef TextWrapper_DEFINED
#define TextWrapper_DEFINED

#include <string>
#include "modules/skparagraph/src/TextLine.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

class ParagraphImpl;

class TextWrapper {
    class ClusterPos {
    public:
        ClusterPos() : fCluster(nullptr), fPos(0) {}
        ClusterPos(Cluster* cluster, size_t pos) : fCluster(cluster), fPos(pos) {}
        Cluster* cluster() const { return fCluster; }
        size_t position() const { return fPos; }
        void move(bool up) {
            fCluster += up ? 1 : -1;
            fPos = up ? 0 : fCluster->endPos();
        }
        void setPosition(size_t pos) { fPos = pos; }
        void clean() {
            fCluster = nullptr;
            fPos = 0;
        }

    private:
        Cluster* fCluster;
        size_t fPos;
    };
    class TextStretch {
    public:
        TextStretch() : fStart(), fEnd(), fWidth(0) {}
        explicit TextStretch(Cluster* s, Cluster* e)
                : fStart(s, 0), fEnd(e, e->endPos()), fMetrics(), fWidth(0) {
            for (auto c = s; c <= e; ++c) {
                if (c->run() != nullptr) {
                    fMetrics.add(c->run());
                }
            }
        }

        SkScalar width() const { return fWidth; }
        Cluster* startCluster() const { return fStart.cluster(); }
        Cluster* endCluster() const { return fEnd.cluster(); }
        Cluster* breakCluster() const { return fBreak.cluster(); }
        LineMetrics& metrics() { return fMetrics; }
        size_t startPos() const { return fStart.position(); }
        size_t endPos() const { return fEnd.position(); }
        bool endOfCluster() { return fEnd.position() == fEnd.cluster()->endPos(); }
        bool endOfWord() {
            return endOfCluster() &&
                   (fEnd.cluster()->isHardBreak() || fEnd.cluster()->isSoftBreak());
        }

        void extend(TextStretch& stretch) {
            fMetrics.add(stretch.fMetrics);
            fEnd = stretch.fEnd;
            fWidth += stretch.fWidth;
            stretch.clean();
        }

        void extend(Cluster* cluster) {
            fEnd = ClusterPos(cluster, cluster->endPos());
            fMetrics.add(cluster->run());
            fWidth += cluster->width();
        }

        void extend(Cluster* cluster, size_t pos) {
            fEnd = ClusterPos(cluster, pos);
            if (cluster->run() != nullptr) {
                fMetrics.add(cluster->run());
            }
        }

        void startFrom(Cluster* cluster, size_t pos) {
            fStart = ClusterPos(cluster, pos);
            fEnd = ClusterPos(cluster, pos);
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

        void saveBreak() { fBreak = fEnd; }

        void restoreBreak() { fEnd = fBreak; }

        void trim() {
            fWidth -= (fEnd.cluster()->width() - fEnd.cluster()->trimmedWidth(fEnd.position()));
        }

        void trim(Cluster* cluster) {
            SkASSERT(fEnd.cluster() == cluster);
            if (fEnd.cluster() > fStart.cluster()) {
                fEnd.move(false);
                fWidth -= cluster->width();
            } else {
                fWidth = 0;
            }
        }

        void clean() {
            fStart.clean();
            fEnd.clean();
            fWidth = 0;
            fMetrics.clean();
        }

    private:
        ClusterPos fStart;
        ClusterPos fEnd;
        ClusterPos fBreak;
        LineMetrics fMetrics;
        SkScalar fWidth;
    };

public:
    TextWrapper() { fLineNumber = 1; }

    using AddLineToParagraph = std::function<void(SkSpan<const char> text,
                                                  SkSpan<const char> textWithSpaces,
                                                  Cluster* start,
                                                  Cluster* end,
                                                  size_t startClip,
                                                  size_t endClip,
                                                  SkVector offset,
                                                  SkVector advance,
                                                  LineMetrics metrics,
                                                  bool addEllipsis)>;
    void breakTextIntoLines(ParagraphImpl* parent,
                            SkScalar maxWidth,
                            const AddLineToParagraph& addLine);

    SkScalar height() const { return fHeight; }
    SkScalar minIntrinsicWidth() const { return fMinIntrinsicWidth; }
    SkScalar maxIntrinsicWidth() const { return fMaxIntrinsicWidth; }

private:
    TextStretch fWords;
    TextStretch fClusters;
    TextStretch fClip;
    TextStretch fEndLine;
    size_t fLineNumber;
    bool fTooLongWord;
    bool fTooLongCluster;

    bool fHardLineBreak;

    SkScalar fHeight;
    SkScalar fMinIntrinsicWidth;
    SkScalar fMaxIntrinsicWidth;

    void reset() {
        fWords.clean();
        fClusters.clean();
        fClip.clean();
        fTooLongCluster = false;
        fTooLongWord = false;
    }

    void lookAhead(SkScalar maxWidth, Cluster* endOfClusters);
    void moveForward();
    void trimEndSpaces(bool includingClusters);
    void trimStartSpaces(Cluster* endOfClusters);
    SkScalar getClustersTrimmedWidth();
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextWrapper_DEFINED
