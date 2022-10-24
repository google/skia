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
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyphRunPainter.h"

#include <tuple>

// DF sizes and thresholds for usage of the small and medium sizes. For example, above
// kSmallDFFontLimit we will use the medium size. The large size is used up until the size at
// which we switch over to drawing as paths as controlled by Control.
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontLimit = 162;
#ifdef SK_BUILD_FOR_MAC
static const int kExtraLargeDFFontLimit = 256;
#endif

SkScalar GrSDFTControl::MinSDFTRange(bool useSDFTForSmallText, SkScalar min) {
    if (!useSDFTForSmallText) {
        return kLargeDFFontLimit;
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

bool GrSDFTControl::isDirect(SkScalar approximateDeviceTextSize, const SkPaint& paint,
                             const SkMatrix& matrix) const {
    return !isSDFT(approximateDeviceTextSize, paint, matrix) &&
           !matrix.hasPerspective() &&
            0 < approximateDeviceTextSize &&
            approximateDeviceTextSize < SkStrikeCommon::kSkSideTooBigForAtlas;
}

bool GrSDFTControl::isSDFT(SkScalar approximateDeviceTextSize, const SkPaint& paint,
                           const SkMatrix& matrix) const {
    return fAbleToUseSDFT &&
           paint.getMaskFilter() == nullptr &&
           paint.getStyle() == SkPaint::kFill_Style &&
           0 < approximateDeviceTextSize &&
           (fMinDistanceFieldFontSize <= approximateDeviceTextSize ||
            matrix.hasPerspective()) &&
           approximateDeviceTextSize <= fMaxDistanceFieldFontSize;
}

std::tuple<SkFont, SkScalar, GrSDFTMatrixRange>
GrSDFTControl::getSDFFont(const SkFont& font, const SkMatrix& viewMatrix,
                          const SkPoint& textLoc) const {
    SkScalar textSize = font.getSize();
    SkScalar scaledTextSize = SkFontPriv::ApproximateTransformedTextSize(font, viewMatrix, textLoc);
    if (scaledTextSize <= 0 || SkScalarNearlyEqual(textSize, scaledTextSize)) {
        scaledTextSize = textSize;
    }

    SkFont dfFont{font};

    SkScalar dfMaskScaleFloor;
    SkScalar dfMaskScaleCeil;
    if (scaledTextSize <= kSmallDFFontLimit) {
        dfMaskScaleFloor = fMinDistanceFieldFontSize;
        dfMaskScaleCeil = kSmallDFFontLimit;
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        dfMaskScaleFloor = kSmallDFFontLimit;
        dfMaskScaleCeil = kMediumDFFontLimit;
#ifdef SK_BUILD_FOR_MAC
    } else if (scaledTextSize <= kLargeDFFontLimit) {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = kLargeDFFontLimit;
    } else {
        dfMaskScaleFloor = kLargeDFFontLimit;
        dfMaskScaleCeil = kExtraLargeDFFontLimit;
    }
#else
    } else {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = kLargeDFFontLimit;
    }
#endif

    dfFont.setSize(SkIntToScalar(dfMaskScaleCeil));
    dfFont.setEdging(SkFont::Edging::kAntiAlias);
    dfFont.setForceAutoHinting(false);
    dfFont.setHinting(SkFontHinting::kNormal);

    // The sub-pixel position will always happen when transforming to the screen.
    dfFont.setSubpixel(false);

    SkScalar minMatrixScale = dfMaskScaleFloor / textSize,
             maxMatrixScale = dfMaskScaleCeil  / textSize;
    return {dfFont, textSize / dfMaskScaleCeil, {minMatrixScale, maxMatrixScale}};
}

bool GrSDFTMatrixRange::matrixInRange(const SkMatrix& matrix) const {
    SkScalar maxScale = matrix.getMaxScale();
    return fMatrixMin < maxScale && maxScale <= fMatrixMax;
}
