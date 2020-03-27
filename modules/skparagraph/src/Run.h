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

typedef size_t GlyphIndex;
typedef SkRange<GlyphIndex> GlyphRange;

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

// This is a part of a shaped text
// Text range (a, b)
// LTR: [a:b) where a < b
// RTL: [b:a) where a > b
class ShapedSpan : public TextRange {
  public:
      ShapedSpan() { }
      ShapedSpan(TextRange textRange, GlyphRange glyphRange);
      ShapedSpan(Run* run);

      virtual ~ShapedSpan() = default;

      TextRange normalize() const {
          if (leftToRight()) {
              return TextRange(start, end);
          } else {
              return TextRange(end, start);
          }
      }

      bool leftToRight() const { return start <= end; }
      size_t textSize() const { return leftToRight() ? width() : - width(); }
      size_t glyphSize() const { return fGlyphs.width(); }

      GlyphIndex& glyphStart() { return fGlyphs.start; }
      GlyphIndex& glyphEnd() { return fGlyphs.end; }
      GlyphIndex glyphStart() const { return fGlyphs.start; }
      GlyphIndex glyphEnd() const { return fGlyphs.end; }

      GlyphRange glyphs() const { return fGlyphs; }
      void setGlyphs(GlyphRange glyphs) { fGlyphs = glyphs; }
      void setText(TextRange text) { start = text.start; end = text.end; }

      bool startsIn(TextRange text) const {
          return contains(text.start);
      }

      bool after(TextRange text) const {
          SkASSERT(text.start <= text.end);
          TextRange container = this->normalize();
          return text.end <= container.start;
      }

      bool before(TextRange text) const {
          SkASSERT(text.start <= text.end);
          TextRange container = this->normalize();
          return text.start >= container.end;
      }

      bool contains(const ShapedSpan& other) const {
          TextRange container = this->normalize();
          TextRange box = other.normalize();
          return container.contains(box);
      }

      bool contains(size_t index) const override {
          TextRange container = this->normalize();
          return container.contains(index);
      }

      bool intersects(const ShapedSpan& other) const {
          TextRange container = this->normalize();
          TextRange box = other.normalize();
          return container.intersects(box);
      }

      bool intersects(TextRange text) const override {
          SkASSERT(text.start <= text.end);
          TextRange container = this->normalize();
          return container.intersects(text);
      }

      // box is normalized already, the result will be, too
      TextRange intersection(TextRange box) const override {
          SkASSERT(box.start <= box.end);
          TextRange container = this->normalize();
          return container.intersection(box);
      }

  protected:
      GlyphRange fGlyphs;
};

class InternalLineMetrics;
class Run : public ShapedSpan {
public:
    Run() = default;
    Run(ParagraphImpl* master,
        const SkShaper::RunHandler::RunInfo& info,
        size_t firstChar,
        SkScalar lineHeight,
        size_t index,
        SkScalar shiftX);

    virtual ~Run() = default;

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

class Cluster : public ShapedSpan {
public:
    enum BreakType {
        None,
        GraphemeBreak,  // calculated for all clusters (UBRK_CHARACTER)
        SoftLineBreak,  // calculated for all clusters (UBRK_LINE & UBRK_CHARACTER)
        HardLineBreak,  // calculated for all clusters (UBRK_LINE)
    };

    Cluster()
            : fRun(nullptr)
            , fWidth()
            , fHalfLetterSpacing(0.0)
            , fWhiteSpaces(false)
            , fBreakType(None) {}

    Cluster(Run* run, ShapedSpan shapedSpan, SkScalar width);

    virtual ~Cluster() = default;
    
    SkScalar sizeToChar(TextIndex ch) const;
    SkScalar sizeFromChar(TextIndex ch) const;
    void widen(SkScalar shift) { fWidth += shift; }
    SkScalar width() const { return fWidth; }
    SkScalar trimmedWidth(size_t pos) const;

    void setIsWhiteSpaces(bool value) { fWhiteSpaces = value; }
    bool isWhiteSpaces() const { return fWhiteSpaces; }

    void setBreakType(BreakType type) { fBreakType = type; }
    bool isHardBreak() const { return fBreakType == HardLineBreak; }
    bool isSoftBreak() const { return fBreakType == SoftLineBreak; }
    bool isGraphemeBreak() const { return fBreakType == GraphemeBreak; }

    void setHalfLetterSpacing(SkScalar halfLetterSpacing) { fHalfLetterSpacing = halfLetterSpacing; }
    SkScalar getHalfLetterSpacing() const { return fHalfLetterSpacing; }

    SkFont font() const;
    Run* run() const { return fRun; }

private:

    friend ParagraphImpl;

    Run* fRun;
    SkScalar fWidth;
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
