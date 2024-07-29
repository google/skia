/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_SubRunControl_DEFINED
#define sktext_gpu_SubRunControl_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"

#include <tuple>

class SkFont;
class SkMatrix;
class SkPaint;
class SkReadBuffer;
class SkWriteBuffer;
struct SkPoint;

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

class SubRunControl {
public:
#if !defined(SK_DISABLE_SDF_TEXT)
    SubRunControl(bool ableToUseSDFT, bool useSDFTForSmallText, bool useSDFTForPerspectiveText,
                  SkScalar min, SkScalar max,
                  bool forcePathAA=false);

    // Produce a font, a scale factor from the nominal size to the source space size, and matrix
    // range where this font can be reused.
    std::tuple<SkFont, SkScalar, SDFTMatrixRange>
    getSDFFont(const SkFont& font, const SkMatrix& viewMatrix, const SkPoint& textLocation) const;

    bool isSDFT(SkScalar approximateDeviceTextSize, const SkPaint& paint,
                const SkMatrix& matrix) const;
    SkScalar maxSize() const { return fMaxDistanceFieldFontSize; }
#else
    SubRunControl(bool forcePathAA=false) : fForcePathAA(forcePathAA) {}
#endif
    bool isDirect(SkScalar approximateDeviceTextSize, const SkPaint& paint,
                  const SkMatrix& matrix) const;

    bool forcePathAA() const { return fForcePathAA; }

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

    // If true, glyphs drawn as paths are always anti-aliased regardless of any edge hinting.
    const bool fForcePathAA;
};

}  // namespace sktext::gpu

#endif  // sktext_SubRunControl_DEFINED
