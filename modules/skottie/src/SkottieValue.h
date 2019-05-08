/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieValue_DEFINED
#define SkottieValue_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"

#include <vector>

namespace skjson { class Value; }

namespace skottie {
namespace internal {
class AnimationBuilder;
} // namespace internal

template <typename T>
struct ValueTraits {
    static bool FromJSON(const skjson::Value&, const internal::AnimationBuilder*, T*);

    template <typename U>
    static U As(const T&);

    static bool CanLerp(const T&, const T&);
    static void Lerp(const T&, const T&, float, T*);
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
