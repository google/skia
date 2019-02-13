/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkCurve.h"

#include "SkRandom.h"
#include "SkReflected.h"

static SkScalar eval_cubic(const SkScalar* pts, SkScalar x) {
    SkScalar ix = (1 - x);
    return pts[0]*ix*ix*ix + pts[1]*3*ix*ix*x + pts[2]*3*ix*x*x + pts[3]*x*x*x;
}

SkScalar SkCurveSegment::eval(SkScalar x, SkRandom& random) const {
    SkScalar result = fConstant ? fMin[0] : eval_cubic(fMin, x);
    if (fRanged) {
        result += ((fConstant ? fMax[0] : eval_cubic(fMax, x)) - result) * random.nextF();
    }
    if (fBidirectional && random.nextBool()) {
        result = -result;
    }
    return result;
}

SkScalar SkCurve2::eval(SkScalar x, SkRandom& random) const {
    SkASSERT(fSegments.count() == fXValues.count() + 1);

    int i = 0;
    for (; i < fXValues.count(); ++i) {
        if (x <= fXValues[i]) {
            break;
        }
    }

    SkScalar rangeMin = (i == 0) ? 0.0f : fXValues[i - 1];
    SkScalar rangeMax = (i == fXValues.count()) ? 1.0f : fXValues[i];
    SkScalar segmentX = (x - rangeMin) / (rangeMax - rangeMin);
    SkASSERT(0.0f <= segmentX && segmentX <= 1.0f);
    return fSegments[i].eval(x, random);
}

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
