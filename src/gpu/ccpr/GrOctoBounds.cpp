/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrOctoBounds.h"

#ifdef SK_DEBUG
void GrOctoBounds::validateBoundsAreTight(const std::function<void(bool)>& validateFn) const {
    constexpr static float epsilon = 1e-3;

    float l=fBounds.left(), l45=fBounds45.left();
    float t=fBounds.top(), t45=fBounds45.top();
    float r=fBounds.right(), r45=fBounds45.right();
    float b=fBounds.bottom(), b45=fBounds45.bottom();

    // Verify diagonals are inside far corners of the dev bounds.
    validateFn(l45 >= Get_x45(l,b) - epsilon);
    validateFn(t45 >= Get_y45(l,t) - epsilon);
    validateFn(r45 <= Get_x45(r,t) + epsilon);
    validateFn(b45 <= Get_y45(r,b) + epsilon);
    // Verify verticals and horizontals are inside far corners of the 45-degree dev bounds.
    validateFn(l >= Get_x(l45,t45) - epsilon);
    validateFn(t >= Get_y(r45,t45) - epsilon);
    validateFn(r <= Get_x(r45,b45) + epsilon);
    validateFn(b <= Get_y(l45,b45) + epsilon);
    // Verify diagonals are outside middle corners of the dev bounds.
    validateFn(l45 <= Get_x45(r,b) + epsilon);
    validateFn(l45 <= Get_x45(l,t) + epsilon);
    validateFn(t45 <= Get_y45(l,b) + epsilon);
    validateFn(t45 <= Get_y45(r,t) + epsilon);
    validateFn(r45 >= Get_x45(l,t) - epsilon);
    validateFn(r45 >= Get_x45(r,b) - epsilon);
    validateFn(b45 >= Get_y45(r,t) - epsilon);
    validateFn(b45 >= Get_y45(l,b) - epsilon);
    // Verify verticals and horizontals are outside middle corners of the 45-degree dev bounds.
    validateFn(l <= Get_x(l45,b45) + epsilon);
    validateFn(l <= Get_x(r45,t45) + epsilon);
    validateFn(t <= Get_y(r45,b45) + epsilon);
    validateFn(t <= Get_y(l45,t45) + epsilon);
    validateFn(r >= Get_x(r45,t45) - epsilon);
    validateFn(r >= Get_x(l45,b45) - epsilon);
    validateFn(b >= Get_y(l45,t45) - epsilon);
    validateFn(b >= Get_y(r45,b45) - epsilon);
}
#endif
