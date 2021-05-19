// Copyright 2021 Google LLC.

#include "experimental/sktext/src/Processor.h"
#include <stack>
#include "experimental/sktext/src/Shaper.h"
#include "experimental/sktext/src/Visitor.h"
#include "experimental/sktext/src/Wrapper.h"

namespace skia {
namespace text {

bool Processor::computeCodeUnitProperties() {

    fCodeUnitProperties.push_back_n(fText.size() + 1, CodeUnitFlags::kNoCodeUnitFlag);

    fUnicode = std::move(SkUnicode::Make());
    if (nullptr == fUnicode) {
        return false;
    }

    // Create utf8 -> utf16 conversion table
    auto text8 = fUnicode->convertUtf16ToUtf8(fText);
    size_t utf16Index = 0;
    fUTF16FromUTF8.push_back_n(text8.size() + 1, utf16Index);
    fUnicode->forEachCodepoint(text8.c_str(), text8.size(),
        [this, &utf16Index](SkUnichar unichar, int32_t start, int32_t end) {
            for (auto i = start; i < end; ++i) {
                fUTF16FromUTF8[i] = utf16Index;
            }
            ++utf16Index;
       });
    fUTF16FromUTF8[text8.size()] = utf16Index;

    // Get white spaces
    fUnicode->forEachCodepoint(fText.c_str(), fText.size(),
       [this](SkUnichar unichar, int32_t start, int32_t end) {
            if (fUnicode->isWhitespace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfWhiteSpace;
                }
            }
       });

    // Get line breaks
    fUnicode->forEachBreak(fText.c_str(), fText.size(), SkUnicode::BreakType::kLines,
                           [&](SkBreakIterator::Position pos, SkBreakIterator::Status status){
                                fCodeUnitProperties[pos] |= (status == (SkBreakIterator::Status)SkUnicode::LineBreakType::kHardLineBreak
                                                               ? CodeUnitFlags::kHardLineBreakBefore
                                                               : CodeUnitFlags::kSoftLineBreakBefore);
                            });

    // Get graphemes
    fUnicode->forEachBreak(fText.c_str(), fText.size(), SkUnicode::BreakType::kGraphemes,
                           [&](SkBreakIterator::Position pos, SkBreakIterator::Status){
                                fCodeUnitProperties[pos]|= CodeUnitFlags::kGraphemeStart;
                            });

    return true;
}

void Processor::markGlyphs() {
    for (auto& run : fRuns) {
        for (auto index : run.fClusters) {
            fCodeUnitProperties[index] |= CodeUnitFlags::kGlyphStart;
        }
    }
}

void Processor::resetLines() {
    fLines.reset();
}

} // namespace text
} // namespace skia
