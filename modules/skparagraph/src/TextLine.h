// Copyright 2019 Google LLC.
#ifndef TextLine_DEFINED
#define TextLine_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/private/SkTArray.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Run.h"

#include <stddef.h>
#include <functional>
#include <memory>
#include <vector>

class SkCanvas;
class SkString;

namespace skia {
namespace textlayout {

class ParagraphImpl;

class TextLine {
public:

    struct ClipContext {
      const Run* run;
      size_t pos;
      size_t size;
      SkScalar fTextShift; // Shifts the text inside the run so it's placed at the right position
      SkRect clip;
      SkScalar fExcludedTrailingSpaces;
      bool clippingNeeded;
    };

    TextLine() = default;
    TextLine(const TextLine&) = delete;
    TextLine& operator=(const TextLine&) = delete;
    TextLine(TextLine&&) = default;
    TextLine& operator=(TextLine&&) = default;
    ~TextLine() = default;

    TextLine(ParagraphImpl* owner,
             SkVector offset,
             SkVector advance,
             BlockRange blocks,
             TextRange textExcludingSpaces,
             TextRange text,
             TextRange textIncludingNewlines,
             ClusterRange clusters,
             ClusterRange clustersWithGhosts,
             SkScalar widthWithSpaces,
             InternalLineMetrics sizes);

    TextRange trimmedText() const { return fTextExcludingSpaces; }
    TextRange textWithNewlines() const { return fTextIncludingNewlines; }
    TextRange text() const { return fText; }
    ClusterRange clusters() const { return fClusterRange; }
    ClusterRange clustersWithSpaces() { return fGhostClusterRange; }
    Run* ellipsis() const { return fEllipsis.get(); }
    InternalLineMetrics sizes() const { return fSizes; }
    bool empty() const { return fTextExcludingSpaces.empty(); }

    SkScalar spacesWidth() const { return fWidthWithSpaces - width(); }
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

    using ClustersVisitor = std::function<bool(const Cluster* cluster, bool ghost)>;
    void iterateThroughClustersInGlyphsOrder(bool reverse, bool includeGhosts, const ClustersVisitor& visitor) const;

    void format(TextAlign align, SkScalar maxWidth);
    SkRect paint(SkCanvas* canvas, SkScalar x, SkScalar y);
    void visit(SkScalar x, SkScalar y);
    void ensureTextBlobCachePopulated();

    void createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool ltr);

    // For testing internal structures
    void scanStyles(StyleType style, const RunStyleVisitor& visitor);

    void setMaxRunMetrics(const InternalLineMetrics& metrics) { fMaxRunMetrics = metrics; }
    InternalLineMetrics getMaxRunMetrics() const { return fMaxRunMetrics; }

    bool isFirstLine();
    bool isLastLine();
    void getRectsForRange(TextRange textRange, RectHeightStyle rectHeightStyle, RectWidthStyle rectWidthStyle, std::vector<TextBox>& boxes);
    void getRectsForPlaceholders(std::vector<TextBox>& boxes);
    PositionWithAffinity getGlyphPositionAtCoordinate(SkScalar dx);

    ClipContext measureTextInsideOneRun(TextRange textRange,
                                        const Run* run,
                                        SkScalar runOffsetInLine,
                                        SkScalar textOffsetInRunInLine,
                                        bool includeGhostSpaces,
                                        bool limitToGraphemes) const;

    LineMetrics getMetrics() const;

    SkRect extendHeight(const ClipContext& context) const;

    SkScalar metricsWithoutMultiplier(TextHeightBehavior correction);
    void shiftVertically(SkScalar shift) { fOffset.fY += shift; }

    bool endsWithHardLineBreak() const;

private:

    std::unique_ptr<Run> shapeEllipsis(const SkString& ellipsis, const Run& run);
    void justify(SkScalar maxWidth);

    void buildTextBlob(TextRange textRange, const TextStyle& style, const ClipContext& context);
    void paintBackground(SkCanvas* canvas, SkScalar x, SkScalar y, TextRange textRange, const TextStyle& style, const ClipContext& context) const;
    SkRect paintShadow(SkCanvas* canvas, SkScalar x, SkScalar y, TextRange textRange, const TextStyle& style, const ClipContext& context) const;
    void paintDecorations(SkCanvas* canvas, SkScalar x, SkScalar y, TextRange textRange, const TextStyle& style, const ClipContext& context) const;

    void shiftCluster(const Cluster* cluster, SkScalar shift, SkScalar prevShift);

    ParagraphImpl* fOwner;
    BlockRange fBlockRange;
    TextRange fTextExcludingSpaces;
    TextRange fText;
    TextRange fTextIncludingNewlines;
    ClusterRange fClusterRange;
    ClusterRange fGhostClusterRange;
    // Avoid the malloc/free in the common case of one run per line
    SkSTArray<1, size_t, true> fRunsInVisualOrder;
    SkVector fAdvance;                  // Text size
    SkVector fOffset;                   // Text position
    SkScalar fShift;                    // Let right
    SkScalar fWidthWithSpaces;
    std::unique_ptr<Run> fEllipsis;     // In case the line ends with the ellipsis
    InternalLineMetrics fSizes;                 // Line metrics as a max of all run metrics and struts
    InternalLineMetrics fMaxRunMetrics;         // No struts - need it for GetRectForRange(max height)
    bool fHasBackground;
    bool fHasShadows;
    bool fHasDecorations;

    LineMetricStyle fAscentStyle;
    LineMetricStyle fDescentStyle;

    struct TextBlobRecord {
        void paint(SkCanvas* canvas, SkScalar x, SkScalar y);

        sk_sp<SkTextBlob> fBlob;
        SkPoint fOffset = SkPoint::Make(0.0f, 0.0f);
        SkPaint fPaint;
        SkRect fBounds = SkRect::MakeEmpty();
        bool fClippingNeeded = false;
        SkRect fClipRect = SkRect::MakeEmpty();

        // Extra fields only used for the (experimental) visitor
        const Run* fVisitor_Run;
        size_t     fVisitor_Pos;
    };
    bool fTextBlobCachePopulated;
public:
    std::vector<TextBlobRecord> fTextBlobCache;
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextLine_DEFINED
