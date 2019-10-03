/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphBuffer.h"

void SkPainterPassStorage::startDevice(
        SkZip<SkGlyphID, SkPoint> input, SkPoint origin, const SkMatrix& viewMatrix,
        const SkStrikeForGPU& strike) {
    fSource = input;
    size_t runSize = fSource.size();
    SkGlyphID* glyphIDs = fSource.get<0>().data();
    SkPoint* positions = fSource.get<1>().data();

    // Add rounding and origin.
    SkMatrix matrix = viewMatrix;
    matrix.preTranslate(origin.x(), origin.y());
    SkPoint rounding = strike.rounding();
    matrix.postTranslate(rounding.x(), rounding.y());
    matrix.mapPoints(fPositions, positions, runSize);

    SkIPoint mask = strike.subpixelMask();

    for (size_t i = 0; i < runSize; i++) {
        SkFixed subX = SkScalarToFixed(fPositions[i].x()) & mask.x(),
                subY = SkScalarToFixed(fPositions[i].y()) & mask.y();
        fMultiBuffer[i].packedID = SkPackedGlyphID{glyphIDs[i], subX, subY};
    }
}
