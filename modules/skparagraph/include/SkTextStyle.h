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

#pragma once

#include <string>
#include <vector>
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFont.h"
#include "SkDartTypes.h"
#include "SkTextShadow.h"

// TODO: Make it external so the other platforms (Android) could use it
#define DEFAULT_FONT_FAMILY "sans-serif"

// Multiple decorations can be applied at once. Ex: Underline and overline is
// (0x1 | 0x2)
enum SkTextDecoration {
  kNoDecoration = 0x0,
  kUnderline = 0x1,
  kOverline = 0x2,
  kLineThrough = 0x4,
};
constexpr std::initializer_list<SkTextDecoration> AllTextDecorations = {
    kNoDecoration,
    kUnderline,
    kOverline,
    kLineThrough,
};

enum SkTextDecorationStyle { kSolid, kDouble, kDotted, kDashed, kWavy };

enum SkStyleType {
  AllAttributes,
  Text,
  Font,
  Foreground,
  Background,
  Shadow,
  Decorations,
  LetterSpacing,
  WordSpacing
};

class SkTextStyle {

 public:

  SkTextStyle();
  ~SkTextStyle() = default;

  bool equals(const SkTextStyle& other) const;
  bool matchOneAttribute(SkStyleType styleType, const SkTextStyle& other) const;
  bool operator==(const SkTextStyle& rhs) const { return this->equals(rhs); }

  // Colors
  inline bool hasForeground() const { return fHasForeground; }
  inline bool hasBackground() const { return fHasBackground; }
  inline SkPaint getForeground() const { return fForeground; }
  inline SkPaint getBackground() const { return fBackground; }
  inline SkColor getColor() const { return fColor; }

  inline void setColor(SkColor color) { fColor = color; }
  void setForegroundColor(SkPaint paint) {
    fHasForeground = true;
    fForeground = std::move(paint);
  }
  void clearForegroundColor() {
    fHasForeground = false;
  }

  void setBackgroundColor(SkPaint paint) {
    fHasBackground = true;
    fBackground = std::move(paint);
  }

  void clearBackgroundColor() {
    fHasBackground = false;
  }

  // Decorations
  inline SkTextDecoration getDecoration() const { return fDecoration; }
  inline SkColor getDecorationColor() const { return fDecorationColor; }
  inline SkTextDecorationStyle getDecorationStyle() const {
    return fDecorationStyle;
  }
  inline SkScalar getDecorationThicknessMultiplier() const {
    return fDecorationThicknessMultiplier;
  }
  void setDecoration(SkTextDecoration decoration) {
    fDecoration = decoration;
  }
  void setDecorationStyle(SkTextDecorationStyle style) {
    fDecorationStyle = style;
  }
  void setDecorationColor(SkColor color) { fDecorationColor = color; }
  void setDecorationThicknessMultiplier(SkScalar m) {
    fDecorationThicknessMultiplier = m;
  }

  // Weight/Width/Slant
  inline SkFontStyle getFontStyle() const { return fFontStyle; }
  inline void setFontStyle(SkFontStyle fontStyle) { fFontStyle = fontStyle; }

  // Shadows
  inline size_t getShadowNumber() const { return fTextShadows.size(); }
  std::vector<SkTextShadow> getShadows() const { return fTextShadows; }
  void addShadow(SkTextShadow shadow) { fTextShadows.emplace_back(shadow); }
  void resetShadows() { fTextShadows.clear(); }

  void getFontMetrics(SkFontMetrics* metrics) const {
    SkFont font(fTypeface, fFontSize);
    font.getMetrics(metrics);
    metrics->fAscent = (metrics->fAscent - metrics->fLeading / 2) * (fHeight == 0 ? 1 : fHeight);
    metrics->fDescent = (metrics->fDescent + metrics->fLeading / 2) * (fHeight == 0 ? 1 : fHeight);
  }

  inline SkScalar getFontSize() const { return fFontSize; }
  inline void setFontSize(SkScalar size) { fFontSize = size; }

  inline std::string getFirstFontFamily() const { return fFontFamilies.front(); };
  inline void setFontFamily(const std::string& family) { fFontFamilies = { family }; }
  inline std::vector<std::string> getFontFamilies() const { return fFontFamilies; }
  inline void setFontFamilies(std::vector<std::string> families) { fFontFamilies = std::move(families); }

  inline void setHeight(SkScalar height) { fHeight = height; }
  inline SkScalar getHeight() const { return fHeight; }

  inline void setLetterSpacing(SkScalar letterSpacing) {
    fLetterSpacing = letterSpacing;
  }
  inline SkScalar getLetterSpacing() const { return fLetterSpacing; }

  inline void setWordSpacing(SkScalar wordSpacing) {
    fWordSpacing = wordSpacing;
  }
  inline SkScalar getWordSpacing() const { return fWordSpacing; }

  inline sk_sp<SkTypeface> getTypeface() const { return fTypeface; }
  inline void setTypeface(sk_sp<SkTypeface> typeface) { fTypeface = typeface; }

  inline std::string getLocale() const { return fLocale; }
  inline void setLocale(const std::string& locale) { fLocale = locale; }

  inline SkTextBaseline getTextBaseline() const { return fTextBaseline; }
  inline void setTextBaseline(SkTextBaseline baseline) { fTextBaseline = baseline; }

 private:
  SkTextDecoration fDecoration;
  SkColor fDecorationColor;
  SkTextDecorationStyle fDecorationStyle;
  SkScalar fDecorationThicknessMultiplier;

  SkFontStyle fFontStyle;

  std::vector<std::string> fFontFamilies;
  SkScalar fFontSize;

  SkScalar fHeight;
  std::string fLocale;
  SkScalar fLetterSpacing;
  SkScalar fWordSpacing;

  SkTextBaseline fTextBaseline;

  SkColor fColor;
  bool fHasBackground;
  SkPaint fBackground;
  bool fHasForeground;
  SkPaint fForeground;

  std::vector<SkTextShadow> fTextShadows;

  sk_sp<SkTypeface> fTypeface;
};

