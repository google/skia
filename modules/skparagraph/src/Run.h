// Copyright 2019 Google LLC.
#ifndef Run_DEFINED
#define Run_DEFINED

#include "include/core/SkFontMetrics.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkSpan.h"
#include "src/core/SkTraceEvent.h"

namespace skia {
namespace textlayout {

class Cluster;
class Run {
public:
    Run() = default;
    Run(SkSpan<const char> text,
          const SkShaper::RunHandler::RunInfo& info,
          SkScalar lineHeight,
          size_t index,
          SkScalar shiftX);
    ~Run() {}

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
    SkScalar descent() const { return fFontMetrics.fDescent; }
    SkScalar leading() const { return fFontMetrics.fLeading; }
    const SkFont& font() const { return fFont; }
    bool leftToRight() const { return fBidiLevel % 2 == 0; }
    size_t index() const { return fIndex; }
    SkScalar lineHeight() const { return fHeightMultiplier; }
    SkSpan<const char> text() const { return fText; }
    size_t clusterIndex(size_t pos) const { return fClusterIndexes[pos]; }
    SkScalar positionX(size_t pos) const { return fPositions[pos].fX + fOffsets[pos]; }
    SkScalar offset(size_t index) const { return fOffsets[index]; }
    SkSpan<Cluster> clusters() const { return fClusters; }
    void setClusters(SkSpan<Cluster> clusters) { fClusters = clusters; }
    SkRect clip() const {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fAdvance.fX, fAdvance.fY);
    }

    SkScalar addSpacesAtTheEnd(SkScalar space, Cluster* cluster);
    SkScalar addSpacesEvenly(SkScalar space, Cluster* cluster);
    void shift(const Cluster* cluster, SkScalar offset);

    SkScalar calculateHeight() const { return fFontMetrics.fDescent - fFontMetrics.fAscent; }
    SkScalar calculateWidth(size_t start, size_t end, bool clip) const;

    void copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size, SkVector offset) const;

    using ClusterVisitor = std::function<void(size_t glyphStart,
                                              size_t glyphEnd,
                                              size_t charStart,
                                              size_t charEnd,
                                              SkScalar width,
                                              SkScalar height)>;
    void iterateThroughClustersInTextOrder(const ClusterVisitor& visitor);

    std::tuple<bool, Cluster*, Cluster*> findLimitingClusters(SkSpan<const char> text);
    SkSpan<const SkGlyphID> glyphs() {
        return SkSpan<const SkGlyphID>(fGlyphs.begin(), fGlyphs.size());
    }
    SkSpan<const SkPoint> positions() {
        return SkSpan<const SkPoint>(fPositions.begin(), fPositions.size());
    }
    SkSpan<const uint32_t> clusterIndexes() {
        return SkSpan<const uint32_t>(fClusterIndexes.begin(), fClusterIndexes.size());
    }

private:
    friend class ParagraphImpl;
    friend class TextLine;
    friend class LineMetrics;

    SkFont fFont;
    SkFontMetrics fFontMetrics;
    SkScalar fHeightMultiplier;
    size_t fIndex;
    uint8_t fBidiLevel;
    SkVector fAdvance;
    SkSpan<const char> fText;
    SkSpan<Cluster> fClusters;
    SkVector fOffset;
    SkShaper::RunHandler::Range fUtf8Range;
    SkSTArray<128, SkGlyphID, false> fGlyphs;
    SkSTArray<128, SkPoint, true> fPositions;
    SkSTArray<128, uint32_t, true> fClusterIndexes;
    SkSTArray<128, SkScalar, true> fOffsets;  // For formatting (letter/word spacing, justification)
    bool fSpaced;
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
            : fText(nullptr, 0)
            , fRun(nullptr)
            , fStart(0)
            , fEnd()
            , fWidth()
            , fSpacing(0)
            , fHeight()
            , fWhiteSpaces(false)
            , fBreakType(None) {}

    Cluster(Run* run,
              size_t start,
              size_t end,
              SkSpan<const char>
                      text,
              SkScalar width,
              SkScalar height)
            : fText(text)
            , fRun(run)
            , fStart(start)
            , fEnd(end)
            , fWidth(width)
            , fSpacing(0)
            , fHeight(height)
            , fWhiteSpaces(false)
            , fBreakType(None) {}

    ~Cluster() = default;

    SkScalar sizeToChar(const char* ch) const;
    SkScalar sizeFromChar(const char* ch) const;

    size_t roundPos(SkScalar s) const;

    void space(SkScalar shift, SkScalar space) {
        fSpacing += space;
        fWidth += shift;
    }

    void setBreakType(BreakType type) { fBreakType = type; }
    void setIsWhiteSpaces(bool ws) { fWhiteSpaces = ws; }
    bool isWhitespaces() const { return fWhiteSpaces; }
    bool canBreakLineAfter() const {
        return fBreakType == SoftLineBreak || fBreakType == HardLineBreak;
    }
    bool isHardBreak() const { return fBreakType == HardLineBreak; }
    bool isSoftBreak() const { return fBreakType == SoftLineBreak; }
    Run* run() const { return fRun; }
    size_t startPos() const { return fStart; }
    size_t endPos() const { return fEnd; }
    SkScalar width() const { return fWidth; }
    SkScalar trimmedWidth() const { return fWidth - fSpacing; }
    SkScalar lastSpacing() const { return fSpacing; }
    SkScalar height() const { return fHeight; }
    SkSpan<const char> text() const { return fText; }
    size_t size() const { return fEnd - fStart; }

    SkScalar trimmedWidth(size_t pos) const;

    void shift(SkScalar offset) const { this->run()->shift(this, offset); }

    void setIsWhiteSpaces();

    bool contains(const char* ch) const { return ch >= fText.begin() && ch < fText.end(); }

    bool belongs(SkSpan<const char> text) const {
        return fText.begin() >= text.begin() && fText.end() <= text.end();
    }

    bool startsIn(SkSpan<const char> text) const {
        return fText.begin() >= text.begin() && fText.begin() < text.end();
    }

private:
    SkSpan<const char> fText;

    Run* fRun;
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

    LineMetrics(SkScalar a, SkScalar d, SkScalar l) {
        fAscent = a;
        fDescent = d;
        fLeading = l;
    }

    LineMetrics(const SkFontMetrics& fm) {
        fAscent = fm.fAscent;
        fDescent = fm.fDescent;
        fLeading = fm.fLeading;
    }

    void add(Run* run) {
        fAscent = SkTMin(fAscent, run->ascent() * run->lineHeight());
        fDescent = SkTMax(fDescent, run->descent() * run->lineHeight());
        fLeading = SkTMax(fLeading, run->leading() * run->lineHeight());
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
    }

    SkScalar delta() const { return height() - ideographicBaseline(); }

    void updateLineMetrics(LineMetrics& metrics, bool forceHeight) {
        if (forceHeight) {
            metrics.fAscent = fAscent;
            metrics.fDescent = fDescent;
            metrics.fLeading = fLeading;
        } else {
            metrics.fAscent = SkTMin(metrics.fAscent, fAscent);
            metrics.fDescent = SkTMax(metrics.fDescent, fDescent);
            metrics.fLeading = SkTMax(metrics.fLeading, fLeading);
        }
    }

    SkScalar runTop(Run* run) const { return fLeading / 2 - fAscent + run->ascent() + delta(); }
    SkScalar height() const { return SkScalarRoundToInt(fDescent - fAscent + fLeading); }
    SkScalar alphabeticBaseline() const { return fLeading / 2 - fAscent; }
    SkScalar ideographicBaseline() const { return fDescent - fAscent + fLeading; }
    SkScalar baseline() const { return fLeading / 2 - fAscent; }
    SkScalar ascent() const { return fAscent; }
    SkScalar descent() const { return fDescent; }
    SkScalar leading() const { return fLeading; }

private:
    SkScalar fAscent;
    SkScalar fDescent;
    SkScalar fLeading;
};
}  // namespace textlayout
}  // namespace skia

#endif  // Run_DEFINED
