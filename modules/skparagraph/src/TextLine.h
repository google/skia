// Copyright 2019 Google LLC.
#ifndef TextLine_DEFINED
#define TextLine_DEFINED

#include "include/core/SkCanvas.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

class TextLine {
public:

    struct ClipContext {
      const Run* run;
      size_t pos;
      size_t size;
      SkScalar fTextShift; // Shifts the text inside the run so it's placed at the right position
      SkRect clip;
      bool clippingNeeded;
    };

    TextLine() = default;
    ~TextLine() = default;

    TextLine(ParagraphImpl* master,
             SkVector offset,
             SkVector advance,
             BlockRange blocks,
             TextRange text,
             TextRange textWithSpaces,
             ClusterRange clusters,
             ClusterRange clustersWithGhosts,
             SkScalar widthWithSpaces,
             InternalLineMetrics sizes);

    void setMaster(ParagraphImpl* master) { fMaster = master; }

    TextRange trimmedText() const { return fTextRange; }
    TextRange textWithSpaces() const { return fTextWithWhitespacesRange; }
    ClusterRange clusters() const { return fClusterRange; }
    ClusterRange clustersWithSpaces() { return fGhostClusterRange; }
    Run* ellipsis() const { return fEllipsis.get(); }
    InternalLineMetrics sizes() const { return fSizes; }
    bool empty() const { return fTextRange.empty(); }

    SkScalar height() const { return fAdvance.fY; }
    SkScalar width() const {
        return fAdvance.fX + (fEllipsis != nullptr ? fEllipsis->fAdvance.fX : 0);
    }
    SkScalar shift() const { return fShift; }
    SkVector offset() const;

    SkScalar alphabeticBaseline() const { return fSizes.alphabeticBaseline(); }
    SkScalar ideographicBaseline() const { return fSizes.ideographicBaseline(); }
    SkScalar baseline() const { return fSizes.baseline(); }

    using RunVisitor = std::function<bool(const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width)>;
    void iterateThroughVisualRuns(bool includingGhostSpaces, const RunVisitor& runVisitor) const;
    using RunStyleVisitor = std::function<void(TextRange textRange, const TextStyle& style, const ClipContext& context)>;
    SkScalar iterateThroughSingleRunByStyles(const Run* run, SkScalar runOffset, TextRange textRange,
                                         StyleType styleType, const RunStyleVisitor& visitor) const;


    using ClustersVisitor = std::function<bool(const Cluster* cluster, ClusterIndex index, bool leftToRight, bool ghost)>;
    void iterateThroughClustersInGlyphsOrder(bool reverse, bool includeGhosts, const ClustersVisitor& visitor) const;

    void format(TextAlign effectiveAlign, SkScalar maxWidth);
    void paint(SkCanvas* canvas);

    void createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool ltr);

    // For testing internal structures
    void scanStyles(StyleType style, const RunStyleVisitor& visitor);

    TextAlign assumedTextAlign() const;

    void setMaxRunMetrics(const InternalLineMetrics& metrics) { fMaxRunMetrics = metrics; }
    InternalLineMetrics getMaxRunMetrics() const { return fMaxRunMetrics; }

    ClipContext measureTextInsideOneRun(TextRange textRange,
                                        const Run* run,
                                        SkScalar runOffsetInLine,
                                        SkScalar textOffsetInRunInLine,
                                        bool includeGhostSpaces,
                                        bool limitToClusters) const;

    LineMetrics getMetrics() const;

private:

    Run* shapeEllipsis(const SkString& ellipsis, Run* run);
    void justify(SkScalar maxWidth);

    void paintText(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const;
    void paintBackground(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const;
    void paintShadow(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const;
    void paintDecorations(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const;

    void computeDecorationPaint(SkPaint& paint, SkRect clip, const TextStyle& style, SkScalar thickness,
                                SkPath& path) const;

    bool contains(const Cluster* cluster) const {
        return fTextRange.contains(cluster->textRange());
    }

    ParagraphImpl* fMaster;
    BlockRange fBlockRange;
    TextRange fTextRange;
    TextRange fTextWithWhitespacesRange;
    ClusterRange fClusterRange;
    ClusterRange fGhostClusterRange;

    SkTArray<size_t, true> fRunsInVisualOrder;
    SkVector fAdvance;                  // Text size
    SkVector fOffset;                   // Text position
    SkScalar fShift;                    // Left right
    SkScalar fWidthWithSpaces;
    std::shared_ptr<Run> fEllipsis;     // In case the line ends with the ellipsis
    InternalLineMetrics fSizes;                 // Line metrics as a max of all run metrics and struts
    InternalLineMetrics fMaxRunMetrics;         // No struts - need it for GetRectForRange(max height)
    bool fHasBackground;
    bool fHasShadows;
    bool fHasDecorations;
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextLine_DEFINED
