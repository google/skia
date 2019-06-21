// Copyright 2019 Google LLC.
#ifndef ParagraphImpl_DEFINED
#define ParagraphImpl_DEFINED

#include <include/private/SkMutex.h>
#include "FontResolver.h"
#include "include/core/SkPicture.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphCache.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"

class SkCanvas;

namespace skia {
namespace textlayout {

template <typename T> bool operator==(const SkSpan<T>& a, const SkSpan<T>& b) {
    return a.size() == b.size() && a.begin() == b.begin();
}

template <typename T> bool operator<=(const SkSpan<T>& a, const SkSpan<T>& b) {
    return a.begin() >= b.begin() && a.end() <= b.end();
}

template <typename TStyle>
struct StyleBlock {
    StyleBlock() : fRange(EMPTY_RANGE), fStyle() { }
    StyleBlock(size_t start, size_t end, const TStyle& style) : fRange(start, end), fStyle(style) {}
    StyleBlock(TextRange textRange, const TStyle& style) : fRange(textRange), fStyle(style) {}
    void add(TextRange tail) {
        SkASSERT(fRange.end == tail.start);
        fRange = TextRange(fRange.start, fRange.start + fRange.width() + tail.width());
    }
    TextRange fRange;
    TStyle fStyle;
};

class ParagraphImpl final : public Paragraph {
public:

    ParagraphImpl(const SkString& text,
                  ParagraphStyle style,
                  SkTArray<Block, true> blocks,
                  sk_sp<FontCollection> fonts);

    ParagraphImpl(const std::u16string& utf16text,
                    ParagraphStyle style,
                    SkTArray<Block, true> blocks,
                    sk_sp<FontCollection> fonts);
    ~ParagraphImpl() override;

    void layout(SkScalar width) override;
    void paint(SkCanvas* canvas, SkScalar x, SkScalar y) override;
    std::vector<TextBox> getRectsForRange(unsigned start,
                                          unsigned end,
                                          RectHeightStyle rectHeightStyle,
                                          RectWidthStyle rectWidthStyle) override;
    PositionWithAffinity getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) override;
    SkRange<size_t> getWordBoundary(unsigned offset) override;
    bool didExceedMaxLines() override {
        return !fParagraphStyle.unlimited_lines() && fLines.size() > fParagraphStyle.getMaxLines();
    }

    size_t lineNumber() override { return fLines.size(); }

    TextLine& addLine(SkVector offset, SkVector advance, TextRange text, TextRange textWithSpaces,
                      ClusterRange clusters, LineMetrics sizes);

    SkSpan<const char> text() const { return fTextSpan; }
    InternalState state() const { return fState; }
    SkSpan<Run> runs() { return SkSpan<Run>(fRuns.data(), fRuns.size()); }
    SkTArray<FontDescr>& switches() { return fFontResolver.switches(); }
    SkSpan<Block> styles() {
        return SkSpan<Block>(fTextStyles.data(), fTextStyles.size());
    }
    SkSpan<TextLine> lines() { return SkSpan<TextLine>(fLines.data(), fLines.size()); }
    ParagraphStyle paragraphStyle() const { return fParagraphStyle; }
    SkSpan<Cluster> clusters() { return SkSpan<Cluster>(fClusters.begin(), fClusters.size()); }
    void formatLines(SkScalar maxWidth);

    void shiftCluster(ClusterIndex index, SkScalar shift) {
        auto& cluster = fClusters[index];
        auto& run = fRunShifts[cluster.runIndex()];
        for (size_t pos = cluster.startPos(); pos < cluster.endPos(); ++pos) {
            run.fShifts[pos] = shift;
        }
    }

    SkScalar posShift(RunIndex index, size_t pos) const {
        if (fRunShifts.count() == 0) return 0.0;
        return fRunShifts[index].fShifts[pos];
    }

    SkScalar lineShift(size_t index) { return fLines[index].shift(); }

    bool strutEnabled() const { return paragraphStyle().getStrutStyle().getStrutEnabled(); }
    bool strutForceHeight() const {
        return paragraphStyle().getStrutStyle().getForceStrutHeight();
    }
    LineMetrics strutMetrics() const { return fStrutMetrics; }

    Measurement measurement() {
        return {
            fAlphabeticBaseline,
            fIdeographicBaseline,
            fHeight,
            fWidth,
            fMaxIntrinsicWidth,
            fMinIntrinsicWidth,
        };
    }
    void setMeasurement(Measurement m) {
        fAlphabeticBaseline = m.fAlphabeticBaseline;
        fIdeographicBaseline = m.fIdeographicBaseline;
        fHeight = m.fHeight;
        fWidth = m.fWidth;
        fMaxIntrinsicWidth = m.fMaxIntrinsicWidth;
        fMinIntrinsicWidth = m.fMinIntrinsicWidth;
    }

    SkSpan<const char> text(TextRange textRange);
    SkSpan<Cluster> clusters(ClusterRange clusterRange);
    Cluster& cluster(ClusterIndex clusterIndex);
    Run& run(RunIndex runIndex);
    SkSpan<Block> blocks(BlockRange blockRange);
    Block& block(BlockIndex blockIndex);

    void markDirty() override { fState = kUnknown; }
    void turnOnCache(bool on) { fParagraphCacheOn = on; }
    void setState(InternalState state);
    void resetCache() { fParagraphCache.reset(); }
    sk_sp<SkPicture> getPicture() { return fPicture; }

    void resetContext();
    void resolveStrut();
    void resetRunShifts();
    void buildClusterTable();
    void markLineBreaks();
    bool shapeTextIntoEndlessLine();
    void breakShapedTextIntoLines(SkScalar maxWidth);
    void paintLinesIntoPicture();

private:
    friend class ParagraphBuilder;
    friend class ParagraphCacheKey;
    friend class ParagraphCacheValue;
    friend class ParagraphCache;

    BlockRange findAllBlocks(TextRange textRange);
    void extractStyles();

    // Input
    SkTArray<StyleBlock<SkScalar>> fLetterSpaceStyles;
    SkTArray<StyleBlock<SkScalar>> fWordSpaceStyles;
    SkTArray<StyleBlock<SkPaint>> fBackgroundStyles;
    SkTArray<StyleBlock<SkPaint>> fForegroundStyles;
    SkTArray<StyleBlock<std::vector<TextShadow>>> fShadowStyles;
    SkTArray<StyleBlock<Decoration>> fDecorationStyles;
    SkTArray<Block, true> fTextStyles; // TODO: take out only the font stuff
    SkString fText;
    SkSpan<const char> fTextSpan;

    // Internal structures
    InternalState fState;
    SkTArray<Run> fRuns;                // kShaped
    SkTArray<Cluster, true> fClusters;  // kClusterized (cached: text, word spacing, letter spacing, resolved fonts)

    SkTArray<RunShifts, true> fRunShifts;
    SkTArray<TextLine, true> fLines;    // kFormatted   (cached: width, max lines, ellipsis, text align)
    sk_sp<SkPicture> fPicture;          // kRecorded    (cached: text styles)

    LineMetrics fStrutMetrics;
    FontResolver fFontResolver;

    SkScalar fOldWidth;
    SkScalar fOldHeight;

    // Cache
    bool fParagraphCacheOn;
    static ParagraphCache fParagraphCache;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphImpl_DEFINED
