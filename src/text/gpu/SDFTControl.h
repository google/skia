/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_SDFTControl_DEFINED
#define sktext_gpu_SDFTControl_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkFont.h"
#include "include/core/SkScalar.h"

#include <tuple>

class SkMatrix;
class SkSurfaceProps;

namespace sktext::gpu {

#if !defined(SK_DISABLE_SDF_TEXT)
// Two numbers fMatrixMin and fMatrixMax such that if viewMatrix.getMaxScale() is between them then
// this SDFT size can be reused.
class SDFTMatrixRange {
public:
    SDFTMatrixRange(SkScalar min, SkScalar max) : fMatrixMin{min}, fMatrixMax{max} {}
    bool matrixInRange(const SkMatrix& matrix) const;
    void flatten(SkWriteBuffer& buffer) const;
    static SDFTMatrixRange MakeFromBuffer(SkReadBuffer& buffer);

private:
    const SkScalar fMatrixMin,
                   fMatrixMax;
};
#endif

class SDFTControl {
public:
#if !defined(SK_DISABLE_SDF_TEXT)
    SDFTControl(bool ableToUseSDFT, bool useSDFTForSmallText, bool useSDFTForPerspectiveText,
                SkScalar min, SkScalar max);

    // Produce a font, a scale factor from the nominal size to the source space size, and matrix
    // range where this font can be reused.
    std::tuple<SkFont, SkScalar, SDFTMatrixRange>
    getSDFFont(const SkFont& font, const SkMatrix& viewMatrix, const SkPoint& textLocation) const;

    bool isSDFT(SkScalar approximateDeviceTextSize, const SkPaint& paint,
                const SkMatrix& matrix) const;
    SkScalar maxSize() const { return fMaxDistanceFieldFontSize; }
#else
    SDFTControl() {}
#endif
    bool isDirect(SkScalar approximateDeviceTextSize, const SkPaint& paint,
                  const SkMatrix& matrix) const;


private:
#if !defined(SK_DISABLE_SDF_TEXT)
    static SkScalar MinSDFTRange(bool useSDFTForSmallText, SkScalar min);

    // Below this size (in device space) distance field text will not be used.
    const SkScalar fMinDistanceFieldFontSize;

    // Above this size (in device space) distance field text will not be used and glyphs will
    // be rendered from outline as individual paths.
    const SkScalar fMaxDistanceFieldFontSize;

    const bool fAbleToUseSDFT;
    const bool fAbleToUsePerspectiveSDFT;
#endif
};

}  // namespace sktext::gpu

#endif  // sktext_SDFTControl_DEFINED
