/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieValue.h"

#include "SkColor.h"
#include "SkNx.h"
#include "SkPoint.h"
#include "SkSize.h"

namespace  skottie {

template <>
bool ValueTraits<ScalarValue>::CanLerp(const ScalarValue&, const ScalarValue&) {
    return true;
}

template <>
void ValueTraits<ScalarValue>::Lerp(const ScalarValue& v0, const ScalarValue& v1, float t,
                                    ScalarValue* result) {
    SkASSERT(t >= 0 && t <= 1);
    *result = v0 + (v1 - v0) * t;
}

template <>
template <>
SkScalar ValueTraits<ScalarValue>::As<SkScalar>(const ScalarValue& v) {
    return v;
}

template <>
bool ValueTraits<VectorValue>::CanLerp(const VectorValue& v1, const VectorValue& v2) {
    return v1.size() == v2.size();
}

template <>
void ValueTraits<VectorValue>::Lerp(const VectorValue& v0, const VectorValue& v1, float t,
                                    VectorValue* result) {
    SkASSERT(v0.size() == v1.size());

    result->resize(v0.size());

    for (size_t i = 0; i < v0.size(); ++i) {
        ValueTraits<ScalarValue>::Lerp(v0[i], v1[i], t, &(*result)[i]);
    }
}

template <>
template <>
SkColor ValueTraits<VectorValue>::As<SkColor>(const VectorValue& v) {
    // best effort to turn this into a color
    const auto r = v.size() > 0 ? v[0] : 0,
               g = v.size() > 1 ? v[1] : 0,
               b = v.size() > 2 ? v[2] : 0,
               a = v.size() > 3 ? v[3] : 1;

    return SkColorSetARGB(SkScalarRoundToInt(SkTPin(a, 0.0f, 1.0f) * 255),
                          SkScalarRoundToInt(SkTPin(r, 0.0f, 1.0f) * 255),
                          SkScalarRoundToInt(SkTPin(g, 0.0f, 1.0f) * 255),
                          SkScalarRoundToInt(SkTPin(b, 0.0f, 1.0f) * 255));
}

template <>
template <>
SkPoint ValueTraits<VectorValue>::As<SkPoint>(const VectorValue& vec) {
    // best effort to turn this into a point
    const auto x = vec.size() > 0 ? vec[0] : 0,
               y = vec.size() > 1 ? vec[1] : 0;
    return SkPoint::Make(x, y);
}

template <>
template <>
SkSize ValueTraits<VectorValue>::As<SkSize>(const VectorValue& vec) {
    const auto pt = ValueTraits::As<SkPoint>(vec);
    return SkSize::Make(pt.x(), pt.y());
}

template <>
bool ValueTraits<ShapeValue>::CanLerp(const ShapeValue& v1, const ShapeValue& v2) {
    return v1.fVertices.size() == v2.fVertices.size()
        && v1.fClosed == v2.fClosed;
}

static SkPoint lerp_point(const SkPoint& v0, const SkPoint& v1, const Sk2f& t) {
    const auto v2f0 = Sk2f::Load(&v0),
               v2f1 = Sk2f::Load(&v1);

    SkPoint v;
    (v2f0 + (v2f1 - v2f0) * t).store(&v);

    return v;
}

template <>
void ValueTraits<ShapeValue>::Lerp(const ShapeValue& v0, const ShapeValue& v1, float t,
                                   ShapeValue* result) {
    SkASSERT(t >= 0 && t <= 1);
    SkASSERT(v0.fVertices.size() == v1.fVertices.size());
    SkASSERT(v0.fClosed == v1.fClosed);

    result->fClosed = v0.fClosed;
    result->fVolatile = true; // interpolated values are volatile

    const auto t2f = Sk2f(t);
    result->fVertices.resize(v0.fVertices.size());

    for (size_t i = 0; i < v0.fVertices.size(); ++i) {
        result->fVertices[i] = BezierVertex({
            lerp_point(v0.fVertices[i].fInPoint , v1.fVertices[i].fInPoint , t2f),
            lerp_point(v0.fVertices[i].fOutPoint, v1.fVertices[i].fOutPoint, t2f),
            lerp_point(v0.fVertices[i].fVertex  , v1.fVertices[i].fVertex  , t2f)
        });
    }
}

template <>
template <>
SkPath ValueTraits<ShapeValue>::As<SkPath>(const ShapeValue& shape) {
    SkPath path;

    if (!shape.fVertices.empty()) {
        // conservatively assume all cubics
        path.incReserve(1 + SkToU32(shape.fVertices.size() * 3));

        path.moveTo(shape.fVertices.front().fVertex);
    }

    const auto& addCubic = [&](size_t from, size_t to) {
        const auto c0 = shape.fVertices[from].fVertex + shape.fVertices[from].fOutPoint,
                   c1 = shape.fVertices[to].fVertex   + shape.fVertices[to].fInPoint;

        if (c0 == shape.fVertices[from].fVertex &&
            c1 == shape.fVertices[to].fVertex) {
            // If the control points are coincident, we can power-reduce to a straight line.
            // TODO: we could also do that when the controls are on the same line as the
            //       vertices, but it's unclear how common that case is.
            path.lineTo(shape.fVertices[to].fVertex);
        } else {
            path.cubicTo(c0, c1, shape.fVertices[to].fVertex);
        }
    };

    for (size_t i = 1; i < shape.fVertices.size(); ++i) {
        addCubic(i - 1, i);
    }

    if (!shape.fVertices.empty() && shape.fClosed) {
        addCubic(shape.fVertices.size() - 1, 0);
        path.close();
    }

    path.setIsVolatile(shape.fVolatile);

    return path;
}

} // namespace skottie
