// Copyright 2021 Google LLC.
#ifndef Processor_DEFINED
#define Processor_DEFINED
#include <string>
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/TextRun.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skunicode/include/SkUnicode.h"

namespace skia {
namespace text {

class UnicodeText;
class Text {
public:
    static std::unique_ptr<UnicodeText> parse(SkSpan<uint16_t> utf16);
};

// Font iterator that finds all formatting marks
// and breaks runs on them (so we can select and interpret them later)
class FormattingFontIterator final : public SkShaper::FontRunIterator {
public:
    FormattingFontIterator(TextIndex textCount, SkSpan<FontBlock> fontBlocks, SkSpan<TextIndex> marks)
        : fTextCount(textCount)
        , fFontBlocks(fontBlocks)
        , fFormattingMarks(std::move(marks))
        , fCurrentBlock(fontBlocks.begin())
        , fCurrentMark(marks.begin())
        , fCurrentFontIndex(0)
        , fCurrentFont(fCurrentBlock->createFont())
    { }

    void consume() override {
        SkASSERT(fCurrentBlock < fFontBlocks.end());
        SkASSERT(fCurrentMark < fFormattingMarks.end());

        if (fCurrentFontIndex <= *fCurrentMark) {
            if (fCurrentFontIndex == *fCurrentMark) {
                ++fCurrentMark;
            }
            ++fCurrentBlock;
            if (fCurrentBlock < fFontBlocks.end()) {
                fCurrentFontIndex += fCurrentBlock->charCount;
                fCurrentFont = fCurrentBlock->createFont();
            }
        } else {
            ++fCurrentMark;
        }
    }
    size_t endOfCurrentRun() const override {
        SkASSERT(fCurrentMark != fFormattingMarks.end() || fCurrentBlock != fFontBlocks.end());
        if (fCurrentMark == fFormattingMarks.end()) {
            return fCurrentFontIndex;
        } else if (fCurrentBlock == fFontBlocks.end()) {
            return *fCurrentMark;
        } else {
            return fCurrentFontIndex <= *fCurrentMark ? fCurrentFontIndex : *fCurrentMark;
        }
    }
    bool atEnd() const override {
        return (fCurrentBlock != fFontBlocks.end() ? fCurrentFontIndex == fTextCount : true) &&
                (fCurrentMark != fFormattingMarks.end() ? *fCurrentMark == fTextCount : true);
    }

    const SkFont& currentFont() const override {
        return fCurrentFont;
    }

private:
    TextIndex const fTextCount;
    SkSpan<FontBlock> fFontBlocks;
    SkSpan<TextIndex> fFormattingMarks;
    FontBlock* fCurrentBlock;
    TextIndex* fCurrentMark;
    TextIndex fCurrentFontIndex;
    SkFont fCurrentFont;
};

class ShapedText;
class UnicodeText : public SkShaper::RunHandler {

public:
    std::unique_ptr<ShapedText> shape(SkSpan<FontBlock> blocks,
                                      TextDirection textDirection);

    bool shapeParagraph(TextRange text,
                        SkSpan<FontBlock> blocks,
                        TextDirection textDirection);

    bool hasProperty(size_t index, CodeUnitFlags flag) {
        return (fCodeUnitProperties[index] & flag) == flag;
    }
    bool isHardLineBreak(size_t index) {
        return this->hasProperty(index, CodeUnitFlags::kHardLineBreakBefore);
    }
    bool isSoftLineBreak(size_t index) {
        return index != 0 && this->hasProperty(index, CodeUnitFlags::kSoftLineBreakBefore);
    }
    SkUnicode* getUnicode() const { return fUnicode.get(); }

private:
    friend class Text;
    UnicodeText() : fCurrentRun(nullptr) { }
    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    void commitLine() override {}
    void commitRunBuffer(const RunInfo&) override;
    Buffer runBuffer(const RunInfo& info) override {
        fCurrentRun = std::make_unique<TextRun>(info, fParagraphTextStart, fRunGlyphStart);
        return fCurrentRun->newRunBuffer();
    }

    SkFont createFont(const FontBlock& fontBlock);

    SkTArray<size_t, true> fUTF16FromUTF8;
    SkTArray<size_t, true> fUTF8FromUTF16;
    SkTArray<CodeUnitFlags, true> fCodeUnitProperties;
    SkString fText8;
    std::u16string fText16;
    std::unique_ptr<SkUnicode> fUnicode;
    std::unique_ptr<TextRun> fCurrentRun;
    TextIndex fParagraphTextStart;
    SkScalar fRunGlyphStart;
    std::unique_ptr<ShapedText> fShapedText;
};

class WrappedText;
class ShapedText {
public:
    std::unique_ptr<WrappedText> wrap(float width, float height, SkUnicode* unicode);
    bool isClusterEdge(size_t index) const {
        return (fGlyphUnitProperties[index] & GlyphUnitFlags::kGlyphClusterStart) == GlyphUnitFlags::kGlyphClusterStart;
    }
    void adjustLeft(size_t* index) const {
        SkASSERT(index != nullptr);
        while (*index != 0) {
            if (isClusterEdge(*index)) {
                return;
            }
            --index;
        }
    }
    void adjustRight(size_t* index) const {
        SkASSERT(index != nullptr);
        while (*index < this->fGlyphUnitProperties.size()) {
            if (isClusterEdge(*index)) {
                return;
            }
            ++index;
        }
    }
    bool hasProperty(size_t index, GlyphUnitFlags flag) {
        return (fGlyphUnitProperties[index] & flag) == flag;
    }
    bool isHardLineBreak(size_t index) {
        return this->hasProperty(index, GlyphUnitFlags::kHardLineBreakBefore);
    }
    bool isSoftLineBreak(size_t index) {
        return index != 0 && this->hasProperty(index, GlyphUnitFlags::kSoftLineBreakBefore);
    }
    bool isWhitespaces(TextRange range) {
        if (range.leftToRight()) {
            for (auto i = range.fStart; i < range.fEnd; ++i) {
                if (!this->hasProperty(i, GlyphUnitFlags::kPartOfWhiteSpace)) {
                    return false;
                }
            }
        } else {
            for (auto i = range.fStart; i > range.fEnd; --i) {
                if (!this->hasProperty(i, GlyphUnitFlags::kPartOfWhiteSpace)) {
                    return false;
                }
            }
        }
        return true;
}

private:
    friend class UnicodeText;
    ShapedText() { }
    SkTArray<TextRun, false> fRuns;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
};

class FormattedText;
class WrappedText {
public:
    sk_sp<FormattedText> format(TextAlign textAlign, TextDirection textDirection);
    SkSize actualSize() const { return fActualSize; }
    size_t countLines() const { return fLines.size(); }
private:
    friend class ShapedText;
    WrappedText() : fActualSize(SkSize::MakeEmpty()) { }
    void addLine(Stretch& stretch, Stretch& spaces, SkUnicode* unicode, bool hardLineBreak);
    SkTArray<TextRun, false> fRuns;
    SkTArray<Line, false> fLines;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
    SkSize fActualSize;
};

class FormattedText : public SkRefCnt {
public:
    SkSize  actualSize() const { return fActualSize; }

    Position adjustedPosition(PositionType positionType, SkPoint point) const;

    Position adjustedPosition(PositionType positionType, TextIndex textIndex) const;

    bool recalculateBoundaries(Position& position) const;

    const TextRun* visuallyPreviousRun(size_t lineIndex, const TextRun* run) const;
    const TextRun* visuallyNextRun(size_t lineIndex, const TextRun* run) const;
    const TextRun* visuallyFirstRun(size_t lineIndex) const;
    const TextRun* visuallyLastRun(size_t lineIndex) const;
    bool isVisuallyFirst(size_t lineIndex, const TextRun* run) const;
    bool isVisuallyLast(size_t lineIndex, const TextRun* run) const;

    Position previousElement(Position element) const;
    Position nextElement(Position current) const;
    Position firstElement(PositionType positionType) const;
    Position lastElement(PositionType positionType) const;

    bool isFirstOnTheLine(Position element) const;
    bool isLastOnTheLine(Position element) const;

    size_t lineIndex(const Line* line) const { return line - fLines.data(); }
    size_t countLines() const { return fLines.size(); }
    const Line* line(size_t lineIndex) const { return fLines.empty() ? nullptr : &fLines[lineIndex]; }
    size_t runIndex(const TextRun* run) const { return run == nullptr ? EMPTY_INDEX : run - fRuns.data(); }

    bool hasProperty(size_t index, GlyphUnitFlags flag) const {
        return (fGlyphUnitProperties[index] & flag) == flag;
    }

    class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void onBeginLine(TextRange lineText, float baselineY, float horizontalOffset) { }
        virtual void onEndLine(TextRange lineText, float baselineY) { }
        virtual void onGlyphRun(SkFont font,
                                TextRange textRange,
                                SkRect boundingRect,
                                int glyphCount,
                                const uint16_t glyphs[],
                                const SkPoint  positions[],
                                const SkPoint offsets[]) {
            SkTextBlobBuilder builder;
            const auto& blobBuffer = builder.allocRunPos(font, SkToInt(glyphCount));
            sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
            sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
            fTextBlobs.emplace_back(builder.make());
        }
        virtual void onPlaceholder(TextRange, const SkRect& bounds) { }

        void buildTextBlobs(FormattedText* formattedText) {
            fTextBlobs.clear();
            formattedText->visit(this);
        }
        std::vector<sk_sp<SkTextBlob>>& getTextBlobs() { return fTextBlobs; }

    private:
        std::vector<sk_sp<SkTextBlob>> fTextBlobs;
    };

    // Visit runs as is by lines
    void visit(Visitor*) const;
    // Visit chunked runs
    void visit(Visitor*, SkSpan<size_t> blocks) const;

    void paint(SkCanvas* canvas, SkPaint& foreground, SkPoint xy, bool rebuild = true) {

        if (fVisitor == nullptr) {
            fVisitor = std::make_unique<Visitor>();
            rebuild = true;
        }
        if (rebuild) {
            fVisitor->buildTextBlobs(this);
        }
        for (auto& textBlob : fVisitor->getTextBlobs()) {
            canvas->drawTextBlob(textBlob, xy.fX, xy.fY, foreground);
        }
    }

private:

    void adjustTextRange(Position* position) const;

    friend class WrappedText;
    FormattedText() { }
    SkTArray<TextRun, false> fRuns;
    SkTArray<Line, false> fLines;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
    SkSize fActualSize;
    std::unique_ptr<Visitor> fVisitor;
};

}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
