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

    enum InternalState {
      kUnknown = 0,
      kShaped = 1,
      kClusterized = 2,
      kLineBroken = 3,
      kFormatted = 4,
      kRecorded = 5
    };

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

    TextLine& addLine(SkVector offset, SkVector advance, SkSpan<const char> text,
                      SkSpan<const char> textWithSpaces, SkSpan<const Cluster> clusters,
                      LineMetrics sizes);

    SkSpan<const char> text() const { return fTextSpan; }
    InternalState state() const { return fState; }
    SkSpan<Run> runs() { return SkSpan<Run>(fRuns.data(), fRuns.size()); }
    SkTArray<FontDescr>& switches() { return fFontResolver.switches(); }
    SkSpan<TextBlock> styles() {
        return SkSpan<TextBlock>(fTextStyles.data(), fTextStyles.size());
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

    SkSpan<const TextBlock> findAllBlocks(SkSpan<const char> text);

    // Input
    SkTArray<TextBlock, true> fTextStyles;
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

    // Painting
    sk_sp<SkPicture> fPicture;

    // Cache
    static ParagraphCache fParagraphCache;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphImpl_DEFINED
