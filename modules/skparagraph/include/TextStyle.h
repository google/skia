/*
 * Copyright 2019 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TextStyle_DEFINED
#define TextStyle_DEFINED

#include <string>
#include <vector>
#include "DartTypes.h"
#include "TextShadow.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"

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
constexpr std::initializer_list<TextDecoration> AllTextDecorations = {
        kNoDecoration,
        kUnderline,
        kOverline,
        kLineThrough,
};

enum TextDecorationStyle { kSolid, kDouble, kDotted, kDashed, kWavy };

enum StyleType {
    kAllAttributes,
    kText,
    kFont,
    kForeground,
    kBackground,
    kShadow,
    kDecorations,
    kLetterSpacing,
    kWordSpacing
};

class TextStyle {
public:
    TextStyle();
    ~TextStyle() = default;

    bool equals(const TextStyle& other) const;
    bool matchOneAttribute(StyleType styleType, const TextStyle& other) const;
    bool operator==(const TextStyle& rhs) const { return this->equals(rhs); }

    // Colors
    bool hasForeground() const { return fHasForeground; }
    bool hasBackground() const { return fHasBackground; }
    SkPaint getForeground() const { return fForeground; }
    SkPaint getBackground() const { return fBackground; }
    SkColor getColor() const { return fColor; }

    void setColor(SkColor color) { fColor = color; }
    void setForegroundColor(SkPaint paint) {
        fHasForeground = true;
        fForeground = std::move(paint);
    }
    void clearForegroundColor() { fHasForeground = false; }

    void setBackgroundColor(SkPaint paint) {
        fHasBackground = true;
        fBackground = std::move(paint);
    }

    void clearBackgroundColor() { fHasBackground = false; }

    // Decorations
    TextDecoration getDecoration() const { return fDecoration; }
    SkColor getDecorationColor() const { return fDecorationColor; }
    TextDecorationStyle getDecorationStyle() const { return fDecorationStyle; }
    SkScalar getDecorationThicknessMultiplier() const {
        return fDecorationThicknessMultiplier;
    }
    void setDecoration(TextDecoration decoration) { fDecoration = decoration; }
    void setDecorationStyle(TextDecorationStyle style) { fDecorationStyle = style; }
    void setDecorationColor(SkColor color) { fDecorationColor = color; }
    void setDecorationThicknessMultiplier(SkScalar m) { fDecorationThicknessMultiplier = m; }

    // Weight/Width/Slant
    SkFontStyle getFontStyle() const { return fFontStyle; }
    void setFontStyle(SkFontStyle fontStyle) { fFontStyle = fontStyle; }

    // Shadows
    size_t getShadowNumber() const { return fTextShadows.size(); }
    std::vector<TextShadow> getShadows() const { return fTextShadows; }
    void addShadow(TextShadow shadow) { fTextShadows.emplace_back(shadow); }
    void resetShadows() { fTextShadows.clear(); }

    void getFontMetrics(SkFontMetrics* metrics) const {
        SkFont font(fTypeface, fFontSize);
        font.getMetrics(metrics);
        metrics->fAscent =
                (metrics->fAscent - metrics->fLeading / 2) * (fHeight == 0 ? 1 : fHeight);
        metrics->fDescent =
                (metrics->fDescent + metrics->fLeading / 2) * (fHeight == 0 ? 1 : fHeight);
    }

    SkScalar getFontSize() const { return fFontSize; }
    void setFontSize(SkScalar size) { fFontSize = size; }

    std::string getFirstFontFamily() const { return fFontFamilies.front(); }
    void setFontFamily(const std::string& family) { fFontFamilies = {family}; }
    std::vector<std::string> getFontFamilies() const { return fFontFamilies; }
    void setFontFamilies(std::vector<std::string> families) {
        fFontFamilies = std::move(families);
    }

    void setHeight(SkScalar height) { fHeight = height; }
    SkScalar getHeight() const { return fHeight; }

    void setLetterSpacing(SkScalar letterSpacing) { fLetterSpacing = letterSpacing; }
    SkScalar getLetterSpacing() const { return fLetterSpacing; }

    void setWordSpacing(SkScalar wordSpacing) { fWordSpacing = wordSpacing; }
    SkScalar getWordSpacing() const { return fWordSpacing; }

    sk_sp<SkTypeface> getTypeface() const { return fTypeface; }
    void setTypeface(sk_sp<SkTypeface> typeface) { fTypeface = typeface; }

    std::string getLocale() const { return fLocale; }
    void setLocale(const std::string& locale) { fLocale = locale; }

    TextBaseline getTextBaseline() const { return fTextBaseline; }
    void setTextBaseline(TextBaseline baseline) { fTextBaseline = baseline; }

private:
    TextDecoration fDecoration;
    SkColor fDecorationColor;
    TextDecorationStyle fDecorationStyle;
    SkScalar fDecorationThicknessMultiplier;

    SkFontStyle fFontStyle;

    std::vector<std::string> fFontFamilies;
    SkScalar fFontSize;

    SkScalar fHeight;
    std::string fLocale;
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
};
}  // namespace textlayout
}  // namespace skia

#endif  // TextStyle_DEFINED