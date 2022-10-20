/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSDFTControl_DEFINED
#define GrSDFTControl_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkScalar.h"

#include <tuple>

class SkMatrix;
class SkSurfaceProps;

// Two numbers fMatrixMin and fMatrixMax such that if viewMatrix.getMaxScale() is between them then
// this SDFT size can be reused.
class GrSDFTMatrixRange {
public:
    GrSDFTMatrixRange(SkScalar min, SkScalar max) : fMatrixMin{min}, fMatrixMax{max} {}
    bool matrixInRange(const SkMatrix& matrix) const;

private:
    const SkScalar fMatrixMin,
                   fMatrixMax;
};

class GrSDFTControl {
public:
    GrSDFTControl(bool ableToUseSDFT, bool useSDFTForSmallText, SkScalar min, SkScalar max);

    // Produce a font, a scale factor from the nominal size to the source space size, and matrix
    // range where this font can be reused.
    std::tuple<SkFont, SkScalar, GrSDFTMatrixRange>
    getSDFFont(const SkFont& font, const SkMatrix& viewMatrix) const;

    bool isDirect(SkScalar approximateDeviceTextSize, const SkPaint& paint) const;
    bool isSDFT(SkScalar approximateDeviceTextSize, const SkPaint& paint) const;

private:
    static SkScalar MinSDFTRange(bool useSDFTForSmallText, SkScalar min);

    // Below this size (in device space) distance field text will not be used.
    const SkScalar fMinDistanceFieldFontSize;

    // Above this size (in device space) distance field text will not be used and glyphs will
    // be rendered from outline as individual paths.
    const SkScalar fMaxDistanceFieldFontSize;

    const bool fAbleToUseSDFT;
};

#endif  // GrSDFTControl_DEFINED
