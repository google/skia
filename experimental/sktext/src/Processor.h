
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

class Processor {

public:

    Processor(std::u16string text, std::vector<FontBlock> fontBlocks, TextDirection defaultTextDirection, TextAlign textAlign)
        : fText(std::move(text))
        , fFontBlocks(std::move(fontBlocks))
        , fDefaultTextDirection(defaultTextDirection)
        , fTextAlign(textAlign)
        , fUnicode(nullptr) {}

    ~Processor() = default;

    bool computeCodeUnitProperties();
    void resetLines();

private:
    friend class Line;
    friend class Shaper;
    friend class Wrapper;
    friend class Visitor;
    friend class Paint;

    SkUnicode* getUnicode() { return fUnicode == nullptr ? nullptr : fUnicode.get(); }
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
    void markGlyphs();

    std::u16string fText;
    std::vector<FontBlock> fFontBlocks;
    TextDirection fDefaultTextDirection;
    TextAlign fTextAlign;

    SkTArray<TextRun, false> fRuns;
    SkTArray<Line, false> fLines;

    std::unique_ptr<SkUnicode> fUnicode;
    SkTArray<CodeUnitFlags, true> fCodeUnitProperties;
    SkTArray<size_t, true> fUTF16FromUTF8;
};

}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
