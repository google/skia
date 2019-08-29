/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkStrikeForGPU.h"

void SkDrawableGlyphBuffer::startSource(
        const SkZip<const SkGlyphID, const SkPoint>& source, SkPoint origin) {
    // Properly size the buffers.
    this->reset(source.size());

    // Map all the positions.
    auto positions = source.get<1>();
    SkMatrix::MakeTrans(origin.x(), origin.y()).mapPoints(
            fPositions, positions.data(), positions.size());

    // Convert from SkGlyphIDs to SkPackedGlyphIDs.
    SkBag* packedIDCursor = fMultiBuffer;
    for (auto t : source) {
        *packedIDCursor++ = SkPackedGlyphID{std::get<0>(t)};
    }
}

void SkDrawableGlyphBuffer::startDevice(
        const SkZip<const SkGlyphID, const SkPoint>& source,
        SkPoint origin, const SkMatrix& viewMatrix, const SkStrikeForGPU& strike) {
    // Properly size the buffers.
    this->reset(source.size());

    // Map the positions including subpixel position.
    auto positions = source.get<1>();
    SkMatrix matrix = viewMatrix;
    matrix.preTranslate(origin.x(), origin.y());
    SkPoint rounding = strike.rounding();
    matrix.postTranslate(rounding.x(), rounding.y());
    matrix.mapPoints(fPositions, positions.data(), positions.size());

    // Mask for controlling axis alignment.
    SkIPoint mask = strike.subpixelMask();

    // Convert glyph ids and positions to packed glyph ids.
    SkBag* packedIDCursor = fMultiBuffer;
    for (auto t : source) {
        SkGlyphID glyphID; SkPoint pos;
        std::tie(glyphID, pos) = t;
        SkFixed subX = SkScalarToFixed(pos.x()) & mask.x(),
                subY = SkScalarToFixed(pos.y()) & mask.y();
        *packedIDCursor++ = SkPackedGlyphID{glyphID, subX, subY};
    }
}
