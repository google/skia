/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOctoBounds_DEFINED
#define GrOctoBounds_DEFINED

#include "include/core/SkRect.h"
#include <functional>

/**
 * This class is composed of two bounding boxes: one in device space, and one in a 45-degree rotated
 * space.
 *
 * The 45-degree bounding box resides in "| 1  -1 | * coords" space.
 *                                        | 1   1 |
 *
 * The intersection of these two boxes defines the bounding octagon of a shape.
 *
 * Furthermore, both bounding boxes are fully tightened. This means we can blindly find the
 * intersections between each diagonal and its vertical and horizontal neighbors, and be left with
 * 8 points that define a convex (possibly degenerate) octagon.
 */
class GrOctoBounds {
public:
    GrOctoBounds() = default;
    GrOctoBounds(const SkRect& bounds, const SkRect& bounds45) {
        this->set(bounds, bounds45);
    }

    void set(const SkRect& bounds, const SkRect& bounds45) {
        fBounds = bounds;
        fBounds45 = bounds45;
        SkDEBUGCODE(this->validateBoundsAreTight());
    }

    bool operator==(const GrOctoBounds& that) const {
        return fBounds == that.fBounds && fBounds45 == that.fBounds45;
    }
    bool operator!=(const GrOctoBounds& that) const { return !(*this == that); }

    const SkRect& bounds() const { return fBounds; }
    float left() const { return fBounds.left(); }
    float top() const { return fBounds.top(); }
    float right() const { return fBounds.right(); }
    float bottom() const { return fBounds.bottom(); }


    // The 45-degree bounding box resides in "| 1  -1 | * coords" space.
    //                                        | 1   1 |
    const SkRect& bounds45() const { return fBounds45; }
    float left45() const { return fBounds45.left(); }
    float top45() const { return fBounds45.top(); }
    float right45() const { return fBounds45.right(); }
    float bottom45() const { return fBounds45.bottom(); }

    void roundOut(SkIRect* out) const {
        // The octagon is the intersection of fBounds and fBounds45 (see the comment at the start of
        // the class). The octagon's bounding box is therefore just fBounds. And the integer
        // bounding box can be found by simply rounding out fBounds.
        fBounds.roundOut(out);
    }

    GrOctoBounds makeOffset(float dx, float dy) const {
        GrOctoBounds offset;
        offset.setOffset(*this, dx, dy);
        return offset;
    }

    void setOffset(const GrOctoBounds& octoBounds, float dx, float dy) {
        fBounds = octoBounds.fBounds.makeOffset(dx, dy);
        fBounds45 = octoBounds.fBounds45.makeOffset(dx - dy, dx + dy);
        SkDEBUGCODE(this->validateBoundsAreTight());
    }

    void outset(float radius) {
        fBounds.outset(radius, radius);
        fBounds45.outset(radius*SK_ScalarSqrt2, radius*SK_ScalarSqrt2);
        SkDEBUGCODE(this->validateBoundsAreTight());
    }

    // Clips the octo bounds by a clip rect and ensures the resulting bounds are fully tightened.
    // Returns false if the octagon and clipRect do not intersect at all.
    //
    // NOTE: Does not perform a trivial containment test before the clip routine. It is probably a
    // good idea to not call this method if 'this->bounds()' are fully contained within 'clipRect'.
    bool SK_WARN_UNUSED_RESULT clip(const SkIRect& clipRect);

    // The 45-degree bounding box resides in "| 1  -1 | * coords" space.
    //                                        | 1   1 |
    //
    //  i.e., | x45 | = | x - y |
    //        | y45 | = | x + y |
    //
    // These methods transform points between device space and 45-degree space.
    constexpr static float Get_x45(float x, float y) { return x - y; }
    constexpr static float Get_y45(float x, float y) { return x + y; }
    constexpr static float Get_x(float x45, float y45) { return (x45 + y45) * .5f; }
    constexpr static float Get_y(float x45, float y45) { return (y45 - x45) * .5f; }

#if defined(SK_DEBUG) || defined(GR_TEST_UTILS)
    void validateBoundsAreTight() const;
    void validateBoundsAreTight(const std::function<void(
            bool cond, const char* file, int line, const char* code)>& validateFn) const;
#endif

private:
    SkRect fBounds;
    SkRect fBounds45;
};

#endif
