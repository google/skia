/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieValue.h"

#include "SkColor.h"
#include "SkPoint.h"
#include "SkSize.h"

namespace  skottie {

template <>
size_t ValueTraits<ScalarValue>::Cardinality(const ScalarValue&) {
    return 1;
}

template <>
template <>
SkScalar ValueTraits<ScalarValue>::As<SkScalar>(const ScalarValue& v) {
    return v;
}

template <>
size_t ValueTraits<VectorValue>::Cardinality(const VectorValue& vec) {
    return vec.size();
}

template <>
template <>
SkColor ValueTraits<VectorValue>::As<SkColor>(const VectorValue& v) {
    // best effort to turn this into a color
    const auto r = v.size() > 0 ? v[0] : 0,
               g = v.size() > 1 ? v[1] : 0,
               b = v.size() > 2 ? v[2] : 0,
               a = v.size() > 3 ? v[3] : 1;

    return SkColorSetARGB(SkTPin<SkScalar>(a, 0, 1) * 255,
                          SkTPin<SkScalar>(r, 0, 1) * 255,
                          SkTPin<SkScalar>(g, 0, 1) * 255,
                          SkTPin<SkScalar>(b, 0, 1) * 255);
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
size_t ValueTraits<ShapeValue>::Cardinality(const ShapeValue& path) {
    return SkTo<size_t>(path.countVerbs());
}

template <>
template <>
SkPath ValueTraits<ShapeValue>::As<SkPath>(const ShapeValue& path) {
    return path;
}

} // namespace skottie
