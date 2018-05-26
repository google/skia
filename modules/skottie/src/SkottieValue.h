/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieValue_DEFINED
#define SkottieValue_DEFINED

#include "SkPath.h"
#include "SkScalar.h"

#include <vector>

namespace  skottie {

template <typename T>
struct ValueTraits {
    static size_t Cardinality(const T&);

    template <typename U>
    static U As(const T&);

    static T Lerp(const T&, const T&, float);
};

using ScalarValue = SkScalar;
using VectorValue = std::vector<ScalarValue>;

struct BezierVertex {
    SkPoint fInPoint,  // "in" control point, relative to the vertex
            fOutPoint, // "out" control point, relative to the vertex
            fVertex;

    bool operator==(const BezierVertex& other) const {
        return fInPoint  == other.fInPoint
            && fOutPoint == other.fOutPoint
            && fVertex   == other.fVertex;
    }

    bool operator!=(const BezierVertex& other) const { return !(*this == other); }
};

struct ShapeValue {
    std::vector<BezierVertex> fVertices;
    bool                      fClosed   : 1,
                              fVolatile : 1;

    ShapeValue() : fClosed(false), fVolatile(false) {}
    ShapeValue(const ShapeValue&)            = default;
    ShapeValue(ShapeValue&&)                 = default;
    ShapeValue& operator=(const ShapeValue&) = default;

    bool operator==(const ShapeValue& other) const {
        return fVertices == other.fVertices && fClosed == other.fClosed;
    }

    bool operator!=(const ShapeValue& other) const { return !(*this == other); }
};

} // namespace skottie

#endif // SkottieValue_DEFINED
