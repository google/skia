// Copyright 2021 Google LLC.
#ifndef Types_DEFINED
#define Types_DEFINED

#include <algorithm>
#include <cstddef>
#include "include/core/SkFont.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkTo.h"

namespace skia {
namespace text {
enum class TextAlign {
    kLeft,
    kRight,
    kCenter,
    kJustify,
    kStart,
    kEnd,
    kNothing,
};

enum class TextDirection {
    kRtl,
    kLtr,
};

// This enum lists all possible ways to query output positioning
enum class PositionType {
    kRandomText,
    kGraphemeCluster, // Both the edge of the glyph cluster and the text grapheme
    kGlyphCluster,
    kGlyph,
    kGlyphPart
};

enum class LineBreakType {
  kSortLineBreakBefore,
  kHardLineBreakBefore,
};

enum class LogicalRunType {
    kText,
    kLineBreak
};

enum class GlyphUnitFlags : uint8_t {
    kNoGlyphUnitFlag = (1 << 0),
    //kPartOfWhiteSpace = (1 << 1),
    //kGraphemeStart = (1 << 2),
    //kSoftLineBreakBefore = (1 << 3),
    //kHardLineBreakBefore = (1 << 4),
    kGlyphClusterStart = (1 << 5),
    kGraphemeClusterStart = (1 << 6),
};

typedef size_t TextIndex;
typedef size_t GlyphIndex;
typedef size_t RunIndex;
typedef size_t LineIndex;
const size_t EMPTY_INDEX = std::numeric_limits<size_t>::max();

template <typename T>
class Range {
public:
    Range() : fStart(0), fEnd(0) { }
    Range(T start, T end) : fStart(start) , fEnd(end) { }

    bool operator==(Range<T> other) {
        return fStart == other.fStart && fEnd == other.fEnd;
    }

    void clean() {
        fStart = 0;
        fEnd = 0;
    }

    bool isEmpty() const {
        return fEnd == fStart;
    }

    bool contains(T index) const {
        return index >= fStart && index < fEnd;
    }

    bool contains(Range<T> range) const {
        return range.fStart >= fStart && range.fEnd < fEnd;
    }

    // For RTL ranges start >= end
    int width() const {
        return SkToInt(fEnd - fStart);
    }

    void merge(Range tail) {
        SkASSERT(this->fEnd == tail.fStart || this->fStart == tail.fEnd);
        this->fStart = std::min(this->fStart, tail.fStart);
        this->fEnd = std::max(this->fEnd, tail.fEnd);
    }

    void intersect(Range other) {
        this->fStart = std::max(this->fStart, other.fStart);
        this->fEnd = std::min(this->fEnd, other.fEnd);
        if (this->fStart > this->fEnd) {
            // There is nothing in the intersection; make it empty
            this->fEnd = this->fStart;
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
struct DirTextRange : public TextRange {
    DirTextRange(TextRange textRange, bool leftToRight)
        : TextRange(textRange)
        , fLeftToRight(leftToRight) { }
    DirTextRange(TextIndex start, TextIndex end, bool leftToRight)
        : TextRange(start, end)
        , fLeftToRight(leftToRight) { }

    bool before(TextIndex index) const {
        return fLeftToRight ? index >= fEnd : index < fStart;
    }
    bool after(TextIndex index) const {
        return fLeftToRight ? index < fEnd : index >= fStart;
    }
    bool left() const {
        return fLeftToRight ? fStart : fEnd;
    }
    bool right() const {
        return fLeftToRight ? fEnd : fStart;
    }
    TextRange normalized() const {
        return fLeftToRight ? TextRange(fStart, fEnd) : TextRange(fEnd, fStart);
    }
    bool fLeftToRight;
};

const Range<size_t> EMPTY_RANGE = Range<size_t>(EMPTY_INDEX, EMPTY_INDEX);

// Blocks
enum BlockType {
    kFontChain,
    kPlaceholder,
};

struct Placeholder {
    SkSize  dimensions;
    float   yOffsetFromBaseline;
};

class FontChain : public SkRefCnt {
public:
    // Returns the number of faces in the chain. Always >= 1
    virtual size_t count() const = 0;
    virtual sk_sp<SkTypeface> operator[](size_t index) const = 0;
    virtual float fontSize() const = 0;
    virtual SkString locale() const = 0;
};

struct FontBlock {
    FontBlock(uint32_t count, sk_sp<FontChain> fontChain)
        : type(BlockType::kFontChain)
        , charCount(count)
        , chain(fontChain) { }
    FontBlock() : FontBlock(0, nullptr) { }
    FontBlock(FontBlock& block) {
        this->type = block.type;
        this->charCount = block.charCount;
        this->chain = block.chain;
    }
    ~FontBlock() { }

    BlockType  type;
    uint32_t   charCount;
    union {
        sk_sp<FontChain>  chain;
        Placeholder placeholder;
    };
};

struct ResolvedFontBlock {
    ResolvedFontBlock(TextRange textRange, sk_sp<SkTypeface> typeface, SkScalar size, SkFontStyle fontStyle)
        : textRange(textRange)
        , typeface(typeface)
        , size(size)
        , style(fontStyle) { }

    TextRange textRange;
    sk_sp<SkTypeface> typeface;
    float size;
    SkFontStyle style;
};

}  // namespace text
}  // namespace skia

namespace sknonstd {
template <> struct is_bitmask_enum<skia::text::GlyphUnitFlags> : std::true_type {};
}

#endif
