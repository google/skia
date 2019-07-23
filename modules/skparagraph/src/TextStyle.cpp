// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/TextStyle.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontStyle.h"

namespace skia {
namespace textlayout {

TextStyle::TextStyle() : fFontStyle() {
    fFontFamilies.reserve(1);
    fFontFamilies.emplace_back(DEFAULT_FONT_FAMILY);
    fColor = SK_ColorWHITE;
    fDecoration.fType = TextDecoration::kNoDecoration;
    // Does not make sense to draw a transparent object, so we use it as a default
    // value to indicate no decoration color was set.
    fDecoration.fColor = SK_ColorTRANSPARENT;
    fDecoration.fStyle = TextDecorationStyle::kSolid;
    // Thickness is applied as a multiplier to the default thickness of the font.
    fDecoration.fThicknessMultiplier = 1.0;
    fFontSize = 14.0;
    fLetterSpacing = 0.0;
    fWordSpacing = 0.0;
    fHeight = 1.0;
    fHasBackground = false;
    fHasForeground = false;
    fTextBaseline = TextBaseline::kAlphabetic;
    fLocale = "";
}

bool TextStyle::equals(const TextStyle& other) const {
    if (fColor != other.fColor) {
        return false;
    }
    if (!(fDecoration == other.fDecoration)) {
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
    if (fHeight != other.fHeight) {
        return false;
    }
    if (fFontSize != other.fFontSize) {
        return false;
    }
    if (fLocale != other.fLocale) {
        return false;
    }
    if (fHasForeground != other.fHasForeground || fForeground != other.fForeground) {
        return false;
    }
    if (fHasBackground != other.fHasBackground || fBackground != other.fBackground) {
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

bool TextStyle::matchOneAttribute(StyleType styleType, const TextStyle& other) const {
    switch (styleType) {
        case kForeground:
            if (fHasForeground) {
                return other.fHasForeground && fForeground == other.fForeground;
            } else {
                return !other.fHasForeground && fColor == other.fColor;
            }

        case kBackground:
            return (fHasBackground == other.fHasBackground && fBackground == other.fBackground);

        case kShadow:
            if (fTextShadows.size() != other.fTextShadows.size()) {
                return false;
            }

            for (int32_t i = 0; i < SkToInt(fTextShadows.size()); ++i) {
                if (fTextShadows[i] != other.fTextShadows[i]) {
                    return false;
                }
            }
            return true;

        case kDecorations:
            return this->fDecoration == other.fDecoration;

        case kLetterSpacing:
            return fLetterSpacing == other.fLetterSpacing;

        case kWordSpacing:
            return fWordSpacing == other.fWordSpacing;

        case kAllAttributes:
            return this->equals(other);

        case kFont:
            // TODO: should not we take typefaces in account?
            return fFontStyle == other.fFontStyle && fFontFamilies == other.fFontFamilies &&
                   fFontSize == other.fFontSize && fHeight == other.fHeight;

        default:
            SkASSERT(false);
            return false;
    }
}

}  // namespace textlayout
}  // namespace skia
