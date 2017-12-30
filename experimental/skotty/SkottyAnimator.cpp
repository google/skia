/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottyAnimator.h"

namespace skotty {

namespace {

SkScalar lerp_scalar(SkScalar v0, SkScalar v1, float t) {
    SkASSERT(t >= 0 && t <= 1);
    return v0 * (1 - t) + v1 * t;
}

SkPoint lerp_point(const SkPoint& v0, const SkPoint& v1, float t) {
    SkASSERT(t >= 0 && t <= 1);
    return SkPoint::Make(lerp_scalar(v0.x(), v1.x(), t),
                         lerp_scalar(v0.y(), v1.y(), t));
}

} // namespace

template <>
void KeyframeInterval<ScalarValue>::lerp(float t, ScalarValue* v) const {
    *v = lerp_scalar(fV0, fV1, t);
}

template <>
void KeyframeInterval<VectorValue>::lerp(float t, VectorValue* v) const {
    SkASSERT(fV0.cardinality() == fV1.cardinality());
    SkASSERT(v->cardinality() == 0);

    v->fVals.reserve(fV0.cardinality());
    for (int i = 0; i < fV0.fVals.count(); ++i) {
        v->fVals.emplace_back(lerp_scalar(fV0.fVals[i], fV1.fVals[i], t));
    }
}

template <>
void KeyframeInterval<ShapeValue>::lerp(float t, ShapeValue* v) const {
    SkASSERT(fV0.cardinality() == fV1.cardinality());
    SkASSERT(v->cardinality() == 0);

    v->fVertices.reserve(fV0.cardinality());
    for (int i = 0; i < fV0.fVertices.count(); ++i) {
        v->fVertices.push_back(
            BezierVertex({
                lerp_point(fV0.fVertices[i].fInPoint , fV1.fVertices[i].fInPoint , t),
                lerp_point(fV0.fVertices[i].fOutPoint, fV1.fVertices[i].fOutPoint, t),
                lerp_point(fV0.fVertices[i].fVertex  , fV1.fVertices[i].fVertex  , t)
            }));
    }

    // hmm, any meaningful interpolation to consider here?
    v->fClose = fV0.fClose;
}

} // namespace skotty
