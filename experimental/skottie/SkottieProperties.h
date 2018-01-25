/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieProperties_DEFINED
#define SkottieProperties_DEFINED

#include "SkColor.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkSize.h"
#include "SkRefCnt.h"
#include "SkTArray.h"
#include "SkTypes.h"

#include <memory>
#include <vector>

namespace sksg {
class Color;
class Gradient;
class LinearGradient;
class Matrix;
class Path;
class RadialGradient;
class RRect;
class RenderNode;;
}

namespace Json { class Value; }

namespace  skottie {

template <typename T>
struct ValueTraits {
    static size_t Cardinality(const T&);

    template <typename U>
    static U As(const T&);
};

using ScalarValue = SkScalar;
using VectorValue = std::vector<ScalarValue>;
using ShapeValue  = SkPath;

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
    COMPOSITE_PROPERTY(Radius  , SkSize  , SkSize::Make(0, 0))

private:
    void apply();

    sk_sp<sksg::RRect> fRRectNode;

    using INHERITED = SkRefCnt;
};

class CompositePolyStar final : public SkRefCnt {
public:
    enum class Type {
        kStar, kPoly,
    };

    CompositePolyStar(sk_sp<sksg::Path>, Type);

    COMPOSITE_PROPERTY(Position      , SkPoint , SkPoint::Make(0, 0))
    COMPOSITE_PROPERTY(PointCount    , SkScalar, 0)
    COMPOSITE_PROPERTY(InnerRadius   , SkScalar, 0)
    COMPOSITE_PROPERTY(OuterRadius   , SkScalar, 0)
    COMPOSITE_PROPERTY(InnerRoundness, SkScalar, 0)
    COMPOSITE_PROPERTY(OuterRoundness, SkScalar, 0)
    COMPOSITE_PROPERTY(Rotation      , SkScalar, 0)

private:
    void apply();

    sk_sp<sksg::Path> fPathNode;
    Type              fType;

    using INHERITED = SkRefCnt;
};

class CompositeTransform final : public SkRefCnt {
public:
    explicit CompositeTransform(sk_sp<sksg::Matrix>);

    COMPOSITE_PROPERTY(AnchorPoint, SkPoint , SkPoint::Make(0, 0))
    COMPOSITE_PROPERTY(Position   , SkPoint , SkPoint::Make(0, 0))
    COMPOSITE_PROPERTY(Scale      , SkVector, SkPoint::Make(100, 100))
    COMPOSITE_PROPERTY(Rotation   , SkScalar, 0)
    COMPOSITE_PROPERTY(Skew       , SkScalar, 0)
    COMPOSITE_PROPERTY(SkewAxis   , SkScalar, 0)

private:
    void apply();

    sk_sp<sksg::Matrix> fMatrixNode;

    using INHERITED = SkRefCnt;
};

class CompositeGradient : public SkRefCnt {
public:
    COMPOSITE_PROPERTY(StartPoint, SkPoint              , SkPoint::Make(0, 0)    )
    COMPOSITE_PROPERTY(EndPoint  , SkPoint              , SkPoint::Make(0, 0)    )
    COMPOSITE_PROPERTY(ColorStops, std::vector<SkScalar>, std::vector<SkScalar>())

protected:
    CompositeGradient(sk_sp<sksg::Gradient>, size_t stopCount);

    const SkPoint& startPoint() const { return fStartPoint; }
    const SkPoint& endPoint()   const { return fEndPoint;   }

    sk_sp<sksg::Gradient> fGradient;
    size_t                fStopCount;

    virtual void onApply() = 0;

private:
    void apply();

    using INHERITED = SkRefCnt;
};

class CompositeLinearGradient final : public CompositeGradient {
public:
    CompositeLinearGradient(sk_sp<sksg::LinearGradient>, size_t stopCount);

private:
    void onApply() override;

    using INHERITED = CompositeGradient;
};

class CompositeRadialGradient final : public CompositeGradient {
public:
    CompositeRadialGradient(sk_sp<sksg::RadialGradient>, size_t stopCount);

private:
    void onApply() override;

    using INHERITED = CompositeGradient;
};

#undef COMPOSITE_PROPERTY

} // namespace skottie

#endif // SkottieProperties_DEFINED
