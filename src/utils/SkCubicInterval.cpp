#include "SkCubicInterval.h"

static SkScalar eval_cubic(SkScalar c1, SkScalar c2, SkScalar c3,
                           SkScalar t) {
    return SkScalarMul(SkScalarMul(SkScalarMul(c3, t) + c2, t) + c1, t);
}

static SkScalar find_cubic_t(SkScalar c1, SkScalar c2, SkScalar c3,
                             SkScalar targetX) {
    SkScalar minT = 0;
    SkScalar maxT = SK_Scalar1;
    SkScalar t;

    for (;;) {
        t = SkScalarAve(minT, maxT);
        SkScalar x = eval_cubic(c1, c2, c3, t);
        if (SkScalarNearlyZero(x - targetX)) {
            break;
        }
        // subdivide the range and try again
        if (x < targetX) {
            minT = t;
        } else {
            maxT = t;
        }
    }
    return t;
}

/*
    a(1-t)^3 + 3bt(1-t)^2 + 3ct^2(1-t) + dt^3
    a: [0, 0]
    d: [1, 1]

    3bt - 6bt^2 + 3bt^3 + 3ct^2 - 3ct^3 + t^3
    C1 = t^1: 3b
    C2 = t^2: 3c - 6b
    C3 = t^3: 3b - 3c + 1

    ((C3*t + C2)*t + C1)*t
 */
SkScalar SkEvalCubicInterval(SkScalar x1, SkScalar y1,
                             SkScalar x2, SkScalar y2,
                             SkScalar unitX) {
    x1 = SkScalarPin(x1, 0, SK_Scalar1);
    x2 = SkScalarPin(x2, 0, SK_Scalar1);
    unitX = SkScalarPin(unitX, 0, SK_Scalar1);

    // First compute our coefficients in X
    x1 *= 3;
    x2 *= 3;

    // now search for t given unitX
    SkScalar t = find_cubic_t(x1, x2 - 2*x1, x1 - x2 + SK_Scalar1, unitX);
    
    // now evaluate the cubic in Y
    y1 *= 3;
    y2 *= 3;
    return eval_cubic(y1, y2 - 2*y1, y1 - y2 + SK_Scalar1, t);
}

