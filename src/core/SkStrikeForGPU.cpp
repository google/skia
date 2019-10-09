/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "src/core/SkStrikeForGPU.h"

#include "src/core/SkGlyphRunPainter.h"

bool SkStrikeForGPU::CanDrawAsMask(const SkGlyph& glyph) {
    return glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas;
}

bool SkStrikeForGPU::CanDrawAsSDFT(const SkGlyph& glyph, const SkPoint& position,
                                   const SkMatrix& viewMatrix, SkScalar strikeToSourceRatio) {
    if (viewMatrix.hasPerspective()) {
        // starting area
        SkScalar baseArea = glyph.width()*glyph.height();

        // transform glyphRect to device space
        SkRect glyphRect = SkRect::MakeXYWH(
                         SkIntToScalar(glyph.left())    * strikeToSourceRatio + position.x(),
                         SkIntToScalar(glyph.top())     * strikeToSourceRatio + position.y(),
                         SkIntToScalar(glyph.width())   * strikeToSourceRatio,
                         SkIntToScalar(glyph.height())  * strikeToSourceRatio);
        SkPoint quad[4];
        viewMatrix.mapRectToQuad(quad, glyphRect);

        // compute area of transformed glyph's quad
        SkPoint v0 = quad[1] - quad[0];
        SkPoint v1 = quad[2] - quad[0];
        SkScalar area = v0.cross(v1);
        SkPoint v2 = quad[3] - quad[0];
        area += v1.cross(v2);
        area = SkTAbs(0.5f*area);

        // scaling up by more than 2x the original size (or 4x the area) can cause artifacts,
        // fall back to paths in that case
        return (area/baseArea <= 4);
    } else {
        return glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas
               && glyph.maskFormat() == SkMask::kSDF_Format;
    }
}

bool SkStrikeForGPU::CanDrawAsPath(const SkGlyph& glyph) {
    SkASSERT(glyph.isColor() || glyph.setPathHasBeenCalled());
    return !glyph.isColor() && glyph.path() != nullptr;
}

