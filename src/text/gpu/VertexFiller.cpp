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

#include <cmath>
#include <optional>

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {

bool VertexFiller::ValidDirectPosition(SkPoint pt) {
    // Single-precision floats have a 24-bit significand. At or below 2^21, floats can represent
    // all 1/4 subpixel increments (ULP <= 0.25f) and exact integers. Bounding direct coordinates
    // and offsets to 2^21 ensures that glyph bounds and sums remain well within the 2^24 exact
    // integer range, prevents subpixel rounding absorption (e.g. pt + 0.5f == pt), and avoids
    // 32-bit integer overflow.
    return pt.isFinite() &&
           std::abs(pt.x()) <= kMaxDirectCoord && std::abs(pt.y()) <= kMaxDirectCoord &&
           SkScalarIsInt(pt.x()) && SkScalarIsInt(pt.y());
}

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
#if defined(SK_DEBUG)
    if (fillerType == kIsDirect) {
        for (const SkPoint& pt : leftTop) {
            SkASSERT(ValidDirectPosition(pt));
        }
    }
#endif
    return VertexFiller{
            maskType, creationMatrix, creationBounds, leftTop, fillerType == kIsDirect};
}

SkMatrix VertexFiller::viewDifference(const SkMatrix &positionMatrix) const {
    if (SkMatrix inverse; fCreationMatrix.invert(&inverse)) {
        return SkMatrix::Concat(positionMatrix, inverse);
    }
    return SkMatrix::I();
}

// Check for integer translate with the same 2x2 matrix.
// Returns the translation, and true if the change from creation matrix to the position matrix
// supports using direct glyph masks. Returns {false, {0, 0}} otherwise.
std::tuple<bool, SkVector> VertexFiller::CanUseDirect(
        const SkMatrix& creationMatrix, const SkMatrix& positionMatrix) {
    // The existing direct glyph info can be used if the creationMatrix and the positionMatrix
    // have the same 2x2, no perspective is involved, and the translation between them is a valid
    // direct-mask offset.
    if (!creationMatrix.isFinite() || !positionMatrix.isFinite() ||
        creationMatrix.hasPerspective() || positionMatrix.hasPerspective() ||
        creationMatrix.getScaleX() != positionMatrix.getScaleX() ||
        creationMatrix.getScaleY() != positionMatrix.getScaleY() ||
        creationMatrix.getSkewX()  != positionMatrix.getSkewX()  ||
        creationMatrix.getSkewY()  != positionMatrix.getSkewY()) {
        return {false, {0, 0}};
    }

    // Both translations are single-precision results of the same SkMatrix::preTranslate math, so
    // drawing with an unchanged matrix yields an exact zero offset. Take the difference in double
    // and require it to be exactly representable as a float so that rounding can't turn a
    // non-integer (or overflowing) difference into an integer. Float rounding of very large
    // translations can't cause atlas sampling problems: the validated integer offset is what
    // positions the glyph quads.
    const double dx = (double)positionMatrix.getTranslateX() - creationMatrix.getTranslateX();
    const double dy = (double)positionMatrix.getTranslateY() - creationMatrix.getTranslateY();
    const SkVector offset = {(float)dx, (float)dy};
    if ((double)offset.x() != dx || (double)offset.y() != dy || !ValidDirectPosition(offset)) {
        return {false, {0, 0}};
    }
    return {true, offset};
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
                                                                 SkPoint drawOrigin,
                                                                 bool supportBilerpAtlas) const {
    // Compute the positionMatrix exactly as it was computed when the glyphs were created, so
    // that drawing with an unchanged matrix yields a zero offset. The baked-in matrix differs from
    // the current positionMatrix by a translation if the upper 2x2 remains the same, and there's
    // no perspective. Since there's no projection, Z is irrelevant, so it's okay that
    // fCreationMatrix is an SkMatrix and has discarded the 3rd row/col, and can ignore those
    // values in localToDevice.
    SkMatrix positionMatrix = localToDevice;
    positionMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());

    if (auto [directDrawCompatible, offset] = CanUseDirect(fCreationMatrix, positionMatrix);
        directDrawCompatible) {
        // The offset is a bounded integer, which means the generated mask can be accessed
        // without changing how texels would be sampled.
        return {fCreationBounds, SkMatrix::Translate(offset)};
    }

    if (fCanDrawDirect && !supportBilerpAtlas) {
        // Direct-mask glyphs have no padding in the atlas when atlas bilerp is unsupported.
        // Drawing with a fractional translation or non-identical matrix causes sampling
        // outside the glyph's atlas bounds, leaking neighboring atlas texels.
        return {SkRect::MakeEmpty(), SkMatrix::I()};
    }

    // Otherwise compute the relative transformation from fCreationMatrix to positionMatrix. If
    // fCreationMatrix is not invertible, return empty bounds so the draw is dropped.
    const std::optional<SkMatrix> inverse = fCreationMatrix.invert();
    if (!inverse) {
        return {SkRect::MakeEmpty(), SkMatrix::I()};
    }
    return {fCreationBounds, SkMatrix::Concat(positionMatrix, *inverse)};
}

}  // namespace sktext::gpu
