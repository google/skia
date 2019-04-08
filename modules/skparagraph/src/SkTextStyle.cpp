/*
 * Copyright 2019 Google, Inc.
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

#include "SkTextStyle.h"
#include "SkFontStyle.h"
#include "SkColor.h"

// TODO: Make it external so the other platforms (Android) could use it
#define DEFAULT_FONT_FAMILY "Arial"

SkTextStyle::SkTextStyle()
    : fFontStyle() {

    fFontFamilies.emplace_back(DEFAULT_FONT_FAMILY);
    fColor = SK_ColorWHITE;
    fDecoration = SkTextDecoration::kNoDecoration;
    // Does not make sense to draw a transparent object, so we use it as a default
    // value to indicate no decoration color was set.
    fDecorationColor = SK_ColorTRANSPARENT;
    fDecorationStyle = SkTextDecorationStyle::kSolid;
    // Thickness is applied as a multiplier to the default thickness of the font.
    fDecorationThicknessMultiplier = 1.0;
    fFontSize = 14.0;
    fLetterSpacing = 0.0;
    fWordSpacing = 0.0;
    fFontHeight = 1.0;
    fHasBackground = false;
    fHasForeground = false;
}

bool SkTextStyle::equals(const SkTextStyle& other) const {

    if (fColor != other.fColor) {
        return false;
    }
    if (fDecoration != other.fDecoration) {
        return false;
    }
    if (fDecorationColor != other.fDecorationColor) {
        return false;
    }
    if (fDecorationStyle != other.fDecorationStyle) {
        return false;
    }
    if (fDecorationThicknessMultiplier
        != other.fDecorationThicknessMultiplier) {
            return false;
    }
    if (!(fFontStyle == other.fFontStyle)) {
        return false;
    }
    if (fFontFamilies != other.fFontFamilies) {
        return false;
    }
    if (fLetterSpacing != other.fLetterSpacing) {
        return false;
    }
    if (fWordSpacing != other.fWordSpacing) {
        return false;
    }
    if (fFontHeight != other.fFontHeight) {
        return false;
    }
    if (fLocale != other.fLocale) {
        return false;
    }
    if (fForeground != other.fForeground) {
        return false;
    }
    if (fTextShadows.size() != other.fTextShadows.size()) {
        return false;
    }

    for (int32_t i = 0; i < (int32_t)fTextShadows.size(); ++i) {
        if (fTextShadows[i] != other.fTextShadows[i]) {
            return false;
        }
    }

    return true;
}

bool SkTextStyle::matchOneAttribute(SkStyleType styleType, const SkTextStyle& other) const {

  switch (styleType) {
      case Foreground:
          if (fHasForeground) {
              return other.fHasForeground && fForeground == other.fForeground;
          } else {
            return !other.fHasForeground && fColor == other.fColor;
          }

      case Background:
        return (fHasBackground == other.fHasBackground && fBackground == other.fBackground);

      case Shadow:
          if (fTextShadows.size() != other.fTextShadows.size()) {
              return false;
          }

      for (int32_t i = 0; i < SkToInt(fTextShadows.size()); ++i) {
          if (fTextShadows[i] != other.fTextShadows[i]) {
              return false;
          }
      }
      return true;

      case Decorations:
          return fDecoration == other.fDecoration &&
              fDecorationColor == other.fDecorationColor &&
              fDecorationStyle == other.fDecorationStyle &&
              fDecorationThicknessMultiplier == other.fDecorationThicknessMultiplier;

      default:
          SkASSERT(false);
      return false;
  }
}
