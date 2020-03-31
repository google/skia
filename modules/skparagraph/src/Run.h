// Copyright 2019 Google LLC.
#ifndef Run_DEFINED
#define Run_DEFINED

#include "include/core/SkFontMetrics.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTextBlob.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkSpan.h"
#include "src/core/SkTraceEvent.h"
#include <functional>  // std::function

namespace skia {
namespace textlayout {

class ParagraphImpl;
class Cluster;
class Run;

typedef size_t RunIndex;
const size_t EMPTY_RUN = EMPTY_INDEX;

typedef size_t ClusterIndex;
typedef SkRange<size_t> ClusterRange;
const size_t EMPTY_CLUSTER = EMPTY_INDEX;
const SkRange<size_t> EMPTY_CLUSTERS = EMPTY_RANGE;

typedef size_t GraphemeIndex;
typedef SkRange<GraphemeIndex> GraphemeRange;

typedef size_t CodepointIndex;
typedef SkRange<CodepointIndex> CodepointRange;

struct RunShifts {
    RunShifts() { }
    RunShifts(size_t count) { fShifts.push_back_n(count, 0.0); }
    SkSTArray<128, SkScalar, true> fShifts;
};

class InternalLineMetrics;
class Run {
public:
    Run() = default;
    Run(ParagraphImpl* master,
        const SkShaper::RunHandler::RunInfo& info,
        size_t firstChar,
        SkScalar lineHeight,
        size_t index,
        SkScalar shiftX);
    ~Run() {}

    void setMaster(ParagraphImpl* master) { fMaster = master; }

    SkShaper::RunHandler::Buffer newRunBuffer();

    SkScalar posX(size_t index) const {
        return fPositions[index].fX + fOffsets[index].fX;
    }
    void addX(size_t index, SkScalar shift) {
        fPositions[index].fX += shift;
    }
    SkScalar posY(size_t index) const {
        return fPositions[index].fY + fOffsets[index].fY;
    }
    size_t size() const { return fGlyphs.size(); }
    void setWidth(SkScalar width) { fAdvance.fX = width; }
    void setHeight(SkScalar height) { fAdvance.fY = height; }
    void shift(SkScalar shiftX, SkScalar shiftY) {
        fOffset.fX += shiftX;
        fOffset.fY += shiftY;
    }
    SkVector advance() const {
        return SkVector::Make(fAdvance.fX, fFontMetrics.fDescent - fFontMetrics.fAscent);
    }
    SkVector offset() const { return fOffset; }
    SkScalar ascent() const { return fFontMetrics.fAscent; }
    SkScalar correctAscent() const {

        if (fHeightMultiplier == 0) {
            return fFontMetrics.fAscent - fFontMetrics.fLeading / 2;
        }
        return fFontMetrics.fAscent * fHeightMultiplier * fFont.getSize() /
                (fFontMetrics.fDescent - fFontMetrics.fAscent + fFontMetrics.fLeading / 2);
    }
    SkScalar correctDescent() const {

        if (fHeightMultiplier == 0) {
            return fFontMetrics.fDescent + fFontMetrics.fLeading / 2;
        }
        return fFontMetrics.fDescent * fHeightMultiplier * fFont.getSize() /
                (fFontMetrics.fDescent - fFontMetrics.fAscent + fFontMetrics.fLeading / 2);
    }
    SkScalar correctLeading() const {

        if (fHeightMultiplier == 0) {
            return fFontMetrics.fAscent;
        }
        return fFontMetrics.fLeading * fHeightMultiplier * fFont.getSize() /
                (fFontMetrics.fDescent - fFontMetrics.fAscent + fFontMetrics.fLeading);
    }
    const SkFont& font() const { return fFont; }
    bool leftToRight() const { return fBidiLevel % 2 == 0; }
    size_t index() const { return fIndex; }
    SkScalar lineHeight() const { return fHeightMultiplier; }
    PlaceholderStyle* placeholderStyle() const;
    bool isPlaceholder() const { return fPlaceholderIndex != std::numeric_limits<size_t>::max(); }
    size_t clusterIndex(size_t pos) const { return fClusterIndexes[pos]; }
    size_t globalClusterIndex(size_t pos) const { return fClusterStart + fClusterIndexes[pos]; }
    SkScalar positionX(size_t pos) const;

    TextRange textRange() const { return fTextRange; }
    ClusterRange clusterRange() const { return fClusterRange; }

    ParagraphImpl* master() const { return fMaster; }

    bool isEllipsis() const { return fEllipsis; }

    void updateMetrics(InternalLineMetrics* endlineMetrics);

    void setClusterRange(size_t from, size_t to) { fClusterRange = ClusterRange(from, to); }
    SkRect clip() const {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fAdvance.fX, fAdvance.fY);
    }

    SkScalar addSpacesAtTheEnd(SkScalar space, Cluster* cluster);
    SkScalar addSpacesEvenly(SkScalar space, Cluster* cluster);
    void shift(const Cluster* cluster, SkScalar offset);

    SkScalar calculateHeight() const {
        if (fHeightMultiplier == 0) {
            return fFontMetrics.fDescent - fFontMetrics.fAscent;
        }
        return fHeightMultiplier * fFont.getSize();
    }
    SkScalar calculateWidth(size_t start, size_t end, bool clip) const;

    void copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size, SkVector offset) const;

    using ClusterVisitor = std::function<void(size_t glyphStart,
                                              size_t glyphEnd,
                                              size_t charStart,
                                              size_t charEnd,
                                              SkScalar width,
                                              SkScalar height)>;
    void iterateThroughClustersInTextOrder(const ClusterVisitor& visitor);

    std::tuple<bool, ClusterIndex, ClusterIndex> findLimitingClusters(TextRange text, bool onlyInnerClusters) const;
    SkSpan<const SkGlyphID> glyphs() const {
        return SkSpan<const SkGlyphID>(fGlyphs.begin(), fGlyphs.size());
    }
    SkSpan<const SkPoint> positions() const {
        return SkSpan<const SkPoint>(fPositions.begin(), fPositions.size());
    }
    SkSpan<const SkPoint> offsets() const {
        return SkSpan<const SkPoint>(fOffsets.begin(), fOffsets.size());
    }
    SkSpan<const uint32_t> clusterIndexes() const {
        return SkSpan<const uint32_t>(fClusterIndexes.begin(), fClusterIndexes.size());
    }
    SkSpan<const SkScalar> shifts() const { return SkSpan<const SkScalar>(fShifts.begin(), fShifts.size()); }

    void commit();

    SkRect getBounds(size_t pos) const { return fBounds[pos]; }

    void resetShifts() {
        for (auto& r: fShifts) { r = 0; }
        fSpaced = false;
    }

    void resetJustificationShifts() {
        fJustificationShifts.reset();
    }
private:
    friend class ParagraphImpl;
    friend class TextLine;
    friend class InternalLineMetrics;
    friend class ParagraphCache;
    friend class OneLineShaper;

    ParagraphImpl* fMaster;
    TextRange fTextRange;
    ClusterRange fClusterRange;

    SkFont fFont;
    SkFontMetrics fFontMetrics;
    SkScalar fHeightMultiplier;
    size_t fPlaceholderIndex;
    bool fEllipsis;
    size_t fIndex;
    uint8_t fBidiLevel;
    SkVector fAdvance;
    SkVector fOffset;
    TextIndex fClusterStart;
    SkShaper::RunHandler::Range fUtf8Range;
    SkSTArray<128, SkGlyphID, true> fGlyphs;
    SkSTArray<128, SkPoint, true> fPositions;
    SkSTArray<128, SkPoint, true> fJustificationShifts; // For justification (current and prev shifts)
    SkSTArray<128, SkPoint, true> fOffsets;
    SkSTArray<128, uint32_t, true> fClusterIndexes;
    SkSTArray<128, SkRect, true> fBounds;

    SkSTArray<128, SkScalar, true> fShifts;  // For formatting (letter/word spacing)
    bool fSpaced;
};

struct Codepoint {

  Codepoint(GraphemeIndex graphemeIndex, TextIndex textIndex, size_t index)
    : fGrapheme(graphemeIndex), fTextIndex(textIndex), fIndex(index) { }

  GraphemeIndex fGrapheme;
  TextIndex fTextIndex;             // Used for getGlyphPositionAtCoordinate
  size_t fIndex;
};

struct Grapheme {
    Grapheme(CodepointRange codepoints, TextRange textRange)
        : fCodepointRange(codepoints), fTextRange(textRange) { }
    CodepointRange fCodepointRange;
    TextRange fTextRange;           // Used for getRectsForRange
};

class Cluster {
public:
    enum BreakType {
        None,
        CharacterBoundary,       // not yet in use (UBRK_CHARACTER)
        WordBoundary,            // calculated for all clusters (UBRK_WORD)
        WordBreakWithoutHyphen,  // calculated only for hyphenated words
        WordBreakWithHyphen,
        SoftLineBreak,  // calculated for all clusters (UBRK_LINE)
        HardLineBreak,  // calculated for all clusters (UBRK_LINE)
        GraphemeBreak,
    };

    Cluster()
            : fMaster(nullptr)
            , fRunIndex(EMPTY_RUN)
            , fTextRange(EMPTY_TEXT)
            , fGraphemeRange(EMPTY_RANGE)
            , fStart(0)
            , fEnd()
            , fWidth()
            , fSpacing(0)
            , fHeight()
            , fHalfLetterSpacing(0.0)
            , fWhiteSpaces(false)
            , fBreakType(None) {}

    Cluster(ParagraphImpl* master,
            RunIndex runIndex,
            size_t start,
            size_t end,
            SkSpan<const char> text,
            SkScalar width,
            SkScalar height);

    Cluster(TextRange textRange) : fTextRange(textRange), fGraphemeRange(EMPTY_RANGE) { }

    ~Cluster() = default;

    void setMaster(ParagraphImpl* master) { fMaster = master; }
    SkScalar sizeToChar(TextIndex ch) const;
    SkScalar sizeFromChar(TextIndex ch) const;

    size_t roundPos(SkScalar s) const;

    void space(SkScalar shift, SkScalar space) {
        fSpacing += space;
        fWidth += shift;
    }

    void setBreakType(BreakType type) { fBreakType = type; }
    bool isWhitespaces() const { return fWhiteSpaces; }
    bool canBreakLineAfter() const {
        return fBreakType == SoftLineBreak || fBreakType == HardLineBreak;
    }
    bool isHardBreak() const { return fBreakType == HardLineBreak; }
    bool isSoftBreak() const { return fBreakType == SoftLineBreak; }
    bool isGraphemeBreak() const { return fBreakType == GraphemeBreak; }
    size_t startPos() const { return fStart; }
    size_t endPos() const { return fEnd; }
    SkScalar width() const { return fWidth; }
    SkScalar height() const { return fHeight; }
    size_t size() const { return fEnd - fStart; }

    void setHalfLetterSpacing(SkScalar halfLetterSpacing) { fHalfLetterSpacing = halfLetterSpacing; }
    SkScalar getHalfLetterSpacing() const { return fHalfLetterSpacing; }

    TextRange textRange() const { return fTextRange; }

    RunIndex runIndex() const { return fRunIndex; }
    ParagraphImpl* master() const { return fMaster; }

    Run* run() const;
    SkFont font() const;

    SkScalar trimmedWidth(size_t pos) const;

    void setIsWhiteSpaces();

    bool contains(TextIndex ch) const { return ch >= fTextRange.start && ch < fTextRange.end; }

    bool belongs(TextRange text) const {
        return fTextRange.start >= text.start && fTextRange.end <= text.end;
    }

    bool startsIn(TextRange text) const {
        return fTextRange.start >= text.start && fTextRange.start < text.end;
    }

private:

    friend ParagraphImpl;

    ParagraphImpl* fMaster;
    RunIndex fRunIndex;
    TextRange fTextRange;
    GraphemeRange fGraphemeRange;

    size_t fStart;
    size_t fEnd;
    SkScalar fWidth;
    SkScalar fSpacing;
    SkScalar fHeight;
    SkScalar fHalfLetterSpacing;
    bool fWhiteSpaces;
    BreakType fBreakType;
};

class InternalLineMetrics {
public:

    InternalLineMetrics() {
        clean();
        fForceStrut = false;
    }

    InternalLineMetrics(bool forceStrut) {
        clean();
        fForceStrut = forceStrut;
    }

    InternalLineMetrics(SkScalar a, SkScalar d, SkScalar l) {
        fAscent = a;
        fDescent = d;
        fLeading = l;
        fForceStrut = false;
    }

    InternalLineMetrics(const SkFont& font, bool forceStrut) {
        SkFontMetrics metrics;
        font.getMetrics(&metrics);
        fAscent = metrics.fAscent;
        fDescent = metrics.fDescent;
        fLeading = metrics.fLeading;
        fForceStrut = forceStrut;
    }

    void add(Run* run) {

        if (fForceStrut) {
            return;
        }

        fAscent = std::min(fAscent, run->correctAscent());
        fDescent = std::max(fDescent, run->correctDescent());
        fLeading = std::max(fLeading, run->correctLeading());
    }

    void add(InternalLineMetrics other) {
        fAscent = std::min(fAscent, other.fAscent);
        fDescent = std::max(fDescent, other.fDescent);
        fLeading = std::max(fLeading, other.fLeading);
    }
    void clean() {
        fAscent = 0;
        fDescent = 0;
        fLeading = 0;
    }

    SkScalar delta() const { return height() - ideographicBaseline(); }

    void updateLineMetrics(InternalLineMetrics& metrics) {
        if (metrics.fForceStrut) {
            metrics.fAscent = fAscent;
            metrics.fDescent = fDescent;
            metrics.fLeading = fLeading;
        } else {
            // This is another of those flutter changes. To be removed...
            metrics.fAscent = std::min(metrics.fAscent, fAscent - fLeading / 2.0f);
            metrics.fDescent = std::max(metrics.fDescent, fDescent + fLeading / 2.0f);
        }
    }

    SkScalar runTop(const Run* run) const {
        return fLeading / 2 - fAscent + run->ascent() + delta();
    }

    SkScalar height() const {
        return ::round((double)fDescent - fAscent + fLeading);
    }

    SkScalar alphabeticBaseline() const { return fLeading / 2 - fAscent; }
    SkScalar ideographicBaseline() const { return fDescent - fAscent + fLeading; }
    SkScalar deltaBaselines() const { return fLeading / 2 + fDescent; }
    SkScalar baseline() const { return fLeading / 2 - fAscent; }
    SkScalar ascent() const { return fAscent; }
    SkScalar descent() const { return fDescent; }
    SkScalar leading() const { return fLeading; }
    void setForceStrut(bool value) { fForceStrut = value; }

private:

    friend class TextWrapper;

    SkScalar fAscent;
    SkScalar fDescent;
    SkScalar fLeading;
    bool fForceStrut;
};
}  // namespace textlayout
}  // namespace skia

#endif  // Run_DEFINED
