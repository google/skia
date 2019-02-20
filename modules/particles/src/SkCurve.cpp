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

static SkColor4f operator+(SkColor4f c1, SkColor4f c2) {
    return { c1.fR + c2.fR, c1.fG + c2.fG, c1.fB + c2.fB, c1.fA + c2.fA };
}

static SkColor4f eval_cubic(const SkColor4f* pts, SkScalar x) {
    SkScalar ix = (1 - x);
    return pts[0]*(ix*ix*ix) + pts[1]*(3*ix*ix*x) + pts[2]*(3*ix*x*x) + pts[3]*(x*x*x);
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

void SkCurveSegment::visitFields(SkFieldVisitor* v) {
    v->visit("Constant", fConstant);
    v->visit("Ranged", fRanged);
    v->visit("Bidirectional", fBidirectional);
    v->visit("A0", fMin[0]);
    v->visit("B0", fMin[1]);
    v->visit("C0", fMin[2]);
    v->visit("D0", fMin[3]);
    v->visit("A1", fMax[0]);
    v->visit("B1", fMax[1]);
    v->visit("C1", fMax[2]);
    v->visit("D1", fMax[3]);
}

SkScalar SkCurve::eval(SkScalar x, SkRandom& random) const {
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
    if (!SkScalarIsFinite(segmentX)) {
        segmentX = rangeMin;
    }
    SkASSERT(0.0f <= segmentX && segmentX <= 1.0f);
    return fSegments[i].eval(segmentX, random);
}

void SkCurve::visitFields(SkFieldVisitor* v) {
    v->visit("XValues", fXValues);
    v->visit("Segments", fSegments);

    // Validate and fixup
    if (fSegments.empty()) {
        fSegments.push_back().setConstant(0.0f);
    }
    fXValues.resize_back(fSegments.count() - 1);
    for (int i = 0; i < fXValues.count(); ++i) {
        fXValues[i] = SkTPin(fXValues[i], i > 0 ? fXValues[i - 1] : 0.0f, 1.0f);
    }
}

// TODO: This implementation is extremely conservative, because it uses the position of the control
// points as the actual range. The curve typically doesn't reach that far. Evaluating the curve at
// each of [0, 1/3, 2/3, 1] would be tighter, but can be too tight in some cases.
void SkCurve::getExtents(SkScalar extents[2]) const {
    extents[0] = INFINITY;
    extents[1] = -INFINITY;
    auto extend = [=](SkScalar y) {
        extents[0] = SkTMin(extents[0], y);
        extents[1] = SkTMax(extents[1], y);
    };
    for (const auto& segment : fSegments) {
        for (int i = 0; i < (segment.fConstant ? 1 : 4); ++i) {
            extend(segment.fMin[i]);
            if (segment.fRanged) {
                extend(segment.fMax[i]);
            }
        }
    }
}

SkColor4f SkColorCurveSegment::eval(SkScalar x, SkRandom& random) const {
    SkColor4f result = fConstant ? fMin[0] : eval_cubic(fMin, x);
    if (fRanged) {
        result = result +
                ((fConstant ? fMax[0] : eval_cubic(fMax, x)) + (result * -1)) * random.nextF();
    }
    return result;
}

void SkColorCurveSegment::visitFields(SkFieldVisitor* v) {
    v->visit("Constant", fConstant);
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

SkColor4f SkColorCurve::eval(SkScalar x, SkRandom& random) const {
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
    if (!SkScalarIsFinite(segmentX)) {
        segmentX = rangeMin;
    }
    SkASSERT(0.0f <= segmentX && segmentX <= 1.0f);
    return fSegments[i].eval(segmentX, random);
}

void SkColorCurve::visitFields(SkFieldVisitor* v) {
    v->visit("XValues", fXValues);
    v->visit("Segments", fSegments);

    // Validate and fixup
    if (fSegments.empty()) {
        fSegments.push_back().setConstant(SkColor4f{ 1.0f, 1.0f, 1.0f, 1.0f });
    }
    fXValues.resize_back(fSegments.count() - 1);
    for (int i = 0; i < fXValues.count(); ++i) {
        fXValues[i] = SkTPin(fXValues[i], i > 0 ? fXValues[i - 1] : 0.0f, 1.0f);
    }
}
