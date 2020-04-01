/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/animator/Vector.h"

namespace skottie {

// Shapes (paths) are encoded as a vector of floats.  For each vertex, we store 6 floats:
//
//   - vertex point      (2 floats)
//   - in-tangent point  (2 floats)
//   - out-tangent point (2 floats)
//
// Additionally, we store one trailing "closed shape" flag - e.g.
//
//  [ v0.x, v0.y, v0_in.x, v0_in.y, v0_out.x, v0_out.y, ... , closed_flag ]
//
enum ShapeEncodingInfo : size_t {
            kX_Index = 0,
            kY_Index = 1,
          kInX_Index = 2,
          kInY_Index = 3,
         kOutX_Index = 4,
         kOutY_Index = 5,

    kFloatsPerVertex = 6
};

static size_t shape_encoding_len(size_t vertex_count) {
    return vertex_count * kFloatsPerVertex + 1;
}

// Some versions wrap shape values as single-element arrays.
static const skjson::ObjectValue* shape_root(const skjson::Value& jv) {
    if (const skjson::ArrayValue* av = jv) {
        if (av->size() == 1) {
            return (*av)[0];
        }
    }

    return jv;
}

static bool parse_encoding_len(const skjson::Value& jv, size_t* len) {
    if (const auto* jshape = shape_root(jv)) {
        if (const skjson::ArrayValue* jvs = (*jshape)["v"]) {
            *len = shape_encoding_len(jvs->size());
            return true;
        }
    }
    return false;
}

static bool parse_encoding_data(const skjson::Value& jv, size_t data_len, float data[]) {
    const auto* jshape = shape_root(jv);
    if (!jshape) {
        return false;
    }

    // vertices are required, in/out tangents are optional
    const skjson::ArrayValue* jvs = (*jshape)["v"]; // vertex points
    const skjson::ArrayValue* jis = (*jshape)["i"]; // in-tangent points
    const skjson::ArrayValue* jos = (*jshape)["o"]; // out-tangent points

    if (!jvs || data_len != shape_encoding_len(jvs->size())) {
        return false;
    }

    auto parse_point = [](const skjson::ArrayValue* ja, size_t i, float* x, float* y) {
        SkASSERT(ja);
        const skjson::ArrayValue* jpt = (*ja)[i];

        if (!jpt || jpt->size() != 2ul) {
            return false;
        }

        return Parse((*jpt)[0], x) && Parse((*jpt)[1], y);
    };

    auto parse_optional_point = [&parse_point](const skjson::ArrayValue* ja, size_t i,
                                               float* x, float* y) {
        if (!ja || i >= ja->size()) {
            // default control point
            *x = *y = 0;
            return true;
        }

        return parse_point(*ja, i, x, y);
    };

    for (size_t i = 0; i < jvs->size(); ++i) {
        float* dst = data + i * kFloatsPerVertex;
        SkASSERT(dst + kFloatsPerVertex <= data + data_len);

        if (!parse_point         (jvs, i, dst +    kX_Index, dst +    kY_Index) ||
            !parse_optional_point(jis, i, dst +  kInX_Index, dst +  kInY_Index) ||
            !parse_optional_point(jos, i, dst + kOutX_Index, dst + kOutY_Index)) {
            return false;
        }
    }

    // "closed" flag
    data[data_len - 1] = ParseDefault<bool>((*jshape)["c"], false);

    return true;
}

ShapeValue::operator SkPath() const {
    const auto vertex_count = fData.size() / kFloatsPerVertex;

    SkPath path;

    if (vertex_count) {
        // conservatively assume all cubics
        path.incReserve(1 + SkToInt(vertex_count * 3));

        // Move to first vertex.
        path.moveTo(fData[kX_Index], fData[kY_Index]);
    }

    auto addCubic = [&](size_t from_vertex, size_t to_vertex) {
        const auto from_index = kFloatsPerVertex * from_vertex,
                     to_index = kFloatsPerVertex *   to_vertex;

        const SkPoint p0 = SkPoint{ fData[from_index +    kX_Index],
                                    fData[from_index +    kY_Index] },
                      p1 = SkPoint{ fData[  to_index +    kX_Index],
                                    fData[  to_index +    kY_Index] },
                      c0 = SkPoint{ fData[from_index + kOutX_Index],
                                    fData[from_index + kOutY_Index] } + p0,
                      c1 = SkPoint{ fData[  to_index +  kInX_Index],
                                    fData[  to_index +  kInY_Index] } + p1;

        if (c0 == p0 && c1 == p1) {
            // If the control points are coincident, we can power-reduce to a straight line.
            // TODO: we could also do that when the controls are on the same line as the
            //       vertices, but it's unclear how common that case is.
            path.lineTo(p1);
        } else {
            path.cubicTo(c0, c1, p1);
        }
    };

    for (size_t i = 1; i < vertex_count; ++i) {
        addCubic(i - 1, i);
    }

    // Close the path with an extra cubic, if needed.
    if (vertex_count && fData.back() != 0) {
        addCubic(vertex_count - 1, 0);
        path.close();
    }

    path.shrinkToFit();

    return path;
}

namespace internal {

template <>
bool AnimatablePropertyContainer::bind<ShapeValue>(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop,
                                                  ShapeValue* v) {
    VectorKeyframeAnimatorBuilder builder(parse_encoding_len, parse_encoding_data);

    return this->bindImpl(abuilder, jprop, builder, &v->fData);
}

} // namespace internal

} // namespace skottie
