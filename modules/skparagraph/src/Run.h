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

class LineMetrics;
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

        if (fHeightMultiplier == 0 || fHeightMultiplier == 1) {
            return fFontMetrics.fAscent - fFontMetrics.fLeading / 2;
        }
        return fFontMetrics.fAscent * fHeightMultiplier * fFont.getSize() /
                (fFontMetrics.fDescent - fFontMetrics.fAscent + fFontMetrics.fLeading / 2);
    }
    SkScalar correctDescent() const {

        if (fHeightMultiplier == 0 || fHeightMultiplier == 1) {
            return fFontMetrics.fDescent + fFontMetrics.fLeading / 2;
        }
        return fFontMetrics.fDescent * fHeightMultiplier * fFont.getSize() /
                (fFontMetrics.fDescent - fFontMetrics.fAscent + fFontMetrics.fLeading / 2);
    }
    SkScalar correctLeading() const {

        if (fHeightMultiplier == 0 || fHeightMultiplier == 1) {
            return fFontMetrics.fAscent;
        }
        return fFontMetrics.fLeading * fHeightMultiplier * fFont.getSize() /
                (fFontMetrics.fDescent - fFontMetrics.fAscent + fFontMetrics.fLeading);
    }
    const SkFont& font() const { return fFont; }
    bool leftToRight() const { return fBidiLevel % 2 == 0; }
    size_t index() const { return fIndex; }
    SkScalar lineHeight() const { return fHeightMultiplier; }
    PlaceholderStyle* placeholder() const { return fPlaceholder; }
    bool isPlaceholder() const { return fPlaceholder != nullptr; }
    size_t clusterIndex(size_t pos) const { return fClusterIndexes[pos]; }
    SkScalar positionX(size_t pos) const;

    TextRange textRange() { return fTextRange; }
    ClusterRange clusterRange() { return fClusterRange; }

    void updateMetrics(LineMetrics* endlineMetrics);

    void setClusterRange(size_t from, size_t to) { fClusterRange = ClusterRange(from, to); }
    SkRect clip() const {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fAdvance.fX, fAdvance.fY);
    }

    SkScalar addSpacesAtTheEnd(SkScalar space, Cluster* cluster);
    SkScalar addSpacesEvenly(SkScalar space, Cluster* cluster);
    void shift(const Cluster* cluster, SkScalar offset);

    SkScalar calculateHeight() const {
        if (fHeightMultiplier == 0 || fHeightMultiplier == 1) {
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

    std::tuple<bool, ClusterIndex, ClusterIndex> findLimitingClusters(TextRange);
    SkSpan<const SkGlyphID> glyphs() const {
        return SkSpan<const SkGlyphID>(fGlyphs.begin(), fGlyphs.size());
    }
    SkSpan<const SkPoint> positions() const {
        return SkSpan<const SkPoint>(fPositions.begin(), fPositions.size());
    }
    SkSpan<const uint32_t> clusterIndexes() const {
        return SkSpan<const uint32_t>(fClusterIndexes.begin(), fClusterIndexes.size());
    }
    SkSpan<const SkScalar> offsets() const { return SkSpan<const SkScalar>(fOffsets.begin(), fOffsets.size()); }

private:
    friend class ParagraphImpl;
    friend class TextLine;
    friend class LineMetrics;
    friend class ParagraphCache;

    ParagraphImpl* fMaster;
    TextRange fTextRange;
    ClusterRange fClusterRange;

    SkFont fFont;
    SkFontMetrics fFontMetrics;
    SkScalar fHeightMultiplier;
    PlaceholderStyle* fPlaceholder;
    size_t fIndex;
    uint8_t fBidiLevel;
    SkVector fAdvance;
    SkVector fOffset;
    size_t fFirstChar;
    SkShaper::RunHandler::Range fUtf8Range;
    SkSTArray<128, SkGlyphID, false> fGlyphs;
    SkSTArray<128, SkPoint, true> fPositions;
    SkSTArray<128, uint32_t, true> fClusterIndexes;
    SkSTArray<128, SkScalar, true> fOffsets;  // For formatting (letter/word spacing, justification)
    bool fSpaced;
};

struct Codepoint {

  Codepoint(GraphemeIndex graphemeIndex, TextIndex textIndex)
    : fGrapeme(graphemeIndex), fTextIndex(textIndex) { }

  GraphemeIndex fGrapeme;
  TextIndex fTextIndex;             // Used for getGlyphPositionAtCoordinate
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
    size_t startPos() const { return fStart; }
    size_t endPos() const { return fEnd; }
    SkScalar width() const { return fWidth; }
    SkScalar height() const { return fHeight; }
    size_t size() const { return fEnd - fStart; }

    TextRange textRange() const { return fTextRange; }

    RunIndex runIndex() const { return fRunIndex; }
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
    bool fWhiteSpaces;
    BreakType fBreakType;
};

class LineMetrics {
public:

    LineMetrics() { clean(); }
    LineMetrics(bool forceStrut) {
        clean();
        fForceStrut = forceStrut;
    }

    LineMetrics(SkScalar a, SkScalar d, SkScalar l) {
        fAscent = a;
        fDescent = d;
        fLeading = l;
        fForceStrut = false;
    }

    LineMetrics(const SkFont& font, bool forceStrut) {
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

        fAscent = SkTMin(fAscent, run->correctAscent());
        fDescent = SkTMax(fDescent, run->correctDescent());
        fLeading = SkTMax(fLeading, run->correctLeading());

    }

    void add(LineMetrics other) {
        fAscent = SkTMin(fAscent, other.fAscent);
        fDescent = SkTMax(fDescent, other.fDescent);
        fLeading = SkTMax(fLeading, other.fLeading);
    }
    void clean() {
        fAscent = 0;
        fDescent = 0;
        fLeading = 0;
        fForceStrut = false;
    }

    SkScalar delta() const { return height() - ideographicBaseline(); }

    void updateLineMetrics(LineMetrics& metrics) {
        metrics.fAscent = SkTMin(metrics.fAscent, fAscent);
        metrics.fDescent = SkTMax(metrics.fDescent, fDescent);
        metrics.fLeading = SkTMax(metrics.fLeading, fLeading);
    }

    SkScalar runTop(Run* run) const {
        return fLeading / 2 - fAscent + run->ascent() + delta();
    }
    SkScalar height() const { return SkScalarRoundToInt(fDescent - fAscent + fLeading); }
    SkScalar alphabeticBaseline() const { return fLeading / 2 - fAscent; }
    SkScalar ideographicBaseline() const { return fDescent - fAscent + fLeading; }
    SkScalar deltaBaselines() const { return fLeading / 2 + fDescent; }
    SkScalar baseline() const { return fLeading / 2 - fAscent; }
    SkScalar ascent() const { return fAscent; }
    SkScalar descent() const { return fDescent; }
    SkScalar leading() const { return fLeading; }

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
