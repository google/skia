/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkStrikeForGPU.h"

void SkDrawableGlyphBuffer::ensureSize(size_t size) {
    SkASSERT(fPhase == kReset);
    if (size > fMaxSize) {
        fMultiBuffer.reset(size);
        fPositions.reset(size);
        fMaxSize = size;
    }

    fInputSize = 0;
    fDrawableSize = 0;
    SkDEBUGCODE(fPhase = kPackedID);
}

void SkDrawableGlyphBuffer::startSource(
        const SkZip<const SkGlyphID, const SkPoint>& source, SkPoint origin) {
    SkASSERT(fPhase == kPackedID);
    fInputSize = source.size();

    // Map all the positions.
    auto positions = source.get<1>();
    SkMatrix::MakeTrans(origin.x(), origin.y()).mapPoints(
            fPositions, positions.data(), positions.size());

    // Convert from SkGlyphIDs to SkPackedGlyphIDs.
    SkGlyphVariant* packedIDCursor = fMultiBuffer;
    for (auto t : source) {
        *packedIDCursor++ = SkPackedGlyphID{std::get<0>(t)};
    }
    SkDEBUGCODE(fPhase = kInput);
}

void SkDrawableGlyphBuffer::startDevice(
        const SkZip<const SkGlyphID, const SkPoint>& source,
        SkPoint origin, const SkMatrix& viewMatrix,
        const SkGlyphPositionRoundingSpec& roundingSpec) {
    SkASSERT(fPhase == kPackedID);
    fInputSize = source.size();

    // Map the positions including subpixel position.
    auto positions = source.get<1>();
    SkMatrix matrix = viewMatrix;
    matrix.preTranslate(origin.x(), origin.y());
    SkPoint halfSampleFreq = roundingSpec.halfAxisSampleFreq;
    matrix.postTranslate(halfSampleFreq.x(), halfSampleFreq.y());
    matrix.mapPoints(fPositions, positions.data(), positions.size());

    // Mask for controlling axis alignment.
    SkIPoint mask = roundingSpec.ignorePositionMask;

    // Convert glyph ids and positions to packed glyph ids.
    SkZip<const SkGlyphID, const SkPoint> withMappedPos =
            SkMakeZip(source.get<0>(), fPositions.get());
    SkGlyphVariant* packedIDCursor = fMultiBuffer;
    for (auto t : withMappedPos) {
        SkGlyphID glyphID; SkPoint pos;
        std::tie(glyphID, pos) = t;
        SkFixed subX = SkScalarToFixed(pos.x()) & mask.x(),
                subY = SkScalarToFixed(pos.y()) & mask.y();
        *packedIDCursor++ = SkPackedGlyphID{glyphID, subX, subY};
    }
    SkDEBUGCODE(fPhase = kInput);
}

void SkDrawableGlyphBuffer::reset() {
    SkDEBUGCODE(fPhase = kReset);
    if (fMaxSize > 200) {
        fMultiBuffer.reset();
        fPositions.reset();
        fMaxSize = 0;
    }
    fInputSize = 0;
    fDrawableSize = 0;
}

