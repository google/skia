// Copyright 2019 Google LLC.
#ifndef ParagraphImpl_DEFINED
#define ParagraphImpl_DEFINED

#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include <unicode/unistr.h>
#include <unicode/urename.h>
#include "include/core/SkPicture.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/FontResolver.h"
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

class TextBreaker {
public:
    TextBreaker() : fInitialized(false), fPos(-1) {}

    bool initialize(SkSpan<const char> text, UBreakIteratorType type);

    bool initialized() const { return fInitialized; }

    size_t first() {
        fPos = ubrk_first(fIterator.get());
        return eof() ? fSize : fPos;
    }

    size_t next() {
        fPos = ubrk_next(fIterator.get());
        return eof() ? fSize : fPos;
    }

    size_t preceding(size_t offset) {
        auto pos = ubrk_preceding(fIterator.get(), offset);
        return pos == icu::BreakIterator::DONE ? 0 : pos;
    }

    size_t following(size_t offset) {
        auto pos = ubrk_following(fIterator.get(), offset);
        return pos == icu::BreakIterator::DONE ? fSize : pos;
    }

    int32_t status() { return ubrk_getRuleStatus(fIterator.get()); }

    bool eof() { return fPos == icu::BreakIterator::DONE; }

private:
    std::unique_ptr<UBreakIterator, SkFunctionWrapper<decltype(ubrk_close), ubrk_close>> fIterator;
    bool fInitialized;
    int32_t fPos;
    size_t fSize;
};

class ParagraphImpl final : public Paragraph {

public:

    ParagraphImpl(const SkString& text,
                  ParagraphStyle style,
                  SkTArray<Block, true> blocks,
                  SkTArray<Placeholder, true> placeholders,
                  sk_sp<FontCollection> fonts);

    ParagraphImpl(const std::u16string& utf16text,
                  ParagraphStyle style,
                  SkTArray<Block, true> blocks,
                  SkTArray<Placeholder, true> placeholders,
                  sk_sp<FontCollection> fonts);
    ~ParagraphImpl() override;

    void layout(SkScalar width) override;
    void paint(SkCanvas* canvas, SkScalar x, SkScalar y) override;
    std::vector<TextBox> getRectsForRange(unsigned start,
                                          unsigned end,
                                          RectHeightStyle rectHeightStyle,
                                          RectWidthStyle rectWidthStyle) override;
    std::vector<TextBox> getRectsForPlaceholders() override;
    void getLineMetrics(std::vector<LineMetrics>&) override;
    PositionWithAffinity getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) override;
    SkRange<size_t> getWordBoundary(unsigned offset) override;

    size_t lineNumber() override { return fLines.size(); }

    TextLine& addLine(SkVector offset, SkVector advance, TextRange text, TextRange textWithSpaces,
                      ClusterRange clusters, ClusterRange clustersWithGhosts, SkScalar AddLineToParagraph,
                      InternalLineMetrics sizes);

    SkSpan<const char> text() const { return SkSpan<const char>(fText.c_str(), fText.size()); }
    InternalState state() const { return fState; }
    SkSpan<Run> runs() { return SkSpan<Run>(fRuns.data(), fRuns.size()); }
    const SkTArray<FontDescr>& switches() const { return fFontResolver.switches(); }
    SkSpan<Block> styles() {
        return SkSpan<Block>(fTextStyles.data(), fTextStyles.size());
    }
    SkSpan<TextLine> lines() { return SkSpan<TextLine>(fLines.data(), fLines.size()); }
    const ParagraphStyle& paragraphStyle() const { return fParagraphStyle; }
    SkSpan<Cluster> clusters() { return SkSpan<Cluster>(fClusters.begin(), fClusters.size()); }
    sk_sp<FontCollection> fontCollection() const { return fFontCollection; }
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
    bool strutHeightOverride() const {
        return paragraphStyle().getStrutStyle().getHeightOverride();
    }
    InternalLineMetrics strutMetrics() const { return fStrutMetrics; }

    Measurement measurement() {
        return {
            fAlphabeticBaseline,
            fIdeographicBaseline,
            fHeight,
            fWidth,
            fMaxIntrinsicWidth,
            fMinIntrinsicWidth,
            fLongestLine,
        };
    }
    void setMeasurement(Measurement m) {
        fAlphabeticBaseline = m.fAlphabeticBaseline;
        fIdeographicBaseline = m.fIdeographicBaseline;
        fHeight = m.fHeight;
        fWidth = m.fWidth;
        fMaxIntrinsicWidth = m.fMaxIntrinsicWidth;
        fMinIntrinsicWidth = m.fMinIntrinsicWidth;
        fLongestLine = m.fLongestLine;
    }

    SkSpan<const char> text(TextRange textRange);
    SkSpan<Cluster> clusters(ClusterRange clusterRange);
    Cluster& cluster(ClusterIndex clusterIndex);
    Run& run(RunIndex runIndex);
    Run& runByCluster(ClusterIndex clusterIndex);
    SkSpan<Block> blocks(BlockRange blockRange);
    Block& block(BlockIndex blockIndex);

    void markDirty() override { fState = kUnknown; }
    FontResolver& getResolver() { return fFontResolver; }
    void setState(InternalState state);
    sk_sp<SkPicture> getPicture() { return fPicture; }

    using ShapeVisitor =
            std::function<SkScalar(SkSpan<const char>, SkSpan<Block>, SkScalar&, size_t)>;
    bool iterateThroughShapingRegions(ShapeVisitor shape);

    void resetContext();
    void resolveStrut();
    void resetRunShifts();
    void buildClusterTable();
    void markLineBreaks();
    bool shapeTextIntoEndlessLine();
    void breakShapedTextIntoLines(SkScalar maxWidth);
    void paintLinesIntoPicture();

    void updateTextAlign(TextAlign textAlign) override;
    void updateText(size_t from, SkString text) override;
    void updateFontSize(size_t from, size_t to, SkScalar fontSize) override;
    void updateForegroundPaint(size_t from, size_t to, SkPaint paint) override;
    void updateBackgroundPaint(size_t from, size_t to, SkPaint paint) override;

    InternalLineMetrics computeEmptyMetrics();
    InternalLineMetrics getStrutMetrics() const { return fStrutMetrics; }

private:
    friend class ParagraphBuilder;
    friend class ParagraphCacheKey;
    friend class ParagraphCacheValue;
    friend class ParagraphCache;

    friend class TextWrapper;

    BlockRange findAllBlocks(TextRange textRange);
    void extractStyles();

    void markGraphemes();

    // Input
    SkTArray<StyleBlock<SkScalar>> fLetterSpaceStyles;
    SkTArray<StyleBlock<SkScalar>> fWordSpaceStyles;
    SkTArray<StyleBlock<SkPaint>> fBackgroundStyles;
    SkTArray<StyleBlock<SkPaint>> fForegroundStyles;
    SkTArray<StyleBlock<std::vector<TextShadow>>> fShadowStyles;
    SkTArray<StyleBlock<Decoration>> fDecorationStyles;
    SkTArray<Block, true> fTextStyles; // TODO: take out only the font stuff
    SkTArray<Placeholder, true> fPlaceholders;
    SkString fText;

    // Internal structures
    InternalState fState;
    SkTArray<Run> fRuns;                // kShaped
    SkTArray<Cluster, true> fClusters;  // kClusterized (cached: text, word spacing, letter spacing, resolved fonts)
    SkTArray<Grapheme, true> fGraphemes;
    SkTArray<Codepoint, true> fCodePoints;

    SkTArray<RunShifts, true> fRunShifts;
    SkTArray<TextLine, true> fLines;    // kFormatted   (cached: width, max lines, ellipsis, text align)
    sk_sp<SkPicture> fPicture;          // kRecorded    (cached: text styles)

    InternalLineMetrics fStrutMetrics;
    FontResolver fFontResolver;

    SkScalar fOldWidth;
    SkScalar fOldHeight;
    SkScalar fMaxWidthWithTrailingSpaces;

    std::vector<size_t> fWords;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphImpl_DEFINED
