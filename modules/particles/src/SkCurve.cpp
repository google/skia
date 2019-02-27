/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkCurve.h"

#include "SkRandom.h"
#include "SkReflected.h"

constexpr SkFieldVisitor::EnumStringMapping gCurveSegmentTypeMapping[] = {
    { kConstant_SegmentType, "Constant" },
    { kLinear_SegmentType,   "Linear" },
    { kCubic_SegmentType,    "Cubic" },
};

static SkColor4f operator+(SkColor4f c1, SkColor4f c2) {
    return { c1.fR + c2.fR, c1.fG + c2.fG, c1.fB + c2.fB, c1.fA + c2.fA };
}

static SkColor4f operator-(SkColor4f c1, SkColor4f c2) {
    return { c1.fR - c2.fR, c1.fG - c2.fG, c1.fB - c2.fB, c1.fA - c2.fA };
}

template <typename T>
static T eval_cubic(const T* pts, SkScalar x) {
    SkScalar ix = (1 - x);
    return pts[0]*(ix*ix*ix) + pts[1]*(3*ix*ix*x) + pts[2]*(3*ix*x*x) + pts[3]*(x*x*x);
}

template <typename T>
static T eval_segment(const T* pts, SkScalar x, int type) {
    switch (type) {
        case kLinear_SegmentType:
            return pts[0] + (pts[3] - pts[0]) * x;
        case kCubic_SegmentType:
            return eval_cubic(pts, x);
        case kConstant_SegmentType:
        default:
            return pts[0];
    }
}

SkScalar SkCurveSegment::eval(SkScalar x, SkScalar t, bool negate) const {
    SkScalar result = eval_segment(fMin, x, fType);
    if (fRanged) {
        result += (eval_segment(fMax, x, fType) - result) * t;
    }
    if (fBidirectional && negate) {
        result = -result;
    }
    return result;
}

void SkCurveSegment::visitFields(SkFieldVisitor* v) {
    v->visit("Type", fType, gCurveSegmentTypeMapping, SK_ARRAY_COUNT(gCurveSegmentTypeMapping));
    v->visit("Ranged", fRanged);
    v->visit("Bidirectional", fBidirectional);
    v->visit("A0", fMin[0]);
    if (fType == kCubic_SegmentType) {
        v->visit("B0", fMin[1]);
        v->visit("C0", fMin[2]);
    }
    if (fType != kConstant_SegmentType) {
        v->visit("D0", fMin[3]);
    }
    if (fRanged) {
        v->visit("A1", fMax[0]);
        if (fType == kCubic_SegmentType) {
            v->visit("B1", fMax[1]);
            v->visit("C1", fMax[2]);
        }
        if (fType != kConstant_SegmentType) {
            v->visit("D1", fMax[3]);
        }
    }
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

    // Always pull t and negate here, so that the stable generator behaves consistently, even if
    // our segments use an inconsistent feature-set.
    SkScalar t = random.nextF();
    bool negate = random.nextBool();
    return fSegments[i].eval(segmentX, t, negate);
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

SkColor4f SkColorCurveSegment::eval(SkScalar x, SkRandom& random) const {
    SkColor4f result = eval_segment(fMin, x, fType);
    if (fRanged) {
        result = result + (eval_segment(fMax, x, fType) - result) * random.nextF();
    }
    return result;
}

void SkColorCurveSegment::visitFields(SkFieldVisitor* v) {
    v->visit("Type", fType, gCurveSegmentTypeMapping, SK_ARRAY_COUNT(gCurveSegmentTypeMapping));
    v->visit("Ranged", fRanged);
    v->visit("A0", fMin[0]);
    if (fType == kCubic_SegmentType) {
        v->visit("B0", fMin[1]);
        v->visit("C0", fMin[2]);
    }
    if (fType != kConstant_SegmentType) {
        v->visit("D0", fMin[3]);
    }
    if (fRanged) {
        v->visit("A1", fMax[0]);
        if (fType == kCubic_SegmentType) {
            v->visit("B1", fMax[1]);
            v->visit("C1", fMax[2]);
        }
        if (fType != kConstant_SegmentType) {
            v->visit("D1", fMax[3]);
        }
    }
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
