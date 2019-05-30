// Copyright 2019 Google LLC.
#ifndef TextLine_DEFINED
#define TextLine_DEFINED

#include "Run.h"
#include "include/core/SkCanvas.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

class TextBlock {
public:
    TextBlock() : fText(), fTextStyle() {}
    TextBlock(SkSpan<const char> text, const TextStyle& style) : fText(text), fTextStyle(style) {}

    SkSpan<const char> text() const { return fText; }
    TextStyle style() const { return fTextStyle; }

    void add(SkSpan<const char> tail) {
        SkASSERT(fText.end() == tail.begin());
        fText = SkSpan<const char>(fText.begin(), fText.size() + tail.size());
    }

protected:
    SkSpan<const char> fText;
    TextStyle fTextStyle;
};

class TextLine {
public:
    TextLine() = default;
    TextLine(TextLine&&);
    ~TextLine() = default;

    TextLine(SkVector offset, SkVector advance, SkSpan<const TextBlock> blocks,
             SkSpan<const char> text, SkSpan<const char> textWithSpaces,
             SkSpan<const Cluster> clusters, size_t start, size_t end, LineMetrics sizes);

    SkSpan<const char> trimmedText() const { return fText; }
     SkSpan<const char> textWithSpaces() const { return fTextWithSpaces; }
    SkSpan<const Cluster> clusters() const { return fClusters; }
    SkVector offset() const { return fOffset + SkVector::Make(fShift, 0); }
    Run* ellipsis() const { return fEllipsis.get(); }
    LineMetrics sizes() const { return fSizes; }
    bool empty() const { return fText.empty(); }

    SkScalar shift() const { return fShift; }
    SkScalar height() const { return fAdvance.fY; }
    SkScalar width() const {
        return fAdvance.fX + (fEllipsis != nullptr ? fEllipsis->fAdvance.fX : 0);
    }
    void shiftTo(SkScalar shift) { fShift = shift; }

    SkScalar alphabeticBaseline() const { return fSizes.alphabeticBaseline(); }
    SkScalar ideographicBaseline() const { return fSizes.ideographicBaseline(); }
    SkScalar baseline() const { return fSizes.baseline(); }
    SkScalar roundingDelta() const { return fSizes.delta(); }

    using StyleVisitor = std::function<SkScalar(SkSpan<const char> text, const TextStyle& style,
                                                SkScalar offsetX)>;
    void iterateThroughStylesInTextOrder(StyleType styleType, const StyleVisitor& visitor) const;

    using RunVisitor = std::function<bool(Run* run, size_t pos, size_t size, SkRect clip,
                                          SkScalar shift, bool clippingNeeded)>;
    SkScalar iterateThroughRuns(SkSpan<const char> text,
                                SkScalar offsetX,
                                bool includeEmptyText,
                                const RunVisitor& visitor) const;

    using ClustersVisitor = std::function<bool(const Cluster* cluster)>;
    void iterateThroughClustersInGlyphsOrder(bool reverse, const ClustersVisitor& visitor) const;

    void format(TextAlign effectiveAlign, SkScalar maxWidth, bool last);
    void paint(SkCanvas* canvas);

    void createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool ltr);

    // For testing internal structures
    void scanStyles(StyleType style, const StyleVisitor& visitor);
    void scanRuns(const RunVisitor& visitor);

private:
    Run* shapeEllipsis(const SkString& ellipsis, Run* run);
    void justify(SkScalar maxWidth);

    SkRect measureTextInsideOneRun(SkSpan<const char> text,
                                   Run* run,
                                   size_t& pos,
                                   size_t& size,
                                   bool& clippingNeeded) const;

    SkScalar paintText(SkCanvas* canvas, SkSpan<const char> text, const TextStyle& style,
                       SkScalar offsetX) const;
    SkScalar paintBackground(SkCanvas* canvas, SkSpan<const char> text, const TextStyle& style,
                             SkScalar offsetX) const;
    SkScalar paintShadow(SkCanvas* canvas, SkSpan<const char> text, const TextStyle& style,
                         SkScalar offsetX) const;
    SkScalar paintDecorations(SkCanvas* canvas, SkSpan<const char> text, const TextStyle& style,
                              SkScalar offsetX) const;

    void computeDecorationPaint(SkPaint& paint, SkRect clip, const TextStyle& style,
                                SkPath& path) const;

    bool contains(const Cluster* cluster) const {
        return cluster->text().begin() >= fText.begin() && cluster->text().end() <= fText.end();
    }

    SkSpan<const TextBlock> fBlocks;
    SkSpan<const char> fText;
    SkSpan<const char> fTextWithSpaces;
    SkSpan<const Cluster> fClusters;
    // TODO: To clip by glyph:
    //size_t fStartPos;
    //size_t fEndPos;
    SkTArray<Run*, true> fLogical;
    SkScalar fShift;                   // Shift to left - right - center
    SkVector fAdvance;                 // Text size
    SkVector fOffset;                  // Text position
    std::unique_ptr<Run> fEllipsis;  // In case the line ends with the ellipsis
    LineMetrics fSizes;              // Line metrics as a max of all run metrics

    static SkTHashMap<SkFont, Run> fEllipsisCache;  // All found so far shapes of ellipsis
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextLine_DEFINED
