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
        void setPosition(size_t pos) { fPos = pos; }
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

        SkScalar width() const { return fWidth; }
        SkScalar widthWithGhostSpaces() const { return fWidthWithGhostSpaces; }
        Cluster* startCluster() const { return fStart.cluster(); }
        Cluster* endCluster() const { return fEnd.cluster(); }
        Cluster* breakCluster() const { return fBreak.cluster(); }
        InternalLineMetrics& metrics() { return fMetrics; }
        size_t startPos() const { return fStart.position(); }
        size_t endPos() const { return fEnd.position(); }
        bool endOfCluster() { return fEnd.position() == fEnd.cluster()->endPos(); }
        bool endOfWord() {
            return endOfCluster() &&
                   (fEnd.cluster()->isHardBreak() || fEnd.cluster()->isSoftBreak());
        }

        void extend(TextStretch& stretch);
        void extend(Cluster* cluster);

        bool empty() { return fStart.cluster() == fEnd.cluster() &&
                              fStart.position() == fEnd.position(); }

        void setMetrics(const InternalLineMetrics& metrics) { fMetrics = metrics; }

        void startFrom(Cluster* cluster, size_t pos);

        void saveBreak() {
            fWidthWithGhostSpaces = fWidth;
            fBreak = fEnd;
        }

        void restoreBreak() {
            fWidth = fWidthWithGhostSpaces;
            fEnd = fBreak;
        }

        void trim();
        void trim(Cluster* cluster);

        void clean();

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

    using AddLineToParagraph = std::function<void(TextRange text,
                                                  TextRange textWithSpaces,
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
