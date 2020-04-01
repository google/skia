/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottieValue.h"

#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSize.h"
#include "include/private/SkNx.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"

#include <algorithm>

namespace  skottie {

template <>
bool ValueTraits<ScalarValue>::FromJSON(const skjson::Value& jv, const internal::AnimationBuilder*,
                                        ScalarValue* v) {
    return Parse(jv, v);
}

template <>
bool ValueTraits<Vec2Value>::FromJSON(const skjson::Value& jv, const internal::AnimationBuilder*,
                                      Vec2Value* v) {
    return Parse(jv, v);
}

template <>
template <>
SkScalar ValueTraits<ScalarValue>::As<SkScalar>(const ScalarValue& v) {
    return v;
}

// DEPRECATED: remove after converting everything to SkColor4f
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
SkColor4f ValueTraits<VectorValue>::As<SkColor4f>(const VectorValue& v) {
    // best effort to turn a vector into a color
    const auto r = v.size() > 0 ? SkTPin(v[0], 0.0f, 1.0f) : 0,
               g = v.size() > 1 ? SkTPin(v[1], 0.0f, 1.0f) : 0,
               b = v.size() > 2 ? SkTPin(v[2], 0.0f, 1.0f) : 0,
               a = v.size() > 3 ? SkTPin(v[3], 0.0f, 1.0f) : 1;

    return { r, g, b, a };
}

template <>
template <>
SkPoint ValueTraits<VectorValue>::As<SkPoint>(const VectorValue& vec) {
    // best effort to turn this into a 2D point
    return SkPoint {
        vec.size() > 0 ? vec[0] : 0,
        vec.size() > 1 ? vec[1] : 0,
    };
}

template <>
template <>
SkV3 ValueTraits<VectorValue>::As<SkV3>(const VectorValue& vec) {
    // best effort to turn this into a 3D point
    return SkV3 {
        vec.size() > 0 ? vec[0] : 0,
        vec.size() > 1 ? vec[1] : 0,
        vec.size() > 2 ? vec[2] : 0,
    };
}

template <>
template <>
SkSize ValueTraits<VectorValue>::As<SkSize>(const VectorValue& vec) {
    const auto pt = ValueTraits::As<SkPoint>(vec);
    return SkSize::Make(pt.x(), pt.y());
}

} // namespace skottie
