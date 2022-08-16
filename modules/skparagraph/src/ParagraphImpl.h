// Copyright 2019 Google LLC.
#ifndef ParagraphImpl_DEFINED
#define ParagraphImpl_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTemplates.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextShadow.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skunicode/include/SkUnicode.h"

#include <memory>
#include <string>
#include <vector>

class SkCanvas;

namespace skia {
namespace textlayout {

class LineMetrics;
class TextLine;

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

struct ResolvedFontDescriptor {

    ResolvedFontDescriptor(TextIndex index, SkFont font)
        : fFont(font), fTextStart(index) { }
    SkFont fFont;
    TextIndex fTextStart;
};
/*
struct BidiRegion {
    BidiRegion(size_t start, size_t end, uint8_t dir)
        : text(start, end), direction(dir) { }
    TextRange text;
    uint8_t direction;
};
*/
class ParagraphImpl final : public Paragraph {

public:

    ParagraphImpl(const SkString& text,
                  ParagraphStyle style,
                  SkTArray<Block, true> blocks,
                  SkTArray<Placeholder, true> placeholders,
                  sk_sp<FontCollection> fonts,
                  std::shared_ptr<SkUnicode> unicode);

    ParagraphImpl(const std::u16string& utf16text,
                  ParagraphStyle style,
                  SkTArray<Block, true> blocks,
                  SkTArray<Placeholder, true> placeholders,
                  sk_sp<FontCollection> fonts,
                  std::shared_ptr<SkUnicode> unicode);

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

    TextLine& addLine(SkVector offset, SkVector advance,
                      TextRange textExcludingSpaces, TextRange text, TextRange textIncludingNewlines,
                      ClusterRange clusters, ClusterRange clustersWithGhosts, SkScalar widthWithSpaces,
                      InternalLineMetrics sizes);

    SkSpan<const char> text() const { return SkSpan<const char>(fText.c_str(), fText.size()); }
    InternalState state() const { return fState; }
    SkSpan<Run> runs() { return SkSpan<Run>(fRuns.data(), fRuns.size()); }
    SkSpan<Block> styles() {
        return SkSpan<Block>(fTextStyles.data(), fTextStyles.size());
    }
    SkSpan<Placeholder> placeholders() {
        return SkSpan<Placeholder>(fPlaceholders.data(), fPlaceholders.size());
    }
    SkSpan<TextLine> lines() { return SkSpan<TextLine>(fLines.data(), fLines.size()); }
    const ParagraphStyle& paragraphStyle() const { return fParagraphStyle; }
    SkSpan<Cluster> clusters() { return SkSpan<Cluster>(fClusters.begin(), fClusters.size()); }
    sk_sp<FontCollection> fontCollection() const { return fFontCollection; }
    void formatLines(SkScalar maxWidth);
    void ensureUTF16Mapping();
    TextIndex findNextGraphemeBoundary(TextIndex utf8);
    TextIndex findPreviousGraphemeBoundary(TextIndex utf8);
    size_t getUTF16Index(TextIndex index) {
        return fUTF16IndexForUTF8Index[index];
    }

    bool strutEnabled() const { return paragraphStyle().getStrutStyle().getStrutEnabled(); }
    bool strutForceHeight() const {
        return paragraphStyle().getStrutStyle().getForceStrutHeight();
    }
    bool strutHeightOverride() const {
        return paragraphStyle().getStrutStyle().getHeightOverride();
    }
    InternalLineMetrics strutMetrics() const { return fStrutMetrics; }

    SkString getEllipsis() const;

    SkSpan<const char> text(TextRange textRange);
    SkSpan<Cluster> clusters(ClusterRange clusterRange);
    Cluster& cluster(ClusterIndex clusterIndex);
    ClusterIndex clusterIndex(TextIndex textIndex) {
        auto clusterIndex = this->fClustersIndexFromCodeUnit[textIndex];
        SkASSERT(clusterIndex != EMPTY_INDEX);
        return clusterIndex;
    }
    Run& run(RunIndex runIndex) {
        SkASSERT(runIndex < fRuns.size());
        return fRuns[runIndex];
    }

    Run& runByCluster(ClusterIndex clusterIndex);
    SkSpan<Block> blocks(BlockRange blockRange);
    Block& block(BlockIndex blockIndex);
    SkTArray<ResolvedFontDescriptor> resolvedFonts() const { return fFontSwitches; }

    void markDirty() override { fState = kUnknown; }

    int32_t unresolvedGlyphs() override;

    void setState(InternalState state);
    sk_sp<SkPicture> getPicture() { return fPicture; }

    SkScalar widthWithTrailingSpaces() { return fMaxWidthWithTrailingSpaces; }

    void resetContext();
    void resolveStrut();

    bool computeCodeUnitProperties();
    void applySpacingAndBuildClusterTable();
    void buildClusterTable();
    bool shapeTextIntoEndlessLine();
    void breakShapedTextIntoLines(SkScalar maxWidth);

    void updateTextAlign(TextAlign textAlign) override;
    void updateText(size_t from, SkString text) override;
    void updateFontSize(size_t from, size_t to, SkScalar fontSize) override;
    void updateForegroundPaint(size_t from, size_t to, SkPaint paint) override;
    void updateBackgroundPaint(size_t from, size_t to, SkPaint paint) override;

    void visit(const Visitor&) override;

    InternalLineMetrics getEmptyMetrics() const { return fEmptyMetrics; }
    InternalLineMetrics getStrutMetrics() const { return fStrutMetrics; }

    BlockRange findAllBlocks(TextRange textRange);

    void resetShifts() {
        for (auto& run : fRuns) {
            run.resetJustificationShifts();
        }
    }

    bool codeUnitHasProperty(size_t index, SkUnicode::CodeUnitFlags property) const {
        return (fCodeUnitProperties[index] & property) == property;
    }

    SkUnicode* getUnicode() { return fUnicode.get(); }

private:
    friend class ParagraphBuilder;
    friend class ParagraphCacheKey;
    friend class ParagraphCacheValue;
    friend class ParagraphCache;

    friend class TextWrapper;
    friend class OneLineShaper;

    void computeEmptyMetrics();

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
    SkTArray<Run, false> fRuns;         // kShaped
    SkTArray<Cluster, true> fClusters;  // kClusterized (cached: text, word spacing, letter spacing, resolved fonts)
    SkTArray<SkUnicode::CodeUnitFlags, true> fCodeUnitProperties;
    SkTArray<size_t, true> fClustersIndexFromCodeUnit;
    std::vector<size_t> fWords;
    std::vector<SkUnicode::BidiRegion> fBidiRegions;
    // These two arrays are used in measuring methods (getRectsForRange, getGlyphPositionAtCoordinate)
    // They are filled lazily whenever they need and cached
    SkTArray<TextIndex, true> fUTF8IndexForUTF16Index;
    SkTArray<size_t, true> fUTF16IndexForUTF8Index;
    SkOnce fillUTF16MappingOnce;
    size_t fUnresolvedGlyphs;

    SkTArray<TextLine, false> fLines;   // kFormatted   (cached: width, max lines, ellipsis, text align)
    sk_sp<SkPicture> fPicture;          // kRecorded    (cached: text styles)

    SkTArray<ResolvedFontDescriptor> fFontSwitches;

    InternalLineMetrics fEmptyMetrics;
    InternalLineMetrics fStrutMetrics;

    SkScalar fOldWidth;
    SkScalar fOldHeight;
    SkScalar fMaxWidthWithTrailingSpaces;

    std::shared_ptr<SkUnicode> fUnicode;
    bool fHasLineBreaks;
    bool fHasWhitespacesInside;
    TextIndex fTrailingSpaces;
};
}  // namespace textlayout
}  // namespace skia


#endif  // ParagraphImpl_DEFINED
