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

class SkMatrix;
class SkSurfaceProps;

class GrSDFTControl {
public:
    GrSDFTControl(bool ableToUseSDFT, bool useSDFTForSmallText, SkScalar min, SkScalar max);

    SkFont getSDFFont(const SkFont& font,
                      const SkMatrix& viewMatrix,
                      SkScalar* textRatio) const;
    std::pair<SkScalar, SkScalar> computeSDFMinMaxScale(
            SkScalar textSize, const SkMatrix& viewMatrix) const;

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
