// Copyright 2019 Google LLC.
#ifndef TextStyle_DEFINED
#define TextStyle_DEFINED

#include <vector>
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextShadow.h"

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
        kNoDecoration,
        kUnderline,
        kOverline,
        kLineThrough,
};

enum TextDecorationStyle { kSolid, kDouble, kDotted, kDashed, kWavy };

enum StyleType {
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

/// Where to vertically align the placeholder relative to the surrounding text.
enum class PlaceholderAlignment {
  /// Match the baseline of the placeholder with the baseline.
  kBaseline,

  /// Align the bottom edge of the placeholder with the baseline such that the
  /// placeholder sits on top of the baseline.
  kAboveBaseline,

  /// Align the top edge of the placeholder with the baseline specified in
  /// such that the placeholder hangs below the baseline.
  kBelowBaseline,

  /// Align the top edge of the placeholder with the top edge of the font.
  /// When the placeholder is very tall, the extra space will hang from
  /// the top and extend through the bottom of the line.
  kTop,

  /// Align the bottom edge of the placeholder with the top edge of the font.
  /// When the placeholder is very tall, the extra space will rise from
  /// the bottom and extend through the top of the line.
  kBottom,

  /// Align the middle of the placeholder with the middle of the text. When the
  /// placeholder is very tall, the extra space will grow equally from
  /// the top and bottom of the line.
  kMiddle,
};

struct PlaceholderStyle {
    PlaceholderStyle() { }
    PlaceholderStyle(SkScalar width, SkScalar height, PlaceholderAlignment alignment,
                     TextBaseline baseline, SkScalar offset)
            : fWidth(width)
            , fHeight(height)
            , fAlignment(alignment)
            , fBaseline(baseline)
            , fBaselineOffset(offset) {}

    SkScalar fWidth = 0;
    SkScalar fHeight = 0;

    PlaceholderAlignment fAlignment;

    TextBaseline fBaseline;

    // Distance from the top edge of the rect to the baseline position. This
    // baseline will be aligned against the alphabetic baseline of the surrounding
    // text.
    //
    // Positive values drop the baseline lower (positions the rect higher) and
    // small or negative values will cause the rect to be positioned underneath
    // the line. When baseline == height, the bottom edge of the rect will rest on
    // the alphabetic baseline.
    SkScalar fBaselineOffset = 0;
};

class TextStyle {
public:
    TextStyle();
    TextStyle(const TextStyle& other, bool placeholder);
    ~TextStyle() = default;

    bool equals(const TextStyle& other) const;
    bool matchOneAttribute(StyleType styleType, const TextStyle& other) const;
    bool operator==(const TextStyle& rhs) const { return this->equals(rhs); }

    // Colors
    SkColor getColor() const { return fColor; }
    void setColor(SkColor color) { fColor = color; }

    bool hasForeground() const { return fHasForeground; }
    SkPaint getForeground() const { return fForeground; }
    void setForegroundColor(SkPaint paint) {
        fHasForeground = true;
        fForeground = std::move(paint);
    }
    void clearForegroundColor() { fHasForeground = false; }

    bool hasBackground() const { return fHasBackground; }
    SkPaint getBackground() const { return fBackground; }
    void setBackgroundColor(SkPaint paint) {
        fHasBackground = true;
        fBackground = std::move(paint);
    }
    void clearBackgroundColor() { fHasBackground = false; }

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

    const std::vector<SkString>& getFontFamilies() const { return fFontFamilies; }
    void setFontFamilies(std::vector<SkString> families) {
        fFontFamilies = std::move(families);
    }

    void setHeight(SkScalar height) { fHeight = height; }
    SkScalar getHeight() const { return fHeightOverride ? fHeight : 0; }

    void setHeightOverride(bool heightOverride) { fHeightOverride = heightOverride; }
    bool getHeightOverride() const { return fHeightOverride; }

    void setLetterSpacing(SkScalar letterSpacing) { fLetterSpacing = letterSpacing; }
    SkScalar getLetterSpacing() const { return fLetterSpacing; }

    void setWordSpacing(SkScalar wordSpacing) { fWordSpacing = wordSpacing; }
    SkScalar getWordSpacing() const { return fWordSpacing; }

    SkTypeface* getTypeface() const { return fTypeface.get(); }
    sk_sp<SkTypeface> refTypeface() const { return fTypeface; }
    void setTypeface(sk_sp<SkTypeface> typeface) { fTypeface = std::move(typeface); }

    SkString getLocale() const { return fLocale; }
    void setLocale(const SkString& locale) { fLocale = locale; }

    TextBaseline getTextBaseline() const { return fTextBaseline; }
    void setTextBaseline(TextBaseline baseline) { fTextBaseline = baseline; }

    void getFontMetrics(SkFontMetrics* metrics) const;

    bool isPlaceholder() const { return fIsPlaceholder; }
    void setPlaceholder() { fIsPlaceholder = true; }

private:

    Decoration fDecoration;

    SkFontStyle fFontStyle;

    std::vector<SkString> fFontFamilies;
    SkScalar fFontSize;
    SkScalar fHeight;
    bool fHeightOverride;
    SkString fLocale;
    SkScalar fLetterSpacing;
    SkScalar fWordSpacing;

    TextBaseline fTextBaseline;

    SkColor fColor;
    bool fHasBackground;
    SkPaint fBackground;
    bool fHasForeground;
    SkPaint fForeground;

    std::vector<TextShadow> fTextShadows;

    sk_sp<SkTypeface> fTypeface;
    bool fIsPlaceholder;
};

typedef size_t TextIndex;
typedef SkRange<size_t> TextRange;
const SkRange<size_t> EMPTY_TEXT = EMPTY_RANGE;


struct Block {
    Block() : fRange(EMPTY_RANGE), fStyle() { }
    Block(size_t start, size_t end, const TextStyle& style) : fRange(start, end), fStyle(style) {}
    Block(TextRange textRange, const TextStyle& style) : fRange(textRange), fStyle(style) {}

    Block(const Block& other) {
        fRange = other.fRange;
        fStyle = other.fStyle;
    }

    void add(TextRange tail) {
        SkASSERT(fRange.end == tail.start);
        fRange = TextRange(fRange.start, fRange.start + fRange.width() + tail.width());
    }
    TextRange fRange;
    TextStyle fStyle;
};


typedef size_t BlockIndex;
typedef SkRange<size_t> BlockRange;
const size_t EMPTY_BLOCK = EMPTY_INDEX;
const SkRange<size_t> EMPTY_BLOCKS = EMPTY_RANGE;

struct Placeholder {
    Placeholder() : fRange(EMPTY_RANGE), fStyle() {}

    Placeholder(size_t start, size_t end, const PlaceholderStyle& style, BlockRange blocksBefore,
                TextRange textBefore)
            : fRange(start, end)
            , fStyle(style)
            , fBlocksBefore(blocksBefore)
            , fTextBefore(textBefore) {}

    Placeholder(const Placeholder& other) {
        fRange = other.fRange;
        fStyle = other.fStyle;
        fBlocksBefore = other.fBlocksBefore;
        fTextBefore = other.fTextBefore;
    }

    TextRange fRange;
    PlaceholderStyle fStyle;
    BlockRange fBlocksBefore;
    TextRange fTextBefore;
};

}  // namespace textlayout
}  // namespace skia

#endif  // TextStyle_DEFINED
