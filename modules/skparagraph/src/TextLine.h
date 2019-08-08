// Copyright 2019 Google LLC.
#ifndef TextLine_DEFINED
#define TextLine_DEFINED

#include "include/core/SkCanvas.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

class TextLine {
public:
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
             LineMetrics sizes);

    void setMaster(ParagraphImpl* master) { fMaster = master; }

    TextRange trimmedText() const { return fTextRange; }
    TextRange textWithSpaces() const { return fTextWithWhitespacesRange; }
    ClusterRange clusters() const { return fClusterRange; }
    ClusterRange clustersWithSpaces() { return fGhostClusterRange; }
    Run* ellipsis() const { return fEllipsis.get(); }
    LineMetrics sizes() const { return fSizes; }
    bool empty() const { return fTextRange.empty(); }

    SkScalar height() const { return fAdvance.fY; }
    SkScalar width() const {
        return fAdvance.fX + (fEllipsis != nullptr ? fEllipsis->fAdvance.fX : 0);
    }
    SkScalar shift() const { return fShift; }
    SkScalar widthWithSpaces() const { return fWidthWithSpaces; }
    SkVector offset() const;

    SkScalar alphabeticBaseline() const { return fSizes.alphabeticBaseline(); }
    SkScalar ideographicBaseline() const { return fSizes.ideographicBaseline(); }
    SkScalar baseline() const { return fSizes.baseline(); }
    SkScalar roundingDelta() const { return fSizes.delta(); }

    using StyleVisitor = std::function<SkScalar(TextRange textRange, const TextStyle& style,
                                                SkScalar offsetX)>;
    void iterateThroughStylesInTextOrder(StyleType styleType, const StyleVisitor& visitor) const;

    SkScalar calculateLeftVisualOffset(TextRange textRange) const;

    using RunVisitor = std::function<bool(Run* run, size_t pos, size_t size, TextRange text,
                                          SkRect clip, SkScalar shift, bool clippingNeeded)>;
    SkScalar iterateThroughRuns(TextRange textRange,
                                SkScalar offsetX,
                                bool includeGhostWhitespaces,
                                const RunVisitor& visitor) const;

    using ClustersVisitor = std::function<bool(const Cluster* cluster, ClusterIndex index, bool leftToRight, bool ghost)>;
    void iterateThroughClustersInGlyphsOrder(bool reverse, bool includeGhosts, const ClustersVisitor& visitor) const;

    void format(TextAlign effectiveAlign, SkScalar maxWidth);
    void paint(SkCanvas* canvas);

    void createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool ltr);

    // For testing internal structures
    void scanStyles(StyleType style, const StyleVisitor& visitor);
    void scanRuns(const RunVisitor& visitor);

    TextAlign assumedTextAlign() const;

private:

    Run* shapeEllipsis(const SkString& ellipsis, Run* run);
    void justify(SkScalar maxWidth);

    SkRect measureTextInsideOneRun(TextRange textRange,
                                   Run* run,
                                   size_t& pos,
                                   size_t& size,
                                   bool includeGhostSpaces,
                                   bool& clippingNeeded) const;

    SkScalar paintText(SkCanvas* canvas, TextRange textRange, const TextStyle& style, SkScalar offsetX) const;
    SkScalar paintBackground(SkCanvas* canvas, TextRange textRange, const TextStyle& style,
                             SkScalar offsetX) const;
    SkScalar paintShadow(SkCanvas* canvas, TextRange textRange, const TextStyle& style,
                         SkScalar offsetX) const;
    SkScalar paintDecorations(SkCanvas* canvas, TextRange textRange, const TextStyle& style,
                              SkScalar offsetX) const;

    void computeDecorationPaint(SkPaint& paint, SkRect clip, const TextStyle& style,
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

    SkTArray<size_t, true> fLogical;
    SkVector fAdvance;                  // Text size
    SkVector fOffset;                   // Text position
    SkScalar fShift;                    // Left right
    SkScalar fWidthWithSpaces;
    std::shared_ptr<Run> fEllipsis;     // In case the line ends with the ellipsis
    LineMetrics fSizes;                 // Line metrics as a max of all run metrics
    bool fHasBackground;
    bool fHasShadows;
    bool fHasDecorations;

    // TODO: use for ellipsis the common cache
    static SkTHashMap<SkFont, Run> fEllipsisCache;  // All found so far shapes of ellipsis
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextLine_DEFINED
