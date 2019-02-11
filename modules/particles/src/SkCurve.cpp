/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkCurve.h"

#include "SkRandom.h"
#include "SkReflected.h"

SkScalar SkCurve::eval(float x, SkRandom& random) const {
    float ix = (1 - x);
    float y0 = fMin[0] * ix*ix*ix + fMin[1] * 3 * ix*ix*x + fMin[2] * 3 * ix*x*x + fMin[3] * x*x*x;
    float y1 = fMax[0] * ix*ix*ix + fMax[1] * 3 * ix*ix*x + fMax[2] * 3 * ix*x*x + fMax[3] * x*x*x;
    return fRanged ? y0 + (y1 - y0) * random.nextF() : y0;
}

void SkCurve::visitFields(SkFieldVisitor* v) {
    v->visit("Ranged", fRanged);
    v->visit("A0", fMin[0]);
    v->visit("B0", fMin[1]);
    v->visit("C0", fMin[2]);
    v->visit("D0", fMin[3]);
    v->visit("A1", fMax[0]);
    v->visit("B1", fMax[1]);
    v->visit("C1", fMax[2]);
    v->visit("D1", fMax[3]);
}

float SkRangedFloat::eval(SkRandom& random) const {
    return random.nextRangeF(fMin, fMax);
}

void SkRangedFloat::visitFields(SkFieldVisitor* v) {
    v->visit("min", fMin);
    v->visit("max", fMax);
}
