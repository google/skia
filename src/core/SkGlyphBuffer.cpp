/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/text/StrikeForGPU.h"

void SkSourceGlyphBuffer::reset() {
    fRejectedGlyphIDs.reset();
    fRejectedPositions.reset();
}

void SkDrawableGlyphBuffer::ensureSize(size_t size) {
    if (size > fMaxSize) {
        fMultiBuffer.reset(size);
        fPositions.reset(size);
        fFormats.reset(size);
        fMaxSize = size;
    }

    fInputSize = 0;
    fAcceptedSize = 0;
}

void SkDrawableGlyphBuffer::startSource(const SkZip<const SkGlyphID, const SkPoint>& source) {
    fInputSize = source.size();
    fAcceptedSize = 0;

    auto positions = source.get<1>();
    memcpy(fPositions, positions.data(), positions.size() * sizeof(SkPoint));

    // Convert from SkGlyphIDs to SkPackedGlyphIDs.
    SkGlyphVariant* packedIDCursor = fMultiBuffer.get();
    for (auto t : source) {
        *packedIDCursor++ = SkPackedGlyphID{std::get<0>(t)};
    }
    SkDEBUGCODE(fPhase = kInput);
}

void SkDrawableGlyphBuffer::startDevicePositioning(
        const SkZip<const SkGlyphID, const SkPoint>& source,
        const SkMatrix& positionMatrix,
        const SkGlyphPositionRoundingSpec& roundingSpec) {
    fInputSize = source.size();
    fAcceptedSize = 0;

    // Build up the mapping from source space to device space. Add the rounding constant
    // halfSampleFreq, so we just need to floor to get the device result.
    SkMatrix positionMatrixWithRounding = positionMatrix;
    SkPoint halfSampleFreq = roundingSpec.halfAxisSampleFreq;
    positionMatrixWithRounding.postTranslate(halfSampleFreq.x(), halfSampleFreq.y());

    auto positions = source.get<1>();
    positionMatrixWithRounding.mapPoints(fPositions, positions.data(), positions.size());

    auto floor = [](SkPoint pt) -> SkPoint {
        return {SkScalarFloorToScalar(pt.x()), SkScalarFloorToScalar(pt.y())};
    };

    for (auto [packedGlyphID, glyphID, pos]
            : SkMakeZip(fMultiBuffer.get(), source.get<0>(), fPositions.get())) {
        packedGlyphID = SkPackedGlyphID{glyphID, pos, roundingSpec.ignorePositionFieldMask};
        // Store rounded device coords back in pos.
        pos = floor(pos);
    }

    SkDEBUGCODE(fPhase = kInput);
}

SkString SkDrawableGlyphBuffer::dumpInput() const {
    SkASSERT(fPhase == kInput);

    SkString msg;
    for (auto [packedGlyphID, pos]
            : SkZip<SkGlyphVariant, SkPoint>{fInputSize, fMultiBuffer.get(), fPositions.get()}) {
        msg.appendf("%s:(%a,%a), ", packedGlyphID.packedID().shortDump().c_str(), pos.x(), pos.y());
    }
    return msg;
}

void SkDrawableGlyphBuffer::reset() {
    SkDEBUGCODE(fPhase = kReset);
    if (fMaxSize > 200) {
        fMultiBuffer.reset();
        fPositions.reset();
        fFormats.reset();
        fMaxSize = 0;
    }
    fInputSize = 0;
    fAcceptedSize = 0;
}

