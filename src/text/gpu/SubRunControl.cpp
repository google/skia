/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/SubRunControl.h"

#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <tuple>

struct SkPoint;

namespace sktext::gpu {

#if !defined(SK_DISABLE_SDF_TEXT)
// DF sizes and thresholds for usage of the small and medium sizes. For example, above
// kSmallDFFontLimit we will use the medium size. The large size is used up until the size at
// which we switch over to drawing as paths as controlled by Control.
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontLimit = 162;
#ifdef SK_BUILD_FOR_MAC
static const int kExtraLargeDFFontLimit = 256;
#endif

SkScalar SubRunControl::MinSDFTRange(bool useSDFTForSmallText, SkScalar min) {
    if (!useSDFTForSmallText) {
        return kLargeDFFontLimit;
    }
    return min;
}

SubRunControl::SubRunControl(
        bool ableToUseSDFT, bool useSDFTForSmallText, bool useSDFTForPerspectiveText,
        SkScalar min, SkScalar max,
        bool forcePathAA)
        : fMinDistanceFieldFontSize{MinSDFTRange(useSDFTForSmallText, min)}
        , fMaxDistanceFieldFontSize{max}
        , fAbleToUseSDFT{ableToUseSDFT}
        , fAbleToUsePerspectiveSDFT{useSDFTForPerspectiveText}
        , fForcePathAA{forcePathAA} {
    SkASSERT_RELEASE(0 < min && min <= max);
}
#endif // !defined(SK_DISABLE_SDF_TEXT)

bool SubRunControl::isDirect(SkScalar approximateDeviceTextSize, const SkPaint& paint,
                           const SkMatrix& matrix) const {
#if !defined(SK_DISABLE_SDF_TEXT)
    const bool isSDFT = this->isSDFT(approximateDeviceTextSize, paint, matrix);
#else
    const bool isSDFT = false;
#endif
    return !isSDFT &&
           !matrix.hasPerspective() &&
            0 < approximateDeviceTextSize &&
            approximateDeviceTextSize < SkGlyphDigest::kSkSideTooBigForAtlas;
}

#if !defined(SK_DISABLE_SDF_TEXT)
bool SubRunControl::isSDFT(SkScalar approximateDeviceTextSize, const SkPaint& paint,
                           const SkMatrix& matrix) const {
    const bool wideStroke = paint.getStyle() == SkPaint::kStroke_Style &&
            paint.getStrokeWidth() > 0;
    return fAbleToUseSDFT &&
           paint.getMaskFilter() == nullptr &&
            (paint.getStyle() == SkPaint::kFill_Style || wideStroke) &&
           0 < approximateDeviceTextSize &&
           (fAbleToUsePerspectiveSDFT || !matrix.hasPerspective()) &&
           (fMinDistanceFieldFontSize <= approximateDeviceTextSize || matrix.hasPerspective()) &&
           approximateDeviceTextSize <= fMaxDistanceFieldFontSize;
}

std::tuple<SkFont, SkScalar, SDFTMatrixRange>
SubRunControl::getSDFFont(const SkFont& font, const SkMatrix& viewMatrix,
                        const SkPoint& textLoc) const {
    SkScalar textSize = font.getSize();
    SkScalar scaledTextSize = SkFontPriv::ApproximateTransformedTextSize(font, viewMatrix, textLoc);
    if (scaledTextSize <= 0 || SkScalarNearlyEqual(textSize, scaledTextSize)) {
        scaledTextSize = textSize;
    }

    SkFont dfFont{font};

    SkScalar dfMaskScaleFloor;
    SkScalar dfMaskScaleCeil;
    SkScalar dfMaskSize;
    if (scaledTextSize <= kSmallDFFontLimit) {
        dfMaskScaleFloor = fMinDistanceFieldFontSize;
        dfMaskScaleCeil = kSmallDFFontLimit;
        dfMaskSize = kSmallDFFontLimit;
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        dfMaskScaleFloor = kSmallDFFontLimit;
        dfMaskScaleCeil = kMediumDFFontLimit;
        dfMaskSize = kMediumDFFontLimit;
#ifdef SK_BUILD_FOR_MAC
    } else if (scaledTextSize <= kLargeDFFontLimit) {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = kLargeDFFontLimit;
        dfMaskSize = kLargeDFFontLimit;
    } else {
        dfMaskScaleFloor = kLargeDFFontLimit;
        dfMaskScaleCeil = fMaxDistanceFieldFontSize;
        dfMaskSize = kExtraLargeDFFontLimit;
    }
#else
    } else {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = fMaxDistanceFieldFontSize;
        dfMaskSize = kLargeDFFontLimit;
    }
#endif

    dfFont.setSize(dfMaskSize);
    dfFont.setEdging(SkFont::Edging::kAntiAlias);
    dfFont.setForceAutoHinting(false);
    dfFont.setHinting(SkFontHinting::kNormal);

    // The sub-pixel position will always happen when transforming to the screen.
    dfFont.setSubpixel(false);

    SkScalar minMatrixScale = dfMaskScaleFloor / textSize,
             maxMatrixScale = dfMaskScaleCeil  / textSize;
    return {dfFont, textSize / dfMaskSize, {minMatrixScale, maxMatrixScale}};
}

bool SDFTMatrixRange::matrixInRange(const SkMatrix& matrix) const {
    SkScalar maxScale = matrix.getMaxScale();
    return fMatrixMin < maxScale && maxScale <= fMatrixMax;
}

void SDFTMatrixRange::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fMatrixMin);
    buffer.writeScalar(fMatrixMax);
}

SDFTMatrixRange SDFTMatrixRange::MakeFromBuffer(SkReadBuffer& buffer) {
    SkScalar min = buffer.readScalar();
    SkScalar max = buffer.readScalar();
    return SDFTMatrixRange{min, max};
}
#endif // !defined(SK_DISABLE_SDF_TEXT)

}  // namespace sktext::gpu
