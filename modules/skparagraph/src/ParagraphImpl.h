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

class ParagraphImpl final : public Paragraph {
public:

    ParagraphImpl(const SkString& text,
                  ParagraphStyle style,
                  std::vector<Block> blocks,
                  sk_sp<FontCollection> fonts);

    ParagraphImpl(const std::u16string& utf16text,
                    ParagraphStyle style,
                    std::vector<Block> blocks,
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
    static void setChecker(std::function<void(ParagraphImpl* impl, const char*, bool)> checker) {
        fParagraphCache.setChecker(checker);
    }

    static void printCache(const char* title) {
        fParagraphCache.printCache(title);
    }

    void printKeyValue(const char* title, bool found) {
        fParagraphCache.printKeyValue(title, this, found);
    }

private:
    friend class ParagraphBuilder;
    friend class ParagraphCacheKey;
    friend class ParagraphCacheValue;
    friend class ParagraphCache;

    void resetContext();
    void resolveStrut();
    void buildClusterTable();
    bool shapeTextIntoEndlessLine();
    void breakShapedTextIntoLines(SkScalar maxWidth);
    void paintLinesIntoPicture();

    BlockRange findAllBlocks(TextRange textRange);

    // Input
    SkTArray<Block, true> fTextStyles;
    SkString fText;
    SkSpan<const char> fTextSpan;

    // Internal structures
    InternalState fState;
    SkTArray<Run> fRuns;
    SkTArray<Cluster, true> fClusters;
    SkTArray<TextLine> fLines;
    LineMetrics fStrutMetrics;
    FontResolver fFontResolver;

    SkScalar fOldWidth;
    SkScalar fOldHeight;

    // Painting
    sk_sp<SkPicture> fPicture;

    // Cache
    static ParagraphCache fParagraphCache;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphImpl_DEFINED
