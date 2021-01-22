/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSDFTOptions_DEFINED
#define GrSDFTOptions_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkScalar.h"

class SkMatrix;
class SkSurfaceProps;

class GrSDFTOptions {
public:
    GrSDFTOptions(SkScalar min, SkScalar max);

    bool canDrawAsDistanceFields(const SkPaint&, const SkFont&, const SkMatrix& viewMatrix,
                                 const SkSurfaceProps& props,
                                 bool contextSupportsDistanceFieldText) const;
    SkFont getSDFFont(const SkFont& font,
                      const SkMatrix& viewMatrix,
                      SkScalar* textRatio) const;
    std::pair<SkScalar, SkScalar> computeSDFMinMaxScale(
            SkScalar textSize, const SkMatrix& viewMatrix) const;
private:
    // Below this size (in device space) distance field text will not be used.
    const SkScalar fMinDistanceFieldFontSize;

    // Above this size (in device space) distance field text will not be used and glyphs will
    // be rendered from outline as individual paths.
    const SkScalar fMaxDistanceFieldFontSize;
};

#endif  // GrSDFTOptions_DEFINED
