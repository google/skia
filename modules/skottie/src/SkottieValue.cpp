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

PathValue::operator SkPath() const {
    const auto vertex_count = fData.size() / 6;

    SkPath path;

    if (vertex_count) {
        // conservatively assume all cubics
        path.incReserve(1 + SkToU32(vertex_count * 3));

        path.moveTo(fData[0], fData[1]);
    }

    const auto& addCubic = [&](size_t from, size_t to) {
        const auto from_index = from * 6,
                     to_index =   to * 6;

        const SkPoint v0 = SkPoint{ fData[from_index + 0], fData[from_index + 1] },
                      v1 = SkPoint{ fData[  to_index + 0], fData[  to_index + 1] },
                      c0 = SkPoint{ fData[from_index + 4], fData[from_index + 5] } + v0,
                      c1 = SkPoint{ fData[  to_index + 2], fData[  to_index + 3] } + v1;

        if (c0 == v0 && c1 == v1) {
            // If the control points are coincident, we can power-reduce to a straight line.
            // TODO: we could also do that when the controls are on the same line as the
            //       vertices, but it's unclear how common that case is.
            path.lineTo(v1);
        } else {
            path.cubicTo(c0, c1, v1);
        }
    };

    for (size_t i = 1; i < vertex_count; ++i) {
        addCubic(i - 1, i);
    }

    if (vertex_count && fData.back() != 0) {
        addCubic(vertex_count - 1, 0);
        path.close();
    }

    path.shrinkToFit();

    return path;
}

} // namespace skottie
