// Copyright 2021 Google LLC.
#ifndef Iterators_DEFINED
#define Iterators_DEFINED

#include "experimental/sktext/include/Processor.h"
#include "experimental/sktext/include/Types.h"
#include "src/core/SkSpan.h"
#include "src/core/SkTDPQueue.h"

namespace skia {
namespace text {

class TextIterator {
public:
    TextIterator() : fProcessor(nullptr), fIndex(0), fType(CodeUnitFlags::kNoCodeUnitFlag) { }
    TextIterator(Processor* processor, CodeUnitFlags type, const Range& textRange) : fProcessor(processor), fIndex(textRange.fStart), fType(type), fTextRange(textRange) { }
    virtual void reset() { fIndex = fTextRange.fStart; }
    virtual bool eof() const { return fTextRange.leftToRight() ? fIndex >= fTextRange.fEnd : fIndex <= fTextRange.fStart; }
    virtual size_t current() const { return fIndex; }
    virtual size_t next() {
        while (!eof() && !fProcessor->hasProperty(fIndex, fType)) {
            if (fTextRange.leftToRight()) {
                ++fIndex;
            } else {
                ++fIndex;
            }
        }
        return fIndex;
    }
    static bool CompareIterators(TextIterator const& a, TextIterator const& b) {
        size_t aEnd = a.current();
        size_t bEnd = b.current();
        return aEnd  <= bEnd;
    }
protected:
    Processor* fProcessor;
    size_t fIndex;
    CodeUnitFlags fType;
    Range fTextRange;
};

// Custom iterator (over sorted ranges)
// TODO: Implement
class CustomIterator : public TextIterator {
public: CustomIterator(Processor* processor, std::vector<size_t> ranges)
            : TextIterator(processor, CodeUnitFlags::kNoCodeUnitFlag)
            , fRanges(std::move(ranges))
            , fCurrentRange(0) { }

    virtual size_t next() override {
        if (fRanges.empty()) { return 0; }
        if (fRanges.size() == fCurrentRange) { return fIndex; }
        fIndex = fRanges[fCurrentRange];
        ++fCurrentRange;
        return fIndex;
    }
private:
    std::vector<size_t> fRanges;
    size_t fCurrentRange;
};

class GraphemeIterator : public TextIterator {
public: GraphemeIterator(Processor* processor) : TextIterator(processor, CodeUnitFlags::kGraphemeStart) { }
};

class GlyphIterator : public TextIterator {
public: GlyphIterator(Processor* processor) : TextIterator(processor, CodeUnitFlags::kGlyphStart) { }
};

class GlyphClusterIterator : public TextIterator {
public: GlyphClusterIterator(Processor* processor) : TextIterator(processor, CodeUnitFlags::kGlyphClusterStart) { }
};

class LineBreakIterator : public TextIterator {
public: LineBreakIterator(Processor* processor) : TextIterator(processor, CodeUnitFlags::kSoftLineBreakBefore | CodeUnitFlags::kHardLineBreakBefore) { }
};

class WhitespaceIterator : public TextIterator {
public: WhitespaceIterator(Processor* processor) : TextIterator(processor, CodeUnitFlags::kPartOfWhiteSpace) { }
};


}
}
#endif // Iterators_DEFINED
