/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/VertexFiller.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/MaskFormat.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

#include <optional>

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {

VertexFiller::VertexFiller(MaskFormat maskFormat,
                           const SkMatrix& creationMatrix,
                           SkRect creationBounds,
                           SkSpan<const SkPoint> leftTop,
                           bool canDrawDirect)
        : fMaskFormat{maskFormat}
        , fCanDrawDirect{canDrawDirect}
        , fCreationMatrix{creationMatrix}
        , fCreationBounds{creationBounds}
        , fLeftTop{leftTop} {}

VertexFiller VertexFiller::Make(MaskFormat maskType,
                                const SkMatrix &creationMatrix,
                                SkRect creationBounds,
                                SkSpan<const SkPoint> positions,
                                SubRunAllocator *alloc,
                                FillerType fillerType) {
    SkSpan<SkPoint> leftTop = alloc->makePODSpan<SkPoint>(positions);
    return VertexFiller{
            maskType, creationMatrix, creationBounds, leftTop, fillerType == kIsDirect};
}

std::optional<VertexFiller> VertexFiller::MakeFromBuffer(SkReadBuffer &buffer,
                                                         SubRunAllocator *alloc) {
    int checkingMaskType = buffer.readInt();
    if (!buffer.validate(
            0 <= checkingMaskType && checkingMaskType < skgpu::kMaskFormatCount)) {
        return std::nullopt;
    }
    MaskFormat maskType = (MaskFormat) checkingMaskType;

    const bool canDrawDirect = buffer.readBool();

    SkMatrix creationMatrix;
    buffer.readMatrix(&creationMatrix);

    SkRect creationBounds = buffer.readRect();

    SkSpan<SkPoint> leftTop = MakePointsFromBuffer(buffer, alloc);
    if (leftTop.empty()) { return std::nullopt; }

    SkASSERT(buffer.isValid());
    return VertexFiller{maskType, creationMatrix, creationBounds, leftTop, canDrawDirect};
}

void VertexFiller::flatten(SkWriteBuffer &buffer) const {
    buffer.writeInt(static_cast<int>(fMaskFormat));
    buffer.writeBool(fCanDrawDirect);
    buffer.writeMatrix(fCreationMatrix);
    buffer.writeRect(fCreationBounds);
    buffer.writePointArray(fLeftTop);
}

SkMatrix VertexFiller::viewDifference(const SkMatrix &positionMatrix) const {
    if (SkMatrix inverse; fCreationMatrix.invert(&inverse)) {
        return SkMatrix::Concat(positionMatrix, inverse);
    }
    return SkMatrix::I();
}

// Check for integer translate with the same 2x2 matrix.
// Returns the translation, and true if the change from creation matrix to the position matrix
// supports using direct glyph masks.
std::tuple<bool, SkVector> VertexFiller::CanUseDirect(
        const SkMatrix& creationMatrix, const SkMatrix& positionMatrix) {
    // The existing direct glyph info can be used if the creationMatrix, and the
    // positionMatrix have the same 2x2, the translation between them is integer, and no
    // perspective is involved. Calculate the translation in source space to a translation in
    // device space by mapping (0, 0) through both the creationMatrix and the positionMatrix;
    // take the difference.
    SkVector translation = positionMatrix.mapOrigin() - creationMatrix.mapOrigin();
    return {creationMatrix.getScaleX() == positionMatrix.getScaleX() &&
            creationMatrix.getScaleY() == positionMatrix.getScaleY() &&
            creationMatrix.getSkewX()  == positionMatrix.getSkewX()  &&
            creationMatrix.getSkewY()  == positionMatrix.getSkewY()  &&
            !positionMatrix.hasPerspective() && !creationMatrix.hasPerspective() &&
            SkScalarIsInt(translation.x()) && SkScalarIsInt(translation.y()),
            translation};
}

bool VertexFiller::isLCD() const { return fMaskFormat == MaskFormat::kA565; }

// Return true if the positionMatrix represents an integer translation. Return the device
// bounding box of all the glyphs. If the bounding box is empty, then something went singular
// and this operation should be dropped.
std::tuple<bool, SkRect> VertexFiller::deviceRectAndCheckTransform(
            const SkMatrix &positionMatrix) const {
    if (fCanDrawDirect) {
        const auto [directDrawCompatible, offset] = CanUseDirect(fCreationMatrix, positionMatrix);

        if (directDrawCompatible) {
            return {true, fCreationBounds.makeOffset(offset)};
        }
    }

    if (SkMatrix inverse; fCreationMatrix.invert(&inverse)) {
        SkMatrix viewDifference = SkMatrix::Concat(positionMatrix, inverse);
        return {false, viewDifference.mapRect(fCreationBounds)};
    }

    // initialPositionMatrix is singular. Do nothing.
    return {false, SkRect::MakeEmpty()};
}

std::tuple<SkRect, SkMatrix> VertexFiller::boundsAndDeviceMatrix(const SkMatrix& localToDevice,
                                                                 SkPoint drawOrigin) const {
    // The baked-in matrix differs from the current localToDevice by a translation if the
    // upper 2x2 remains the same, and there's no perspective. Since there's no projection,
    // Z is irrelevant, so it's okay that fCreationMatrix is an SkMatrix and has
    // discarded the 3rd row/col, and can ignore those values in localToDevice.
    const bool compatibleMatrix = localToDevice.rc(0, 0) == fCreationMatrix.rc(0, 0) &&
                                  localToDevice.rc(0, 1) == fCreationMatrix.rc(0, 1) &&
                                  localToDevice.rc(1, 0) == fCreationMatrix.rc(1, 0) &&
                                  localToDevice.rc(1, 1) == fCreationMatrix.rc(1, 1) &&
                                  !localToDevice.hasPerspective() &&
                                  !fCreationMatrix.hasPerspective();

    if (compatibleMatrix) {
        SkPoint mappedOrigin = localToDevice.mapPoint(drawOrigin);
        auto offset = mappedOrigin -
                      SkPoint{fCreationMatrix.getTranslateX(), fCreationMatrix.getTranslateY()};
        if (SkScalarIsInt(offset.fX) && SkScalarIsInt(offset.fY)) {
            // The offset is an integer (but make sure), which means the generated mask can be
            // accessed without changing how texels would be sampled.
            return {fCreationBounds, SkMatrix::Translate(offset)};
        }
    }

    // Otherwise compute the relative transformation from fCreationMatrix to
    // localToDevice, with the drawOrigin applied. If fCreationMatrix or the
    // concatenation is not invertible the returned Transform is marked invalid and the draw
    // will be automatically dropped.
    SkMatrix positionMatrix = localToDevice;
    positionMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
    const SkMatrix viewDifference = this->viewDifference(positionMatrix);
    return {fCreationBounds, viewDifference};
}

}  // namespace sktext::gpu
