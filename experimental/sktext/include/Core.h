// Copyright 2021 Google LLC.
#ifndef SkTextShaper_DEFINED
#define SkTextShaper_DEFINED

namespace skia {
namespace text {

enum BlockType {
    kFont,
    kPlaceholder,
};

struct Placeholder {
    SkSize  dimensions;
    float   yOffsetFromBaseline;
};

struct Font {
    SkFont               font;      // note: ignores skewX, kEmbolden, ...
    std::vector<Feature> features;  // todo
};

struct Block {
    BlockType  type;
    uint32_t   charCount;
    union {
        Font        font;
        Placeholder placeholder;
    };
};

class SkTextShaperCore {
public:
    static std::unique_ptr<SkShapedText> Shape(SkSpan<uint16_t> utf16,
                                               SkSpan<Block> blocks,
                                               sk_sp<SkFontFallbacker> fallbacker,
                                               TextDirection);
};

class SkShapedText {
public:
    std::unique_ptr<SkWrappedText> wrap(float width, float height);

    bool hasProperty(size_t index, CodeUnitFlags flag) const {
        return (fCodeUnitProperties[index] & flag) == flag;
    }

    bool isHardLineBreak(size_t index) const {
        return this->hasProperty(index, CodeUnitFlags::kHardLineBreakBefore);
    }

    bool isSoftLineBreak(size_t index) const {
        return index != 0 && this->hasProperty(index, CodeUnitFlags::kSoftLineBreakBefore);
    }

    bool isWhitespaces(TextRange range) const {
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

    bool isClusterEdge(size_t index) const {
        return this->hasProperty(index, CodeUnitFlags::kGraphemeStart) ||
               this->hasProperty(index, CodeUnitFlags::kGlyphStart);
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
}

class SkWrappedText {
public:
    // Options to change/modify inter-line spacing here?
    sk_sp<SkFormattedText> format(TextAlign, TextDirection);

    // any helpers or getters make sense here?
    //
    // e.g. countLines(), etc.
}

// This is immutable and thread-shareable
//
class SkFormattedText : public SkRefCnt {
public:
    SkRect  bounds() const;

    class Visitor {
    public:
        virtual void onBeginLine(TextRange, float baselineY) = 0;
        virtual void onEndLine(TextRange, float baselineY) = 0;

        virtual void onGlyphRun(TextRange,
                                int glyphCount,
                                const uint16_t glyphs[],
                                const SkPoint  positions[],
                                const uint32_t offsets[]) = 0;

        virtual void onPlaceholder(TextRange,
                                   const SkRect& bounds);
    };
    void visit(Visitor*) const;
};

}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
