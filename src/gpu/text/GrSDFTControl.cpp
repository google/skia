/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/text/GrSDFTControl.h"

#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkGlyphRunPainter.h"

#include <tuple>

// DF sizes and thresholds for usage of the small and medium sizes. For example, above
// kSmallDFFontLimit we will use the medium size. The large size is used up until the size at
// which we switch over to drawing as paths as controlled by Control.
static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 72;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontSize = 162;
#ifdef SK_BUILD_FOR_MAC
static const int kLargeDFFontLimit = 162;
static const int kExtraLargeDFFontSize = 256;
#endif

SkScalar GrSDFTControl::MinSDFTRange(bool useSDFTForSmallText, SkScalar min) {
    if (!useSDFTForSmallText) {
        return kLargeDFFontSize;
    }
    return min;
}

GrSDFTControl::GrSDFTControl(
        bool ableToUseSDFT, bool useSDFTForSmallText, SkScalar min, SkScalar max)
        : fMinDistanceFieldFontSize{MinSDFTRange(useSDFTForSmallText, min)}
        , fMaxDistanceFieldFontSize{max}
        , fAbleToUseSDFT{ableToUseSDFT} {
    SkASSERT_RELEASE(0 < min && min <= max);
}

auto GrSDFTControl::drawingType(
        const SkFont& font, const SkPaint& paint, const SkMatrix& viewMatrix) const -> DrawingType {
    SkScalar scaledTextSize = viewMatrix.getMaxScale() * font.getSize();

    // Use SDFT if filled, no mask, and in the right size range.
    if (fAbleToUseSDFT &&
        paint.getMaskFilter() == nullptr &&
        paint.getStyle() == SkPaint::kFill_Style &&
        fMinDistanceFieldFontSize <= scaledTextSize && scaledTextSize <= fMaxDistanceFieldFontSize){
            return kSDFT;
    }

    // Choose to use path if it is likely that a direct mask will be too big.
    return scaledTextSize < SkStrikeCommon::kSkSideTooBigForAtlas ? kDirect : kPath;
}

SkScalar scaled_text_size(const SkScalar textSize, const SkMatrix& viewMatrix) {
    SkScalar scaledTextSize = textSize;

    if (viewMatrix.hasPerspective()) {
        // for perspective, we simply force to the medium size
        // TODO: compute a size based on approximate screen area
        scaledTextSize = kMediumDFFontLimit;
    } else {
        SkScalar maxScale = viewMatrix.getMaxScale();
        // if we have non-unity scale, we need to choose our base text size
        // based on the SkPaint's text size multiplied by the max scale factor
        // TODO: do we need to do this if we're scaling down (i.e. maxScale < 1)?
        if (maxScale > 0 && !SkScalarNearlyEqual(maxScale, SK_Scalar1)) {
            scaledTextSize *= maxScale;
        }
    }

    return scaledTextSize;
}

SkFont GrSDFTControl::getSDFFont(const SkFont& font,
                                 const SkMatrix& viewMatrix,
                                 SkScalar* textRatio) const {
    SkScalar textSize = font.getSize();
    SkScalar scaledTextSize = scaled_text_size(textSize, viewMatrix);

    SkFont dfFont{font};

    if (scaledTextSize <= kSmallDFFontLimit) {
        *textRatio = textSize / kSmallDFFontSize;
        dfFont.setSize(SkIntToScalar(kSmallDFFontSize));
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        *textRatio = textSize / kMediumDFFontSize;
        dfFont.setSize(SkIntToScalar(kMediumDFFontSize));
#ifdef SK_BUILD_FOR_MAC
    } else if (scaledTextSize <= kLargeDFFontLimit) {
        *textRatio = textSize / kLargeDFFontSize;
        dfFont.setSize(SkIntToScalar(kLargeDFFontSize));
    } else {
        *textRatio = textSize / kExtraLargeDFFontSize;
        dfFont.setSize(SkIntToScalar(kExtraLargeDFFontSize));
    }
#else
    } else {
        *textRatio = textSize / kLargeDFFontSize;
        dfFont.setSize(SkIntToScalar(kLargeDFFontSize));
    }
#endif

    dfFont.setEdging(SkFont::Edging::kAntiAlias);
    dfFont.setForceAutoHinting(false);
    dfFont.setHinting(SkFontHinting::kNormal);

    // The sub-pixel position will always happen when transforming to the screen.
    dfFont.setSubpixel(false);
    return dfFont;
}

std::pair<SkScalar, SkScalar> GrSDFTControl::computeSDFMinMaxScale(
        SkScalar textSize, const SkMatrix& viewMatrix) const {

    SkScalar scaledTextSize = scaled_text_size(textSize, viewMatrix);

    // We have three sizes of distance field text, and within each size 'bucket' there is a floor
    // and ceiling.  A scale outside of this range would require regenerating the distance fields
    SkScalar dfMaskScaleFloor;
    SkScalar dfMaskScaleCeil;
    if (scaledTextSize <= kSmallDFFontLimit) {
        dfMaskScaleFloor = fMinDistanceFieldFontSize;
        dfMaskScaleCeil = kSmallDFFontLimit;
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        dfMaskScaleFloor = kSmallDFFontLimit;
        dfMaskScaleCeil = kMediumDFFontLimit;
    } else {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = fMaxDistanceFieldFontSize;
    }

    // Because there can be multiple runs in the blob, we want the overall maxMinScale, and
    // minMaxScale to make regeneration decisions.  Specifically, we want the maximum minimum scale
    // we can tolerate before we'd drop to a lower mip size, and the minimum maximum scale we can
    // tolerate before we'd have to move to a large mip size.  When we actually test these values
    // we look at the delta in scale between the new viewmatrix and the old viewmatrix, and test
    // against these values to decide if we can reuse or not(ie, will a given scale change our mip
    // level)
    SkASSERT(dfMaskScaleFloor <= scaledTextSize && scaledTextSize <= dfMaskScaleCeil);

    return std::make_pair(dfMaskScaleFloor / scaledTextSize, dfMaskScaleCeil / scaledTextSize);
}


