/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyProperties_DEFINED
#define SkottyProperties_DEFINED

#include "SkPoint.h"
#include "SkSize.h"
#include "SkottyPriv.h"
#include "SkRefCnt.h"
#include "SkTArray.h"
#include "SkTypes.h"

#include <memory>

class SkPath;

namespace sksg {
class RRect;
class RenderNode;
class Transform;
}

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

// Composite properties.

#define COMPOSITE_PROPERTY(p_name, p_type, p_default) \
    void set##p_name(const p_type& p) {               \
        if (p == f##p_name) return;                   \
        f##p_name = p;                                \
        this->apply();                                \
    }                                                 \
  private:                                            \
    p_type f##p_name = p_default;                     \
  public:

class CompositeRRect final : public SkRefCnt {
public:
    explicit CompositeRRect(sk_sp<sksg::RRect>);

    COMPOSITE_PROPERTY(Position, SkPoint , SkPoint::Make(0, 0))
    COMPOSITE_PROPERTY(Size    , SkSize  , SkSize::Make(0, 0))
    COMPOSITE_PROPERTY(Radius  , SkScalar, 0)

private:
    void apply();

    sk_sp<sksg::RRect> fRRectNode;

    using INHERITED = SkRefCnt;
};

class CompositeTransform final : public SkRefCnt {
public:
    explicit CompositeTransform(sk_sp<sksg::RenderNode>);

    const sk_sp<sksg::Transform>& node() const { return fTransformNode; }

    COMPOSITE_PROPERTY(AnchorPoint, SkPoint , SkPoint::Make(0, 0))
    COMPOSITE_PROPERTY(Position   , SkPoint , SkPoint::Make(0, 0))
    COMPOSITE_PROPERTY(Scale      , SkVector, SkPoint::Make(100, 100))
    COMPOSITE_PROPERTY(Rotation   , SkScalar, 0)
    COMPOSITE_PROPERTY(Skew       , SkScalar, 0)
    COMPOSITE_PROPERTY(SkewAxis   , SkScalar, 0)

private:
    void apply();

    sk_sp<sksg::Transform> fTransformNode;

    using INHERITED = SkRefCnt;
};

#undef COMPOSITE_PROPERTY

} // namespace skotty

#endif // SkottyProperties_DEFINED
