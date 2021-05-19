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

enum CodeUnitFlags : uint32_t {
    kNoCodeUnitFlag = 0x000,
    kPartOfWhiteSpace = 0x001,
    kGraphemeStart = 0x002,
    kSoftLineBreakBefore = 0x004,
    kHardLineBreakBefore = 0x008,
    // This information we get from SkShaper
    kGlyphStart = 0x010,
    kGlyphClusterStart = 0x020,
    kNonExistingFlag = 0x040,
};

typedef size_t TextIndex;
typedef size_t GlyphIndex;
const size_t EMPTY_INDEX = std::numeric_limits<size_t>::max();

template <typename T>
class Range {
public:
    Range() : fStart(0), fEnd(0) { }
    Range(T start, T end) : fStart(start) , fEnd(end) { }

    bool leftToRight() const {
        return fEnd >= fStart;
    }

    void normalize() {
        if (!this->leftToRight()) {
            std::swap(this->fStart, this->fEnd);
        }
    }

    // For RTL ranges start >= end
    int width() const {
        return leftToRight() ? SkToInt(fEnd - fStart) : SkToInt(fStart - fEnd);
    }

    void clean() {
        fStart = 0;
        fEnd = 0;
    }

    bool isEmpty() const {
        return fEnd == fStart;
    }

    void merge(Range tail) {
        auto ltr1 = this->leftToRight();
        auto ltr2 = tail.leftToRight();
        this->normalize();
        tail.normalize();
        SkASSERT(this->fEnd == tail.fStart || this->fStart == tail.fEnd);
        this->fStart = std::min(this->fStart, tail.fStart);
        this->fEnd = std::max(this->fEnd, tail.fEnd);
        // TODO: Merging 2 different directions
        if (!ltr1 || !ltr2) {
            std::swap(this->fStart, this->fEnd);
            std::swap(tail.fStart, tail.fEnd);
        }
    }

    void intersect(Range other) {
        auto ltr = this->leftToRight();

        this->normalize();
        other.normalize();
        this->fStart = std::max(this->fStart, other.fStart);
        this->fEnd = std::min(this->fEnd, other.fEnd);
        if (this->fStart > this->fEnd) {
            // There is nothing in the intersection; make it empty
            this->fEnd = this->fStart;
        } else if (!ltr) {
            std::swap(this->fStart, this->fEnd);
        }
    }

    template<typename Visitor>
    void iterate(Visitor visitor) {
        if (this->leftToRight()) {
            for (auto index = this->fStart; index < this->fEnd; ++index) {
                visitor(index);
            }
        } else {
            for (auto index = this->fStart; index < this->fEnd; --index) {
                visitor(index);
            }
        }
    }

    T fStart;
    T fEnd;
};

typedef Range<TextIndex> TextRange;
typedef Range<GlyphIndex> GlyphRange;
const Range EMPTY_RANGE = Range(EMPTY_INDEX, EMPTY_INDEX);

}  // namespace text
}  // namespace skia

namespace sknonstd {
template <> struct is_bitmask_enum<skia::text::CodeUnitFlags> : std::true_type {};
}

#endif
