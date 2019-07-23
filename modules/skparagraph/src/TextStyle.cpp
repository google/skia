// Copyright 2019 Google LLC.
#include "include/core/SkColor.h"
#include "include/private/SkTHash.h"
#include "include/core/SkFontStyle.h"
#include "modules/skparagraph/include/StringCache.h"
#include "modules/skparagraph/include/TextStyle.h"


namespace {
    SkTHashMap<SkColor, SkPaint> fSimpleColorPaints;
}

namespace skia {
namespace textlayout {

TextStyle::TextStyle() : fFontStyle(), fFontFamilies(1), fLocale() {

    fFontFamilies.emplace_back(StringCache::gStringCache.make(DEFAULT_FONT_FAMILY));
    fLocale = StringCache::gStringCache.make("");

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
    fHasForeground = false;
    fHasBackground = false;
    fTextBaseline = TextBaseline::kAlphabetic;
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
    if (fHasForeground != other.fHasForeground) {
        return false;
    } else if (fHasForeground &&  fForeground != other.fForeground) {
        return false;
    }

    if (fHasBackground != other.fHasBackground) {
        return false;
    } else if (fHasBackground && fBackground != other.fBackground) {
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
            if (fHasBackground) {
                return other.fHasBackground && fBackground == other.fBackground;
            } else {
                return !other.fHasBackground;
            }


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
            return fFontStyle == other.fFontStyle &&
                   fFontSize == other.fFontSize &&
                   fHeight == other.fHeight &&
                   fFontFamilies == other.fFontFamilies;

        default:
            SkASSERT(false);
            return false;
    }
}

const std::vector<SkString> TextStyle::getFontFamilies() const {

    std::vector<SkString> result;
    result.reserve(fFontFamilies.size());
    for (auto& cs : fFontFamilies) {
        result.emplace_back(StringCache::gStringCache.makerSkString(cs));
    }
    return result;
}

void TextStyle::setFontFamilies(const std::vector<SkString>& families) {
    fFontFamilies.reset();
    fFontFamilies.reserve(families.size());
    for (auto& family : families) {
        fFontFamilies.emplace_back(StringCache::gStringCache.make(family.c_str()));
    }
}

SkPaint* TextStyle::getPaint(SkColor color) {
    auto found = fSimpleColorPaints.find(color);
    if (found == nullptr) {
        SkPaint paint;
        paint.setColor(color);
        found = fSimpleColorPaints.set(color, paint);
    }
    return found;
}

}  // namespace textlayout
}  // namespace skia
