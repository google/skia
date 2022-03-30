// Copyright 2019 Google LLC.
#include "include/core/SkColor.h"
#include "include/core/SkFontStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/FontArguments.h"

namespace skia {
namespace textlayout {

const std::vector<SkString> TextStyle::kDefaultFontFamilies = { SkString(DEFAULT_FONT_FAMILY) };

TextStyle::TextStyle(const TextStyle& other) {
    copyFrom(other);
}

TextStyle& TextStyle::operator=(const TextStyle& other) {
    copyFrom(other);
    return *this;
}

void TextStyle::copyFrom(const TextStyle& other) {
    fIsPlaceholder = other.fIsPlaceholder;
    fColor = other.fColor;
    fDecoration = other.fDecoration;
    fFontStyle = other.fFontStyle;
    fFontFamilies = other.fFontFamilies;
    fLetterSpacing = other.fLetterSpacing;
    fWordSpacing = other.fWordSpacing;
    fHeight = other.fHeight;
    fHeightOverride = other.fHeightOverride;
    fHalfLeading = other.fHalfLeading;
    fBaselineShift = other.fBaselineShift;
    fFontSize = other.fFontSize;
    fLocale = other.fLocale;
    fHasForeground = other.fHasForeground;
    fForeground = other.fForeground;
    fHasBackground = other.fHasBackground;
    fBackground = other.fBackground;
    fTextShadows = other.fTextShadows;
    fFontFeatures = other.fFontFeatures;
    setFontArguments(other.getFontArguments());
}

TextStyle TextStyle::cloneForPlaceholder() {
    TextStyle result;
    result.fColor = fColor;
    result.fFontSize = fFontSize;
    result.fFontFamilies = fFontFamilies;
    result.fDecoration = fDecoration;
    result.fHasBackground = fHasBackground;
    result.fHasForeground = fHasForeground;
    result.fBackground = fBackground;
    result.fForeground = fForeground;
    result.fHeightOverride = fHeightOverride;
    result.fIsPlaceholder = true;
    result.fFontFeatures = fFontFeatures;
    result.fHalfLeading = fHalfLeading;
    result.fBaselineShift = fBaselineShift;
    result.setFontArguments(getFontArguments());
    return result;
}

bool TextStyle::equals(const TextStyle& other) const {

    if (fIsPlaceholder || other.fIsPlaceholder) {
        return false;
    }

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
    if (fHeightOverride != other.fHeightOverride) {
        return false;
    }
    if (fHalfLeading != other.fHalfLeading) {
        return false;
    }
    if (fBaselineShift != other.fBaselineShift) {
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
    for (size_t i = 0; i < fTextShadows.size(); ++i) {
        if (fTextShadows[i] != other.fTextShadows[i]) {
            return false;
        }
    }
    if (fFontFeatures.size() != other.fFontFeatures.size()) {
        return false;
    }
    for (size_t i = 0; i < fFontFeatures.size(); ++i) {
        if (!(fFontFeatures[i] == other.fFontFeatures[i])) {
            return false;
        }
    }
    if (fFontArgs != other.fFontArgs) {
        return false;
    }

    return true;
}

bool TextStyle::equalsByFonts(const TextStyle& that) const {

    return !fIsPlaceholder && !that.fIsPlaceholder &&
           fFontStyle == that.fFontStyle &&
           fFontFamilies == that.fFontFamilies &&
           fFontFeatures == that.fFontFeatures &&
           fFontArgs == that.getFontArguments() &&
           nearlyEqual(fLetterSpacing, that.fLetterSpacing) &&
           nearlyEqual(fWordSpacing, that.fWordSpacing) &&
           nearlyEqual(fHeight, that.fHeight) &&
           nearlyEqual(fBaselineShift, that.fBaselineShift) &&
           nearlyEqual(fFontSize, that.fFontSize) &&
           fLocale == that.fLocale;
}

bool TextStyle::matchOneAttribute(StyleType styleType, const TextStyle& other) const {
    switch (styleType) {
        case kForeground:
            return (!fHasForeground && !other.fHasForeground && fColor == other.fColor) ||
                   ( fHasForeground &&  other.fHasForeground && fForeground == other.fForeground);

        case kBackground:
            return (!fHasBackground && !other.fHasBackground) ||
                   ( fHasBackground &&  other.fHasBackground && fBackground == other.fBackground);

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
            return fFontStyle == other.fFontStyle &&
                   fLocale == other.fLocale &&
                   fFontFamilies == other.fFontFamilies &&
                   fFontSize == other.fFontSize &&
                   fHeight == other.fHeight &&
                   fHalfLeading == other.fHalfLeading &&
                   fBaselineShift == other.fBaselineShift &&
                   fFontArgs == other.fFontArgs;
        default:
            SkASSERT(false);
            return false;
    }
}

void TextStyle::getFontMetrics(SkFontMetrics* metrics) const {
    SkFont font(fTypeface, fFontSize);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setHinting(SkFontHinting::kSlight);
    font.getMetrics(metrics);
    if (fHeightOverride) {
        auto multiplier = fHeight * fFontSize;
        auto height = metrics->fDescent - metrics->fAscent + metrics->fLeading;
        metrics->fAscent = (metrics->fAscent - metrics->fLeading / 2) * multiplier / height;
        metrics->fDescent = (metrics->fDescent + metrics->fLeading / 2) * multiplier / height;

    } else {
        metrics->fAscent = (metrics->fAscent - metrics->fLeading / 2);
        metrics->fDescent = (metrics->fDescent + metrics->fLeading / 2);
    }
    // If we shift the baseline we need to make sure the shifted text fits the line
    metrics->fAscent += fBaselineShift;
    metrics->fDescent += fBaselineShift;
}

void TextStyle::setFontArguments(const std::optional<SkFontArguments>& args) {
    if (!args) {
        fFontArgs.reset();
        return;
    }

    fArgsCoordinates.clear();
    SkFontArguments::VariationPosition argsPosition = args->getVariationDesignPosition();
    for (int i = 0; i < argsPosition.coordinateCount; ++i) {
        fArgsCoordinates.push_back(argsPosition.coordinates[i]);
    }
    SkFontArguments::VariationPosition position{
        fArgsCoordinates.empty() ? nullptr : fArgsCoordinates.data(),
        static_cast<int>(fArgsCoordinates.size())
    };

    fArgsOverrides.clear();
    SkFontArguments::Palette argsPalette = args->getPalette();
    for (int i = 0; i < argsPalette.overrideCount; ++i) {
        fArgsOverrides.push_back(argsPalette.overrides[i]);
    }
    SkFontArguments::Palette palette{
        argsPalette.index,
        fArgsOverrides.empty() ? nullptr : fArgsOverrides.data(),
        static_cast<int>(fArgsOverrides.size())
    };

    fFontArgs.emplace()
        .setCollectionIndex(args->getCollectionIndex())
        .setVariationDesignPosition(position)
        .setPalette(palette);
}

bool PlaceholderStyle::equals(const PlaceholderStyle& other) const {
    return nearlyEqual(fWidth, other.fWidth) &&
           nearlyEqual(fHeight, other.fHeight) &&
           fAlignment == other.fAlignment &&
           fBaseline == other.fBaseline &&
           (fAlignment != PlaceholderAlignment::kBaseline ||
            nearlyEqual(fBaselineOffset, other.fBaselineOffset));
}

}  // namespace textlayout
}  // namespace skia
