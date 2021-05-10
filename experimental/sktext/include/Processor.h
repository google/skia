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
#include "modules/skshaper/src/SkUnicode.h"

namespace skia {
namespace text {

class Block {
public:
    enum BlockType {
        kFont,
        kDecor,
        kFormat,
    };
    Block(BlockType type, TextRange range)
        : fType(type)
        , fRange(range) { }
    BlockType fType;
    TextRange fRange;
};

class FontBlock : public Block {
public:
    FontBlock(const SkString& family, SkScalar size, SkFontStyle style, TextRange range)
        : Block(Block::kFont, range)
        , fFontFamily(family)
        , fFontSize(size)
        , fFontStyle(style) { }
    SkString fFontFamily;
    SkScalar fFontSize;
    SkFontStyle fFontStyle;

    // TODO: Features
};

class DecorBlock : public Block {
public:
    DecorBlock(const SkPaint* foreground, const SkPaint* background, TextRange range)
        : Block(Block::kDecor, range)
        , fForegroundColor(foreground)
        , fBackgroundColor(background) { }

    DecorBlock(TextRange range)
        : DecorBlock(nullptr, nullptr, range) { }

    const SkPaint* fForegroundColor;
    const SkPaint* fBackgroundColor;
    // Everything else
};

class TextFormatStyle {
public:
    TextFormatStyle(TextAlign textAlign, TextDirection defaultTextDirection)
        : fTextAlign(textAlign), fDefaultTextDirection(defaultTextDirection) { }
    TextAlign fTextAlign;
    TextDirection fDefaultTextDirection;
};

class TextFontStyle {
public:
    TextFontStyle(TextDirection textDirection, sk_sp<SkFontMgr> fontManager)
        : fTextDirection(textDirection), fFontManager(fontManager) { }
    TextDirection fTextDirection;
    sk_sp<SkFontMgr> fFontManager;
};

class TextOutput {
 public:
    TextOutput(sk_sp<SkTextBlob> textBlob, const SkPaint& paint, SkSize offset) : fTextBlob(std::move(textBlob)), fPaint(paint), fOffset(offset) { }
    sk_sp<SkTextBlob> fTextBlob;
    SkPaint fPaint;
    SkSize fOffset;
};

class Processor {

public:

    Processor(std::u16string text)
        : fText(std::move(text))
        , fUnicode(nullptr) {}

    ~Processor() = default;

    // All the Unicode information
    SkUnicode* getUnicode() { return fUnicode == nullptr ? nullptr : fUnicode.get(); }

    // Once the text is measured you can get approximate sizing for it
    bool shape(TextFontStyle fontStyle, SkTArray<FontBlock, true> fontBlocks);

    // Once the text is fit into the required box you can get the real sizes for it
    bool wrap(SkScalar width, SkScalar height);

    // Once the text is formatted you can get it's glyphs info line by line
    bool format(TextFormatStyle textFormatStyle);

    // Once the text is decorated you can iterate it by segments (intersect of run, decor block and line)
    bool decorate(SkTArray<DecorBlock, true> decorBlocks);

    bool hasProperty(size_t index, CodeUnitFlags flag) {
        return (fCodeUnitProperties[index] & flag) == flag;
    }

    bool isHardLineBreak(size_t index) {
        return this->hasProperty(index, CodeUnitFlags::kHardLineBreakBefore);
    }

    bool isSoftLineBreak(size_t index) {
        return index != 0 && this->hasProperty(index, CodeUnitFlags::kSoftLineBreakBefore);
    }

    bool isWhitespaces(TextRange range) {
        if (range.leftToRight()) {
            for (auto i = range.fStart; i < range.fEnd; ++i) {
                if (!this->hasProperty(i, CodeUnitFlags::kPartOfWhiteSpace)) {
                    return false;
                }
            }
        } else {
            for (auto i = range.fStart; i > range.fEnd; --i) {
                if (!this->hasProperty(i, CodeUnitFlags::kPartOfWhiteSpace)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool isClusterEdge(size_t index) {
        return this->hasProperty(index, CodeUnitFlags::kGraphemeStart) ||
               this->hasProperty(index, CodeUnitFlags::kGlyphStart);
    }

    void adjustLeft(size_t* index) {
        SkASSERT(index != nullptr);
        while (*index != 0) {
            if (isClusterEdge(*index)) {
                return;
            }
            --index;
        }
    }

    TextRun& run(const size_t index) { return fRuns[index]; }

    // Simplification (using default font manager, default font family and default everything possible)
    static bool drawText(std::u16string text, SkCanvas* canvas, SkScalar x, SkScalar y);
    static bool drawText(std::u16string text, SkCanvas* canvas, SkScalar width);
    static bool drawText(std::u16string text, SkCanvas* canvas, TextFormatStyle textFormat, SkColor foreground, SkColor background, const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle, SkScalar x, SkScalar y);
    static bool drawText(std::u16string text, SkCanvas* canvas,
                         TextFormatStyle textFormat, SkColor foreground, SkColor background, const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle,
                         SkSize reqSize, SkScalar x, SkScalar y);

    void sortDecorBlocks(SkTArray<DecorBlock, true>& decorBlocks);

    bool computeCodeUnitProperties();

    void markGlyphs();

    // Iterating through the output glyphs and breaking the runs by units flag (no breaking if units == CodeUnitFlags::kNonExistingFlag)
    template<typename Visitor>
    void iterateByVisualOrder(CodeUnitFlags units, Visitor visitor);
    template<typename Visitor>
    void iterateByVisualOrder(SkTArray<DecorBlock, true>& decorBlocks, Visitor visitor);

private:
    friend class TextIterator;
    friend class Shaper;
    friend class Wrapper;

    std::u16string fText;
    SkTArray<FontBlock, true> fFontBlocks;
    //TextFormatStyle fTextFormatStyle;
    //TextFontStyle fTextFontStyle;
    SkTArray<TextRun, false> fRuns;
    SkTArray<Line, false> fLines;
    SkTArray<TextOutput, false> fTextOutputs;

    std::unique_ptr<SkUnicode> fUnicode;
    SkTArray<CodeUnitFlags, true> fCodeUnitProperties;
    SkTArray<size_t, true> fUTF16FromUTF8;
};

}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
