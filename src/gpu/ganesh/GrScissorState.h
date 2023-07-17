/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrScissorState_DEFINED
#define GrScissorState_DEFINED

#include "include/core/SkRect.h"

/**
 * The scissor state is stored as the scissor rectangle and the backing store bounds of the render
 * target that the scissor will apply to. If the render target is approximate fit and the padded
 * content should not be modified, the clip should apply the render target context's logical bounds
 * as part of the scissor (e.g. when stenciling). This puts the onus on the render target context
 * to intentionally discard the scissor at its logical bounds when drawing into the padded content
 * is acceptable (e.g. color-only updates).
 */
class GrScissorState {
public:
    // The disabled scissor state for a render target of the given size.
    explicit GrScissorState(const SkISize& rtDims)
            : fRTSize(rtDims)
            , fRect(SkIRect::MakeSize(rtDims)) {}

    void setDisabled() { fRect = SkIRect::MakeSize(fRTSize); }
    bool set(const SkIRect& rect) {
        this->setDisabled();
        return this->intersect(rect);
    }

    [[nodiscard]] bool intersect(const SkIRect& rect) {
        if (!fRect.intersect(rect)) {
            fRect.setEmpty();
            return false;
        } else {
            return true;
        }
    }

    // If the scissor was configured for the backing store dimensions and it's acceptable to
    // draw outside the logical dimensions of the target, this will discard the scissor test if
    // the test wouldn't modify the logical dimensions.
    bool relaxTest(const SkISize& logicalDimensions) {
        SkASSERT(logicalDimensions.fWidth <= fRTSize.fWidth &&
                 logicalDimensions.fHeight <= fRTSize.fHeight);
        if (fRect.fLeft == 0 && fRect.fTop == 0 && fRect.fRight >= logicalDimensions.fWidth &&
            fRect.fBottom >= logicalDimensions.fHeight) {
            this->setDisabled();
            return true;
        } else {
            return false;
        }
    }

    bool operator==(const GrScissorState& other) const {
        return fRTSize == other.fRTSize && fRect == other.fRect;
    }
    bool operator!=(const GrScissorState& other) const { return !(*this == other); }

    bool enabled() const {
        SkASSERT(fRect.isEmpty() || SkIRect::MakeSize(fRTSize).contains(fRect));
        // This is equivalent to a strict contains check on SkIRect::MakeSize(rtSize) w/o creating
        // the render target bounding rectangle.
        return fRect.fLeft > 0 || fRect.fTop > 0 ||
               fRect.fRight < fRTSize.fWidth || fRect.fBottom < fRTSize.fHeight;
    }

    // Will always be equal to or contained in the rt bounds, or empty if scissor rectangles were
    // added that did not intersect with the render target or prior scissor.
    const SkIRect& rect() const {
        SkASSERT(fRect.isEmpty() || SkIRect::MakeSize(fRTSize).contains(fRect));
        return fRect;
    }

private:
    // The scissor is considered enabled if the rectangle does not cover the render target
    SkISize fRTSize;
    SkIRect fRect;
};

#endif
