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
#include <functional>  // std::function

namespace skia {
namespace textlayout {

class ParagraphImpl;
class Cluster;

// LTR: [start: end) where start <= end
// RTL: [end: start) where start >= end
class DirText {
    DirText(bool dir, size_t s, size_t e) : start(s), end(e) { }
    bool isLeftToRight() const { return start <= end; }
    size_t start;
    size_t end;
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
    Run(const Run&) = default;
    Run& operator=(const Run&) = default;
    Run(Run&&) = default;
    Run& operator=(Run&&) = default;
    ~Run() = default;

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
    SkScalar descent() const { return fFontMetrics.fDescent; }
    SkScalar leading() const { return fFontMetrics.fLeading; }
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
    TextDirection getTextDirection() const { return leftToRight() ? TextDirection::kLtr : TextDirection::kRtl; }
    size_t index() const { return fIndex; }
    SkScalar heightMultiplier() const { return fHeightMultiplier; }
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

    SkScalar calculateHeight(LineMetricStyle ascentStyle, LineMetricStyle descentStyle) const {
        auto ascent = ascentStyle == LineMetricStyle::Typographic ? this->ascent()
                                    : this->correctAscent();
        auto descent = descentStyle == LineMetricStyle::Typographic ? this->descent()
                                      : this->correctDescent();
        return descent - ascent;
    }
    SkScalar calculateWidth(size_t start, size_t end, bool clip) const;

    void copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size, SkVector offset) const;

    using ClusterTextVisitor = std::function<void(size_t glyphStart,
                                                  size_t glyphEnd,
                                                  size_t charStart,
                                                  size_t charEnd,
                                                  SkScalar width,
                                                  SkScalar height)>;
    void iterateThroughClustersInTextOrder(const ClusterTextVisitor& visitor);

    using ClusterVisitor = std::function<void(Cluster* cluster)>;
    void iterateThroughClusters(const ClusterVisitor& visitor);

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

    SkScalar runTop(const Run* run, LineMetricStyle ascentStyle) const {
        return fLeading / 2 - fAscent +
          (ascentStyle == LineMetricStyle::Typographic ? run->ascent() : run->correctAscent()) + delta();
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
    bool getForceStrut() const { return fForceStrut; }

private:

    friend class TextWrapper;
    friend class TextLine;

    SkScalar fAscent;
    SkScalar fDescent;
    SkScalar fLeading;
    bool fForceStrut;
};
}  // namespace textlayout
}  // namespace skia

#endif  // Run_DEFINED
