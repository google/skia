// Copyright 2019 Google LLC.
#ifndef TextStyle_DEFINED
#define TextStyle_DEFINED

#include <vector>
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextShadow.h"
#include "modules/skparagraph/include/StringCache.h"

// TODO: Make it external so the other platforms (Android) could use it
#define DEFAULT_FONT_FAMILY "sans-serif"

namespace skia {
namespace textlayout {

// Multiple decorations can be applied at once. Ex: Underline and overline is
// (0x1 | 0x2)
enum TextDecoration {
    kNoDecoration = 0x0,
    kUnderline = 0x1,
    kOverline = 0x2,
    kLineThrough = 0x4,
};
constexpr TextDecoration AllTextDecorations[] = {
        TextDecoration::kNoDecoration,
        TextDecoration::kUnderline,
        TextDecoration::kOverline,
        TextDecoration::kLineThrough,
};

enum class TextDecorationStyle { kSolid, kDouble, kDotted, kDashed, kWavy };

enum class StyleType {
    kAllAttributes,
    kFont,
    kForeground,
    kBackground,
    kShadow,
    kDecorations,
    kLetterSpacing,
    kWordSpacing
};

struct Decoration {
    TextDecoration fType;
    SkColor fColor;
    TextDecorationStyle fStyle;
    SkScalar fThicknessMultiplier;

    bool operator==(const Decoration& other) const {
        return this->fType == other.fType &&
               this->fColor == other.fColor &&
               this->fStyle == other.fStyle &&
               this->fThicknessMultiplier == other.fThicknessMultiplier;
    }
};

class TextStyle {
public:
    TextStyle();
    ~TextStyle() = default;

    bool equals(const TextStyle& other) const;
    bool matchOneAttribute(StyleType styleType, const TextStyle& other) const;
    bool operator==(const TextStyle& rhs) const { return this->equals(rhs); }

    SkColor getColor() const { return fColor; }
    void setColor(SkColor color) { fColor = color; }

    const SkPaint* getForeground() const { return  fHasForeground ? &fForeground : nullptr; }
    bool hasForeground() const { return fHasForeground; }
    void setForeground(const SkPaint& paint) {
        fForeground = paint;
        fHasForeground = true;
    }
    void setForegroundColor(const SkPaint& paint) { setForeground(paint); }

    const SkPaint* getBackground() const { return fHasBackground ? &fBackground : nullptr; }
    bool hasBackground() const { return fHasBackground; }
    void setBackground(const SkPaint& paint) {
        fBackground = paint;
        fHasBackground = true;
    }
    void setBackgroundColor(const SkPaint& paint) { setBackground(paint); }

    // Decorations
    Decoration getDecoration() const { return fDecoration; }
    TextDecoration getDecorationType() const { return fDecoration.fType; }
    SkColor getDecorationColor() const { return fDecoration.fColor; }
    TextDecorationStyle getDecorationStyle() const { return fDecoration.fStyle; }
    SkScalar getDecorationThicknessMultiplier() const {
        return fDecoration.fThicknessMultiplier;
    }
    void setDecoration(TextDecoration decoration) { fDecoration.fType = decoration; }
    void setDecorationStyle(TextDecorationStyle style) { fDecoration.fStyle = style; }
    void setDecorationColor(SkColor color) { fDecoration.fColor = color; }
    void setDecorationThicknessMultiplier(SkScalar m) { fDecoration.fThicknessMultiplier = m; }

    // Weight/Width/Slant
    SkFontStyle getFontStyle() const { return fFontStyle; }
    void setFontStyle(SkFontStyle fontStyle) { fFontStyle = fontStyle; }

    // Shadows
    size_t getShadowNumber() const { return fTextShadows.size(); }
    std::vector<TextShadow> getShadows() const { return fTextShadows; }
    void addShadow(TextShadow shadow) { fTextShadows.emplace_back(shadow); }
    void resetShadows() { fTextShadows.clear(); }

    SkScalar getFontSize() const { return fFontSize; }
    void setFontSize(SkScalar size) { fFontSize = size; }

    const std::vector<SkString> getFontFamilies() const;
    void setFontFamilies(const std::vector<SkString>& families);
    template <typename Fn>
    void foreachFontFamilyName(Fn&& fn) const {
        for (auto& cs : fFontFamilies) {
            if (!fn(cs.value)) {
                break;
            }
        }
    }

    void setHeight(SkScalar height) { fHeight = height; }
    SkScalar getHeight() const { return fHeight; }

    void setLetterSpacing(SkScalar letterSpacing) { fLetterSpacing = letterSpacing; }
    SkScalar getLetterSpacing() const { return fLetterSpacing; }

    void setWordSpacing(SkScalar wordSpacing) { fWordSpacing = wordSpacing; }
    SkScalar getWordSpacing() const { return fWordSpacing; }

    SkString getLocale() const { return  StringCache::gStringCache.makerSkString(fLocale); }
    void setLocale(const SkString& locale) { fLocale = StringCache::gStringCache.make(locale.c_str()); }

    TextBaseline getTextBaseline() const { return fTextBaseline; }
    void setTextBaseline(TextBaseline baseline) { fTextBaseline = baseline; }

    static SkPaint* getPaint(SkColor color);

private:
    Decoration fDecoration;

    SkFontStyle fFontStyle;

    SkTArray<CachedString> fFontFamilies;

    SkScalar fFontSize;
    SkScalar fHeight;
    CachedString fLocale;
    SkScalar fLetterSpacing;
    SkScalar fWordSpacing;

    TextBaseline fTextBaseline;

    SkColor fColor;
    bool fHasForeground;
    bool fHasBackground;
    SkPaint fBackground;
    SkPaint fForeground;

    std::vector<TextShadow> fTextShadows;
};

typedef size_t TextIndex;
typedef SkRange<size_t> TextRange;
const SkRange<size_t> EMPTY_TEXT = EMPTY_RANGE;

struct Block {
    Block() : fRange(EMPTY_RANGE), fStyle() { }
    Block(size_t start, size_t end, const TextStyle& style)
        : fRange(start, end), fStyle(style) {}
    Block(TextRange textRange, const TextStyle& style)
        : fRange(textRange), fStyle(style) {}

    TextRange fRange;
    TextStyle fStyle;
};

}  // namespace textlayout
}  // namespace skia

#endif  // TextStyle_DEFINED
