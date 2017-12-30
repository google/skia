/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyProperties_DEFINED
#define SkottyProperties_DEFINED

#include "SkPoint.h"
#include "SkottyPriv.h"
#include "SkTArray.h"
#include "SkTypes.h"

#include <memory>

class SkPath;

namespace  skotty {

struct BezierVertex {
    SkPoint fInPoint,  // "in" control point, relative to the vertex
            fOutPoint, // "out" control point, relative to the vertex
            fVertex;
};

struct ScalarValue {
    float fVal;

    static bool Parse(const Json::Value&, ScalarValue*);

    ScalarValue() : fVal(0) {}
    explicit ScalarValue(SkScalar v) : fVal(v) {}

    ScalarValue& operator=(SkScalar v) { fVal = v; return *this; }

    operator SkScalar() const { return fVal; }

    size_t cardinality() const { return 1; }

    template <typename T>
    T as() const;
};

template <>
inline SkScalar ScalarValue::as<SkScalar>() const {
    return fVal;
}

struct VectorValue {
    SkTArray<ScalarValue, true> fVals;

    static bool Parse(const Json::Value&, VectorValue*);

    VectorValue()                               = default;
    VectorValue(const VectorValue&)             = delete;
    VectorValue(VectorValue&&)                  = default;
    VectorValue& operator==(const VectorValue&) = delete;

    size_t cardinality() const { return SkTo<size_t>(fVals.count()); }

    template <typename T>
    T as() const;
};

struct ShapeValue {
    SkTArray<BezierVertex, true> fVertices;
    bool                         fClose = false;

    ShapeValue()                              = default;
    ShapeValue(const ShapeValue&)             = delete;
    ShapeValue(ShapeValue&&)                  = default;
    ShapeValue& operator==(const ShapeValue&) = delete;

    static bool Parse(const Json::Value&, ShapeValue*);

    size_t cardinality() const { return SkTo<size_t>(fVertices.count()); }

    template <typename T>
    T as() const;
};

} // namespace skotty

#endif // SkottyProperties_DEFINED
