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
        inline Cluster* cluster() const { return fCluster; }
        inline size_t position() const { return fPos; }
        inline void setPosition(size_t pos) { fPos = pos; }
        void clean() {
            fCluster = nullptr;
            fPos = 0;
        }
        void move(bool up) {
            fCluster += up ? 1 : -1;
            fPos = up ? 0 : fCluster->endPos();
}

    private:
        Cluster* fCluster;
        size_t fPos;
    };
    class TextStretch {
    public:
        TextStretch() : fStart(), fEnd(), fWidth(0), fWidthWithGhostSpaces(0) {}
        TextStretch(Cluster* s, Cluster* e, bool forceStrut)
                : fStart(s, 0), fEnd(e, e->endPos()), fMetrics(forceStrut), fWidth(0), fWidthWithGhostSpaces(0) {
            for (auto c = s; c <= e; ++c) {
                if (c->run() != nullptr) {
                    fMetrics.add(c->run());
                }
            }
        }

        inline SkScalar width() const { return fWidth; }
        SkScalar withWithGhostSpaces() const { return fWidthWithGhostSpaces; }
        inline Cluster* startCluster() const { return fStart.cluster(); }
        inline Cluster* endCluster() const { return fEnd.cluster(); }
        inline Cluster* breakCluster() const { return fBreak.cluster(); }
        inline LineMetrics& metrics() { return fMetrics; }
        inline size_t startPos() const { return fStart.position(); }
        inline size_t endPos() const { return fEnd.position(); }
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
            if (fStart.cluster() == nullptr) {
                fStart = ClusterPos(cluster, cluster->startPos());
            }
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

        void saveBreak() {
            fWidthWithGhostSpaces = fWidth;
            fBreak = fEnd;
        }

        void trim() {

            if (fEnd.cluster() != nullptr &&
                fEnd.cluster()->master() != nullptr &&
                fEnd.cluster()->run() != nullptr &&
                fEnd.cluster()->run()->placeholder() == nullptr) {
                fWidth -= (fEnd.cluster()->width() - fEnd.cluster()->trimmedWidth(fEnd.position()));
            }
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
        SkScalar fWidthWithGhostSpaces;
    };

public:
    TextWrapper() { fLineNumber = 1; }

    using AddLineToParagraph = std::function<void(TextRange text,
                                                  TextRange textWithSpaces,
                                                  ClusterRange clusters,
                                                  ClusterRange clustersWithGhosts,
                                                  SkScalar AddLineToParagraph,
                                                  size_t startClip,
                                                  size_t endClip,
                                                  SkVector offset,
                                                  SkVector advance,
                                                  LineMetrics metrics,
                                                  bool addEllipsis)>;
    void breakTextIntoLines(ParagraphImpl* parent,
                            SkScalar maxWidth,
                            const AddLineToParagraph& addLine);

    inline SkScalar height() const { return fHeight; }
    inline SkScalar minIntrinsicWidth() const { return fMinIntrinsicWidth; }
    inline SkScalar maxIntrinsicWidth() const { return fMaxIntrinsicWidth; }

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
    void trimEndSpaces(TextAlign align);
    std::tuple<Cluster*, size_t, SkScalar> trimStartSpaces(Cluster* endOfClusters);
    SkScalar getClustersTrimmedWidth();
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextWrapper_DEFINED
