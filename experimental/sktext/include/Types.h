// Copyright 2021 Google LLC.
#ifndef Types_DEFINED
#define Types_DEFINED

#include <algorithm>
#include <cstddef>
#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkTo.h"

namespace skia {
namespace text {
enum class TextAlign {
    kLeft,
    kRight,
    kCenter,
    kJustify,
    kStart,
    kEnd,
};

enum class TextDirection {
    kRtl,
    kLtr,
};

enum class LineBreakType {
  kSortLineBreakBefore,
  kHardLineBreakBefore,
};

enum CodeUnitFlags {
    kNoCodeUnitFlag = 0x0,
    kPartOfWhiteSpace = 0x1,
    kGraphemeStart = 0x2,
    kSoftLineBreakBefore = 0x4,
    kHardLineBreakBefore = 0x8,
    kGlyphClusterStart = 0x16,  // This information we get from SkShaper
};

class Range {
public:
    Range() : fStart(0), fEnd(0) { }
    Range(size_t start, size_t end) : fStart(start) , fEnd(end) { }

    int width() {
        return fEnd >= fStart ? SkToInt(fEnd - fStart) : - SkToInt(fStart - fEnd);
    }

    void clean() {
        fStart = 0;
        fEnd = 0;
    }

    bool isEmpty() {
        return fEnd == fStart;
    }

    void merge(Range tail) {
        SkASSERT(this->fEnd == tail.fStart);
        fStart = std::min(this->fStart, tail.fStart);
        fEnd = std::max(this->fEnd, tail.fEnd);
    }

    Range intersect(Range other) {
        return Range(std::max(this->fStart, other.fStart), std::min(this->fEnd, other.fEnd));
    }

    size_t fStart;
    size_t fEnd;
};

const size_t EMPTY_INDEX = std::numeric_limits<size_t>::max();
const Range EMPTY_RANGE = Range(EMPTY_INDEX, EMPTY_INDEX);

}
}

namespace sknonstd {
template <> struct is_bitmask_enum<skia::text::CodeUnitFlags> : std::true_type {};
}
#endif
