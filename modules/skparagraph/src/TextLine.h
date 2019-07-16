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
             LineMetrics sizes);

    void setMaster(ParagraphImpl* master) { fMaster = master; }

    TextRange trimmedText() const { return fTextRange; }
    TextRange textWithSpaces() const { return fTextWithWhitespacesRange; }
    SkVector offset() const { return fOffset + SkVector::Make(fShift, 0); }
    Run* ellipsis() const { return fEllipsis.get(); }
    LineMetrics sizes() const { return fSizes; }
    bool empty() const { return fTextRange.empty(); }

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

    using StyleVisitor = std::function<SkScalar(TextRange textRange, const TextStyle& style,
                                                SkScalar offsetX)>;
    void iterateThroughStylesInTextOrder(StyleType styleType, const StyleVisitor& visitor) const;

    using RunVisitor = std::function<bool(Run* run, size_t pos, size_t size, SkRect clip,
                                          SkScalar shift, bool clippingNeeded)>;
    SkScalar iterateThroughRuns(TextRange textRange,
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

    void resetCacheRecords() {
        fTextBlobRecords.reset();
        fShadowRecords.reset();
        fDecorationRecords.reset();
    }

private:
    struct TextBlobRecord {
        sk_sp<SkTextBlob> fTextBlob;
        bool fClipped;
        SkRect fClip;
        SkScalar fShift;
        SkPaint fPaint;

        TextBlobRecord(sk_sp<SkTextBlob> textBlob,
                       bool clipped,
                       SkRect clip,
                       SkScalar shift,
                       const SkPaint& paint)
           : fTextBlob(std::move(textBlob))
           , fClipped(clipped)
           , fClip(clip)
           , fShift(shift)
           , fPaint(paint) { };

        ~TextBlobRecord() = default;

        void drawText (SkCanvas* canvas) {
            canvas->save();
            if (this->fClipped) {
                canvas->clipRect(this->fClip);
            }
            canvas->translate(this->fShift, 0);
            canvas->drawTextBlob(this->fTextBlob, 0, 0, fPaint);
            canvas->restore();
        };
    };
    
    // TODO: add the other drawing parts to cache
    struct ShadowRecord {
        
    };
    
    struct DecorationRecord {
        
    };

    Run* shapeEllipsis(const SkString& ellipsis, Run* run);
    void justify(SkScalar maxWidth);

    SkRect measureTextInsideOneRun(TextRange textRange,
                                   Run* run,
                                   size_t& pos,
                                   size_t& size,
                                   bool& clippingNeeded) const;

    SkScalar buildText(TextRange textRange, const TextStyle& style, SkScalar offsetX) const;
    SkScalar paintBackground(SkCanvas* canvas,TextRange textRange, const TextStyle& style,
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

    SkTArray<size_t, true> fLogical;
    SkScalar fShift;                    // Shift to left - right - center
    SkVector fAdvance;                  // Text size
    SkVector fOffset;                   // Text position
    std::shared_ptr<Run> fEllipsis;     // In case the line ends with the ellipsis
    LineMetrics fSizes;                 // Line metrics as a max of all run metrics
    bool fHasBackground;
    bool fHasShadows;
    bool fHasDecorations;
    
    // Cached drawing parts
    mutable SkTArray<TextBlobRecord> fTextBlobRecords;
    mutable SkTArray<ShadowRecord> fShadowRecords;
    mutable SkTArray<DecorationRecord> fDecorationRecords;

    static SkTHashMap<SkFont, Run> fEllipsisCache;  // All found so far shapes of ellipsis
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextLine_DEFINED
