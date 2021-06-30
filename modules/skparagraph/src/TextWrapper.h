// Copyright 2019 Google LLC.
#ifndef TextWrapper_DEFINED
#define TextWrapper_DEFINED

#include <string>
#include "include/core/SkSpan.h"
#include "modules/skparagraph/src/TextLine.h"

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
                if (auto r = c->runOrNull()) {
                    fMetrics.add(r);
                }
                if (c < e) {
                    fWidth += c->width();
                }
            }
            fWidthWithGhostSpaces = fWidth;
        }

        inline SkScalar width() const { return fWidth; }
        SkScalar widthWithGhostSpaces() const { return fWidthWithGhostSpaces; }
        inline Cluster* startCluster() const { return fStart.cluster(); }
        inline Cluster* endCluster() const { return fEnd.cluster(); }
        inline Cluster* breakCluster() const { return fBreak.cluster(); }
        inline InternalLineMetrics& metrics() { return fMetrics; }
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

        bool empty() { return fStart.cluster() == fEnd.cluster() &&
                              fStart.position() == fEnd.position(); }

        void setMetrics(const InternalLineMetrics& metrics) { fMetrics = metrics; }

        void extend(Cluster* cluster) {
            if (fStart.cluster() == nullptr) {
                fStart = ClusterPos(cluster, cluster->startPos());
            }
            fEnd = ClusterPos(cluster, cluster->endPos());
            // TODO: Make sure all the checks are correct and there are no unnecessary checks
            auto& r = cluster->run();
            if (!cluster->isHardBreak() && !r.isPlaceholder()) {
                // We ignore metrics for \n as the Flutter does
                fMetrics.add(&r);
            }
            fWidth += cluster->width();
        }

        void extend(Cluster* cluster, size_t pos) {
            fEnd = ClusterPos(cluster, pos);
            if (auto r = cluster->runOrNull()) {
                fMetrics.add(r);
            }
        }

        void startFrom(Cluster* cluster, size_t pos) {
            fStart = ClusterPos(cluster, pos);
            fEnd = ClusterPos(cluster, pos);
            if (auto r = cluster->runOrNull()) {
                // In case of placeholder we should ignore the default text style -
                // we will pick up the correct one from the placeholder
                if (!r->isPlaceholder()) {
                    fMetrics.add(r);
                }
            }
            fWidth = 0;
        }

        void saveBreak() {
            fWidthWithGhostSpaces = fWidth;
            fBreak = fEnd;
        }

        void restoreBreak() {
            fWidth = fWidthWithGhostSpaces;
            fEnd = fBreak;
        }

        void trim() {

            if (fEnd.cluster() != nullptr &&
                fEnd.cluster()->owner() != nullptr &&
                fEnd.cluster()->runOrNull() != nullptr &&
                fEnd.cluster()->run().placeholderStyle() == nullptr &&
                fWidth > 0) {
                fWidth -= (fEnd.cluster()->width() - fEnd.cluster()->trimmedWidth(fEnd.position()));
            }
        }

        void trim(Cluster* cluster) {
            SkASSERT(fEnd.cluster() == cluster);
            if (fEnd.cluster() > fStart.cluster()) {
                fEnd.move(false);
                fWidth -= cluster->width();
            } else {
                fEnd.setPosition(fStart.position());
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
        InternalLineMetrics fMetrics;
        SkScalar fWidth;
        SkScalar fWidthWithGhostSpaces;
    };

public:
    TextWrapper() {
         fLineNumber = 1;
         fHardLineBreak = false;
         fExceededMaxLines = false;
    }

    using AddLineToParagraph = std::function<void(TextRange textExcludingSpaces,
                                                  TextRange text,
                                                  TextRange textIncludingNewlines,
                                                  ClusterRange clusters,
                                                  ClusterRange clustersWithGhosts,
                                                  SkScalar AddLineToParagraph,
                                                  size_t startClip,
                                                  size_t endClip,
                                                  SkVector offset,
                                                  SkVector advance,
                                                  InternalLineMetrics metrics,
                                                  bool addEllipsis)>;
    void breakTextIntoLines(ParagraphImpl* parent,
                            SkScalar maxWidth,
                            const AddLineToParagraph& addLine);

    SkScalar height() const { return fHeight; }
    SkScalar minIntrinsicWidth() const { return fMinIntrinsicWidth; }
    SkScalar maxIntrinsicWidth() const { return fMaxIntrinsicWidth; }
    bool exceededMaxLines() const { return fExceededMaxLines; }

private:
    TextStretch fWords;
    TextStretch fClusters;
    TextStretch fClip;
    TextStretch fEndLine;
    size_t fLineNumber;
    bool fTooLongWord;
    bool fTooLongCluster;

    bool fHardLineBreak;
    bool fExceededMaxLines;

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
    void moveForward(bool hasEllipsis);
    void trimEndSpaces(TextAlign align);
    std::tuple<Cluster*, size_t, SkScalar> trimStartSpaces(Cluster* endOfClusters);
    SkScalar getClustersTrimmedWidth();
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextWrapper_DEFINED
