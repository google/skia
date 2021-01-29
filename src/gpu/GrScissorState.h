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
    // The disabled scissor state for a render target of an unknown size.
    GrScissorState()
        : fRTSize1({-1, -1})
        , fRect({0, 0, 0, 0})
        , fEnabled(false) {
    }

    // The disabled scissor state for a render target of the given size.
    explicit GrScissorState(const SkISize& rtDims)
            : fRTSize1(rtDims)
            , fRect({0, 0, 0, 0})
            , fEnabled(false) {}

    bool known() const {
        return fRTSize1.fWidth > 0 && fRTSize1.fHeight > 0;
    }

    void setDisabled() {
        fRect.setEmpty();
        fEnabled = false;
    }

    bool set1(const SkIRect& rect) {
        fEnabled = true;

        if (this->known()) {
            fRect = SkIRect::MakeSize(fRTSize1);
            return this->intersect(rect);
        } else {
            fRect = rect;
            return !fRect.isEmpty();
        }
    }

    bool SK_WARN_UNUSED_RESULT intersect(const SkIRect& rect) {
        SkASSERT(fEnabled);

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
        if (fRTSize1.fWidth < 0 && fRTSize1.fHeight < 0) {
            // If we don't know the final backing dimensions we can never discard the stencil
            return false;
        }

        SkASSERT(logicalDimensions.fWidth <= fRTSize1.fWidth &&
                 logicalDimensions.fHeight <= fRTSize1.fHeight);
        if (fRect.fLeft == 0 && fRect.fTop == 0 && fRect.fRight >= logicalDimensions.fWidth &&
            fRect.fBottom >= logicalDimensions.fHeight) {
            this->setDisabled();
            return true;
        } else {
            return false;
        }
    }

    bool operator==(const GrScissorState& other) const {
        return fEnabled == other.fEnabled && fRTSize1 == other.fRTSize1 && fRect == other.fRect;
    }
    bool operator!=(const GrScissorState& other) const { return !(*this == other); }

    bool enabled() const {
        return fEnabled;
//        SkASSERT(fRect.isEmpty() || SkIRect::MakeSize(fRTSize).contains(fRect));
        // This is equivalent to a strict contains check on SkIRect::MakeSize(rtSize) w/o creating
        // the render target bounding rectangle.
//        return fRect.fLeft > 0 || fRect.fTop > 0 ||
//               fRect.fRight < fRTSize.fWidth || fRect.fBottom < fRTSize.fHeight;
    }

    // Will always be equal to or contained in the rt bounds, or empty if scissor rectangles were
    // added that did not intersect with the render target or prior scissor.
    const SkIRect& rect() const {
        SkASSERT(fEnabled);
//        SkASSERT(fRect.isEmpty() || SkIRect::MakeSize(fRTSize).contains(fRect));
        return fRect;
    }

    SkISize rtDimensions() const { return fRTSize1; }

    GrScissorState makeXlated(SkIPoint offset) const {
        GrScissorState tmp = *this;
        tmp.fRect.offset(offset.fX, offset.fY);
        return tmp;
    }

    GrScissorState makeOffset(const SkIPoint& p) const {
        return { fRTSize1, fRect.makeOffset(p.fX, p.fY), fEnabled };
    }

private:
    GrScissorState(SkISize a1, SkIRect a2, bool a3) : fRTSize1(a1), fRect(a2), fEnabled(a3) {}

    SkISize fRTSize1;
    SkIRect fRect;
    bool    fEnabled = false;
};

#endif
