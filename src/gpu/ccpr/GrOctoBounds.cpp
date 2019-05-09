/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrOctoBounds.h"

#ifdef SK_DEBUG
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
