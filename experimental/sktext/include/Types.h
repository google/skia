// Copyright 2021 Google LLC.
#ifndef Types_DEFINED
#define Types_DEFINED

#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
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

class FontChain : public SkRefCnt {
public:
    // Returns the number of faces in the chain. Always >= 1
    virtual size_t count() const = 0;
    virtual sk_sp<SkTypeface> operator[](size_t index) const = 0;
};

class TrivialFontChain : public FontChain {
public:
    TrivialFontChain(sk_sp<SkTypeface> typeface)
        : fTypeface(std::move(typeface)) { }

    size_t count() const override { return 1;  }
    sk_sp<SkTypeface> operator[](size_t index) const override {
        SkASSERT(index == 0);
        return  fTypeface;
    }

private:
    sk_sp<SkTypeface> fTypeface;
};

class ListFontChain : public FontChain {
public:
    ListFontChain(std::vector<sk_sp<SkTypeface>> typefaces)
        : fTypefaces(typefaces) { }

    size_t count() const override { return fTypefaces.size();  }
    sk_sp<SkTypeface> operator[](size_t index) const override {
        SkASSERT(index >= 0 && index < fTypefaces.size());
        return fTypefaces[index];
    }

private:
    std::vector<sk_sp<SkTypeface>> fTypefaces;
};

class TextMetrics {

public:
  TextMetrics() {
      clean();
  }

  TextMetrics(const SkFont& font) {

      SkFontMetrics metrics;
      font.getMetrics(&metrics);
      fAscent = metrics.fAscent;
      fDescent = metrics.fDescent;
      fLeading = metrics.fLeading;
  }

  TextMetrics(const TextMetrics&) = default;

  void merge(TextMetrics tail) {
      this->fAscent = std::min(this->fAscent, tail.fAscent);
      this->fDescent = std::max(this->fDescent, tail.fDescent);
      this->fLeading = std::max(this->fLeading, tail.fLeading);
  }

  void clean() {
      this->fAscent = 0;
      this->fDescent = 0;
      this->fLeading = 0;
  }

  SkScalar height() const {
      return this->fDescent - this->fAscent + this->fLeading;
  }

    SkScalar baseline() const {
          return - this->fAscent + this->fLeading / 2;
    }

    SkScalar above() const { return - this->fAscent + this->fLeading / 2; }
    SkScalar below() const { return this->fDescent + this->fLeading / 2; }

private:
    SkScalar fAscent;
    SkScalar fDescent;
    SkScalar fLeading;
};

class FontBlock {
public:
    FontBlock(sk_sp<FontChain> fontChain, SkScalar size, size_t length)
        : fLength(length)
        , fSize(size)
        , fFontChain(std::move(fontChain)) { }

    size_t fLength;
    sk_sp<FontChain> fFontChain;
    SkScalar fSize;

    // TODO: Features
};

class DecorBlock {
public:
    DecorBlock(const SkPaint* foreground, const SkPaint* background, size_t length)
        : fLength(length)
        , fForegroundColor(foreground)
        , fBackgroundColor(background) { }

    DecorBlock(size_t length)
        : DecorBlock(nullptr, nullptr, length) { }

    size_t fLength;
    const SkPaint* fForegroundColor;
    const SkPaint* fBackgroundColor;
    // Everything else
};

}  // namespace text
}  // namespace skia

namespace sknonstd {
template <> struct is_bitmask_enum<skia::text::CodeUnitFlags> : std::true_type {};
}

#endif
