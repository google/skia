// Copyright 2021 Google LLC.
#ifndef Processor_DEFINED
#define Processor_DEFINED

#include "experimental/sktext/src/Line.h"
#include <string>
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/TextRun.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/src/SkUnicode.h"

namespace skia {
namespace text {

class UnicodeText;
class Text {
public:
    static std::unique_ptr<UnicodeText> parse(SkSpan<uint16_t> utf16);
};

class ShapedText;
class UnicodeText : public SkShaper::RunHandler {

public:
    std::unique_ptr<ShapedText> shape(SkSpan<Block> blocks,
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
        fCurrentRun = std::make_unique<TextRun>(info);
        return fCurrentRun->newRunBuffer();
    }

    SkFont createFont(const Block& block);

    SkTArray<size_t, true> fUTF16FromUTF8;
    SkTArray<CodeUnitFlags, true> fCodeUnitProperties;
    SkString fText8;
    std::unique_ptr<SkUnicode> fUnicode;
    std::unique_ptr<TextRun> fCurrentRun;
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
    sk_sp<FormattedText> format(TextAlign, TextDirection);
    SkSize size() const { return fSize; }
    size_t countLines() const { return fLines.size(); }
private:
    friend class ShapedText;
    WrappedText() : fSize(SkSize::MakeEmpty()) { }
    void addLine(Stretch& stretch, Stretch& spaces, SkUnicode* unicode);
    SkTArray<TextRun, false> fRuns;
    SkTArray<Line, false> fLines;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
    SkSize fSize;
};

class FormattedText : public SkRefCnt {
public:
    SkSize  size() const { return fSize; }

    std::tuple<const Line*, const TextRun*, GlyphIndex, SkRect> indexToAdjustedGraphemePosition(TextIndex textIndex) const;
    TextIndex positionToAdjustedGraphemeIndex(SkPoint xy) const;
    size_t lineIndex(const Line* line) const {
        return line - fLines.data();
    }
    size_t countLines() const { return fLines.size(); }

    // This one does not make sense; how would we get glyphRange in the first place?
    // It has to point to the same run or we cannot even index it
    std::vector<TextRange> adjustIndicesAndComputeTextRanges(GlyphRange glyphRange) const;

    class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void onBeginLine(TextRange, float baselineY) = 0;
        virtual void onEndLine(TextRange, float baselineY) = 0;
        virtual void onGlyphRun(SkFont font,
                                TextRange textRange,
                                int glyphCount,
                                const uint16_t glyphs[],
                                const SkPoint  positions[],
                                const SkPoint offsets[]) = 0;
        virtual void onPlaceholder(TextRange, const SkRect& bounds) = 0;
    };

    // Visit runs as is by lines
    void visit(Visitor*) const;
    // Visit chunked runs
    void visit(Visitor*, SkSpan<size_t> blocks) const;

private:
    friend class WrappedText;
    FormattedText() { }
    SkTArray<TextRun, false> fRuns;
    SkTArray<Line, false> fLines;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
    SkSize fSize;
};

}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
