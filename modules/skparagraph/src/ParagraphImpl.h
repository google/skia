// Copyright 2019 Google LLC.
#ifndef ParagraphImpl_DEFINED
#define ParagraphImpl_DEFINED

#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/src/Run.h"
#include "include/core/SkPicture.h"
#include "include/private//SkTHash.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"

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
                  sk_sp<FontCollection> fonts)
            : Paragraph(std::move(style), std::move(fonts))
            , fText(text)
            , fTextSpan(fText.c_str(), fText.size())
            , fPicture(nullptr) {
        fTextStyles.reserve(blocks.size());
        for (auto& block : blocks) {
            fTextStyles.emplace_back(
                    SkSpan<const char>(fTextSpan.begin() + block.fStart, block.fEnd - block.fStart),
                    block.fStyle);
        }
    }

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
                      size_t start, size_t end, LineMetrics sizes);

    SkSpan<const char> text() const { return fTextSpan; }
    SkSpan<Run> runs() { return SkSpan<Run>(fRuns.data(), fRuns.size()); }
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

private:
    friend class ParagraphBuilder;

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
    SkTArray<Run> fRuns;
    SkTArray<Cluster, true> fClusters;
    SkTArray<TextLine> fLines;
    LineMetrics fStrutMetrics;

    // Painting
    sk_sp<SkPicture> fPicture;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphImpl_DEFINED
