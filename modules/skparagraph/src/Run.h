// Copyright 2019 Google LLC.
#ifndef Run_DEFINED
#define Run_DEFINED

#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkSpan.h"
#include "src/core/SkTraceEvent.h"

namespace skia {
namespace textlayout {

class ParagraphImpl;
class Cluster;
class Run;

class TextBlock {
public:
    TextBlock() : fText(), fTextStyle() {}
    TextBlock(SkSpan<const char> text, const TextStyle& style) : fText(text), fTextStyle(style) {}

    SkSpan<const char> text() const { return fText; }
    const TextStyle& style() const { return fTextStyle; }

    void add(SkSpan<const char> tail) {
        SkASSERT(fText.end() == tail.begin());
        fText = SkSpan<const char>(fText.begin(), fText.size() + tail.size());
    }

protected:
    SkSpan<const char> fText;
    TextStyle fTextStyle;
};

template<class M, class T, T* access(M*)>
class StableReference {

public:
    StableReference() : fInitialized(false) { }
    StableReference(uint32_t index) : fInitialized(true), fIndex(index) { }
    StableReference(M* master, T* ref)
    : fInitialized(ref != nullptr), fIndex(ref == nullptr ? 0 : ref - access(master)) { }
    T* reference(M* master) const { return fInitialized ? access(master) + fIndex : nullptr; }
private:
    bool fInitialized;
    uint32_t fIndex;
};

template<class M, class T, T* access(M*)>
class StableRange {
public:
    StableRange() : fMaster(nullptr), fRange() { }
    StableRange(M* master) : fMaster(master), fRange() { }
    StableRange(M* master, SkRange<uint32_t> range) : fMaster(master), fRange(range) { }
    StableRange(M* master, uint32_t from, uint32_t to) : fMaster(master), fRange(from, to) { }
    StableRange(M* master, SkSpan<T> span) : fMaster(master) {
        auto base = fMaster == nullptr ? nullptr : access(fMaster);
        fRange = SkRange<uint32_t>(span.begin() - base, span.end() - base);
    }

    constexpr T& front() const { return access(fMaster) + fRange.start; }
    constexpr T& back()  const { return access(fMaster) + fRange.end - 1; }
    constexpr T* begin() const { return access(fMaster) + fRange.start; }
    constexpr T* end() const { return access(fMaster) + fRange.end; }
    constexpr const T* cbegin() const { return access(fMaster) + fRange.start; }
    constexpr const T* cend() const { return access(fMaster) + fRange.end; }
    constexpr T* data() const { return access(fMaster) + fRange.start; }
    constexpr size_t size() const { return fRange.width(); }
    constexpr bool empty() const { return fRange.width() == 0; }
    constexpr size_t size_bytes() const { return fRange.width() * sizeof(T); }

    void setMaster(M* master) { fMaster = master; }
    SkSpan<T> span() const {
        auto base = access(fMaster);
        return base == nullptr
                       ? SkSpan<T>(nullptr, 0)
                       : SkSpan<T>(base + fRange.start, fRange.width());
    }
    SkRange<uint32_t> range() const { return fRange; }

private:
    M* fMaster;
    SkRange<uint32_t> fRange;
};

const char* accessText(ParagraphImpl* master);
const Cluster* accessCluster(ParagraphImpl* master);
const Run* accessRun(ParagraphImpl* master);
Run* accessRunRef(ParagraphImpl* master);
const TextBlock* accessTextBlock(ParagraphImpl* master);

class Run {
public:
    Run() = default;
    Run(ParagraphImpl* master,
        SkSpan<const char> text,
        const SkShaper::RunHandler::RunInfo& info,
        SkScalar lineHeight,
        size_t index,
        SkScalar shiftX);
    ~Run() {}

    void setMaster(ParagraphImpl* master) {
        fMaster = master;
        this->fTextRange.setMaster(master);
        this->fClusterRange.setMaster(master);
    }

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
    size_t clusterIndex(size_t pos) const { return fClusterIndexes[pos]; }
    SkScalar positionX(size_t pos) const { return fPositions[pos].fX + fOffsets[pos]; }
    SkScalar offset(size_t index) const { return fOffsets[index]; }

    SkSpan<const char> textSpan() { return fTextRange.span(); }
    SkSpan<const Cluster> clusterSpan() { return fClusterRange.span(); }
    void setClusterRange(size_t from, size_t to) {
        fClusterRange = StableRange<ParagraphImpl, const Cluster, &accessCluster>(fMaster, from, to);
    }
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

    std::tuple<bool, const Cluster*, const Cluster*> findLimitingClusters(SkSpan<const char> text);
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
    friend class ParagraphCache;

    ParagraphImpl* fMaster;
    StableRange<ParagraphImpl, const char, &accessText> fTextRange;
    StableRange<ParagraphImpl, const Cluster, &accessCluster> fClusterRange;

    SkFont fFont;
    SkFontMetrics fFontMetrics;
    SkScalar fHeightMultiplier;
    size_t fIndex;
    uint8_t fBidiLevel;
    SkVector fAdvance;
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
            : fMaster(nullptr)
            , fTextRange(nullptr)
            , fRunReference()
            , fStart(0)
            , fEnd()
            , fWidth()
            , fSpacing(0)
            , fHeight()
            , fWhiteSpaces(false)
            , fBreakType(None) {}

    Cluster(ParagraphImpl* master,
            Run* run,
            size_t start,
            size_t end,
            SkSpan<const char> text,
            SkScalar width,
            SkScalar height)
            : fMaster(master)
            , fTextRange(master, text)
            , fRunReference(master, run)
            , fStart(start)
            , fEnd(end)
            , fWidth(width)
            , fSpacing(0)
            , fHeight(height)
            , fWhiteSpaces(false)
            , fBreakType(None) {}

    ~Cluster() = default;

    void setMaster(ParagraphImpl* master) {
        fMaster = master;
        this->fTextRange.setMaster(master);
    }
    SkScalar sizeToChar(const char* ch) const;
    SkScalar sizeFromChar(const char* ch) const;

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
    Run* run() const { return fRunReference.reference(fMaster); }
    size_t startPos() const { return fStart; }
    size_t endPos() const { return fEnd; }
    SkScalar width() const { return fWidth; }
    SkScalar trimmedWidth() const { return fWidth - fSpacing; }
    SkScalar lastSpacing() const { return fSpacing; }
    SkScalar height() const { return fHeight; }
    SkSpan<const char> text() const { return fTextRange.span(); }
    size_t size() const { return fEnd - fStart; }

    SkScalar trimmedWidth(size_t pos) const;

    void shift(SkScalar offset) const { this->run()->shift(this, offset); }

    void setIsWhiteSpaces();

    bool contains(const char* ch) const { return ch >= fTextRange.begin() && ch < fTextRange.end(); }

    bool belongs(SkSpan<const char> text) const {
        return fTextRange.begin() >= text.begin() && fTextRange.end() <= text.end();
    }

    bool startsIn(SkSpan<const char> text) const {
        return fTextRange.begin() >= text.begin() && fTextRange.begin() < text.end();
    }

private:

    ParagraphImpl* fMaster;
    StableRange<ParagraphImpl, const char, &accessText> fTextRange;
    StableReference<ParagraphImpl, Run, &accessRunRef> fRunReference;

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

    LineMetrics(const SkFont& font) {
        SkFontMetrics metrics;
        font.getMetrics(&metrics);
        fAscent = metrics.fAscent;
        fDescent = metrics.fDescent;
        fLeading = metrics.fLeading;
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
