/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrOctoBounds.h"
#include <algorithm>

bool GrOctoBounds::clip(const SkIRect& clipRect) {
    // Intersect dev bounds with the clip rect.
    float l = std::max(fBounds.left(), (float)clipRect.left());
    float t = std::max(fBounds.top(), (float)clipRect.top());
    float r = std::min(fBounds.right(), (float)clipRect.right());
    float b = std::min(fBounds.bottom(), (float)clipRect.bottom());

    float l45 = fBounds45.left();
    float t45 = fBounds45.top();
    float r45 = fBounds45.right();
    float b45 = fBounds45.bottom();

    // Check if either the bounds or 45-degree bounds are empty. We write this check as the NOT of
    // non-empty rects, so we will return false if any values are NaN.
    if (!(l < r && t < b && l45 < r45 && t45 < b45)) {
        return false;
    }

    // Tighten dev bounds around the new (octagonal) intersection that results after clipping. This
    // may be tighter now even than the clipped bounds, depending on the diagonals. Shader code that
    // emits octagons expects both bounding boxes to circumcribe the inner octagon, and will fail if
    // they do not.
    if (l45 > Get_x45(r,b)) {
        // Slide the bottom upward until it crosses the l45 diagonal at x=r.
        //     y = x + (y0 - x0)
        // Substitute: l45 = x0 - y0
        //     y = x - l45
        b = SkScalarPin(r - l45, t, b);
    } else if (r45 < Get_x45(r,b)) {
        // Slide the right side leftward until it crosses the r45 diagonal at y=b.
        //     x = y + (x0 - y0)
        // Substitute: r45 = x0 - y0
        //     x = y + r45
        r = SkScalarPin(b + r45, l, r);
    }
    if (l45 > Get_x45(l,t)) {
        // Slide the left side rightward until it crosses the l45 diagonal at y=t.
        //     x = y + (x0 - y0)
        // Substitute: l45 = x0 - y0
        //     x = y + l45
        l = SkScalarPin(t + l45, l, r);
    } else if (r45 < Get_x45(l,t)) {
        // Slide the top downward until it crosses the r45 diagonal at x=l.
        //     y = x + (y0 - x0)
        // Substitute: r45 = x0 - y0
        //     y = x - r45
        t = SkScalarPin(l - r45, t, b);
    }
    if (t45 > Get_y45(l,b)) {
        // Slide the left side rightward until it crosses the t45 diagonal at y=b.
        //     x = -y + (x0 + y0)
        // Substitute: t45 = x0 + y0
        //     x = -y + t45
        l = SkScalarPin(t45 - b, l, r);
    } else if (b45 < Get_y45(l,b)) {
        // Slide the bottom upward until it crosses the b45 diagonal at x=l.
        //     y = -x + (y0 + x0)
        // Substitute: b45 = x0 + y0
        //     y = -x + b45
        b = SkScalarPin(b45 - l, t, b);
    }
    if (t45 > Get_y45(r,t)) {
        // Slide the top downward until it crosses the t45 diagonal at x=r.
        //     y = -x + (y0 + x0)
        // Substitute: t45 = x0 + y0
        //     y = -x + t45
        t = SkScalarPin(t45 - r, t, b);
    } else if (b45 < Get_y45(r,t)) {
        // Slide the right side leftward until it crosses the b45 diagonal at y=t.
        //     x = -y + (x0 + y0)
        // Substitute: b45 = x0 + y0
        //     x = -y + b45
        r = SkScalarPin(b45 - t, l, r);
    }

    // Tighten the 45-degree bounding box. Since the dev bounds are now fully tightened, we only
    // have to clamp the diagonals to outer corners.
    // NOTE: This will not cause l,t,r,b to need more insetting. We only ever change a diagonal by
    // pinning it to a FAR corner, which, by definition, is still outside the other corners.
    l45 = SkScalarPin(Get_x45(l,b), l45, r45);
    t45 = SkScalarPin(Get_y45(l,t), t45, b45);
    r45 = SkScalarPin(Get_x45(r,t), l45, r45);
    b45 = SkScalarPin(Get_y45(r,b), t45, b45);

    // Make one final check for empty or NaN bounds. If the dev bounds were clipped completely
    // outside one of the diagonals, they will have been pinned to empty. It's also possible that
    // some Infs crept in and turned into NaNs.
    if (!(l < r && t < b && l45 < r45 && t45 < b45)) {
        return false;
    }

    fBounds.setLTRB(l, t, r, b);
    fBounds45.setLTRB(l45, t45, r45, b45);

#ifdef SK_DEBUG
    // Verify dev bounds are inside the clip rect.
    SkASSERT(l >= (float)clipRect.left());
    SkASSERT(t >= (float)clipRect.top());
    SkASSERT(r <= (float)clipRect.right());
    SkASSERT(b <= (float)clipRect.bottom());
    this->validateBoundsAreTight();
#endif

    return true;
}

#if defined(SK_DEBUG) || defined(GR_TEST_UTILS)
void GrOctoBounds::validateBoundsAreTight() const {
    this->validateBoundsAreTight([](bool cond, const char* file, int line, const char* code) {
        SkASSERTF(cond, "%s(%d): assertion failure: \"assert(%s)\"", file, line, code);
    });
}

void GrOctoBounds::validateBoundsAreTight(const std::function<void(
        bool cond, const char* file, int line, const char* code)>& validateFn) const {
    constexpr static float epsilon = 1e-3f;

    float l=fBounds.left(), l45=fBounds45.left();
    float t=fBounds.top(), t45=fBounds45.top();
    float r=fBounds.right(), r45=fBounds45.right();
    float b=fBounds.bottom(), b45=fBounds45.bottom();

#define VALIDATE(CODE) validateFn(CODE, __FILE__, __LINE__, #CODE)
    // Verify diagonals are inside far corners of the dev bounds.
    VALIDATE(l45 >= Get_x45(l,b) - epsilon);
    VALIDATE(t45 >= Get_y45(l,t) - epsilon);
    VALIDATE(r45 <= Get_x45(r,t) + epsilon);
    VALIDATE(b45 <= Get_y45(r,b) + epsilon);
    // Verify verticals and horizontals are inside far corners of the 45-degree dev bounds.
    VALIDATE(l >= Get_x(l45,t45) - epsilon);
    VALIDATE(t >= Get_y(r45,t45) - epsilon);
    VALIDATE(r <= Get_x(r45,b45) + epsilon);
    VALIDATE(b <= Get_y(l45,b45) + epsilon);
    // Verify diagonals are outside middle corners of the dev bounds.
    VALIDATE(l45 <= Get_x45(r,b) + epsilon);
    VALIDATE(l45 <= Get_x45(l,t) + epsilon);
    VALIDATE(t45 <= Get_y45(l,b) + epsilon);
    VALIDATE(t45 <= Get_y45(r,t) + epsilon);
    VALIDATE(r45 >= Get_x45(l,t) - epsilon);
    VALIDATE(r45 >= Get_x45(r,b) - epsilon);
    VALIDATE(b45 >= Get_y45(r,t) - epsilon);
    VALIDATE(b45 >= Get_y45(l,b) - epsilon);
    // Verify verticals and horizontals are outside middle corners of the 45-degree dev bounds.
    VALIDATE(l <= Get_x(l45,b45) + epsilon);
    VALIDATE(l <= Get_x(r45,t45) + epsilon);
    VALIDATE(t <= Get_y(r45,b45) + epsilon);
    VALIDATE(t <= Get_y(l45,t45) + epsilon);
    VALIDATE(r >= Get_x(r45,t45) - epsilon);
    VALIDATE(r >= Get_x(l45,b45) - epsilon);
    VALIDATE(b >= Get_y(l45,t45) - epsilon);
    VALIDATE(b >= Get_y(r45,b45) - epsilon);
#undef VALIDATE
}
#endif
